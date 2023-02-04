#include "Physics/ResolveConstraintsSystem.h"
#include "DxtkString.h"
#include "Physics/Collision.h"
#include "Physics/Rigidbody.h"
#include "Physics/CharacterController.h"
#include "FTime.h"
#include "Logger.h"
#include "Physics/FixedJoint.h"

using framework::FTime;

ft_engine::ResolveConstraintsSystem::ResolveConstraintsSystem() {
	subscribe<EventComponentAdded>(this, &ResolveConstraintsSystem::onComponentAdded);
	subscribeNonConst<EventPhysicsRaycast>(this, &ResolveConstraintsSystem::onRaycastQuery);
	subscribeNonConst<EventPhysicsTriggerCollider>(this, &ResolveConstraintsSystem::onTriggerColliderQuery);
}

ft_engine::ResolveConstraintsSystem::~ResolveConstraintsSystem() {
	unsubscribe<EventComponentAdded>();
	unsubscribeNonConst<EventPhysicsRaycast>();
	unsubscribeNonConst<EventPhysicsTriggerCollider>();
}

void ft_engine::ResolveConstraintsSystem::fixedUpdate(eecs::EntityManager& entities, double deltaTime) {
	// Update Spatial Hashmap
	if (!unhashedEntities.empty()) {
		for (auto it = unhashedEntities.begin(); it != unhashedEntities.end(); ++it) {
			if (it->isValid()) {
				spatialHashmap.tryAddEntity(*it);
			}
		}
		unhashedEntities.clear();
	}
	spatialHashmap.updateDynamicEntities();

	// Gather all colliders and identify which of them are Rigidbodies / CharacterControllers
	std::vector<eecs::Entity> colliders = entities.getEntitiesWithComponents<Collider, Transform>();
	std::set<int> rigidbodies;
	std::set<int> characterControllers;
	std::set<int> jointCubes;

	const auto collidersSize = colliders.size();
	for (int i = 0; i < collidersSize; i++) {
		if (colliders[i].hasComponent<CharacterController>()) {
			characterControllers.insert(i);
			colliders[i].getComponent<CharacterController>()->lastHitGrounds.clear();
		} else if (colliders[i].hasComponent<Rigidbody>()) {
			if (colliders[i].getComponent<Rigidbody>()->constraints.contains(ERigidbodyConstraints::FixedJoint)) {
				jointCubes.insert(i);
			} else if (!colliders[i].getComponent<Rigidbody>()->constraints.contains(ERigidbodyConstraints::Freeze)) {
				rigidbodies.insert(i);
			}
		}
	}

	resolveStaticCollisions(colliders, rigidbodies, characterControllers, jointCubes);
	resolveDynamicCollisions(colliders, rigidbodies, characterControllers, jointCubes);

	//Update ground touch timer
	for (const auto& controllerIndex : characterControllers) {
		Entity currCharacterControllerEntity = colliders[controllerIndex];
		if (ASSERT_FAIL(currCharacterControllerEntity.hasComponent<CharacterController>(), "Chosen Entity should have Character Controller"))
			continue;

		CharacterController* controller = currCharacterControllerEntity.getComponent<CharacterController>().get();
		controller->timeSinceLastGroundTouch = (controller->bOnGround) ? 0.0f : controller->timeSinceLastGroundTouch + deltaTime;
	}
}

void ft_engine::ResolveConstraintsSystem::onRaycastQuery(EventPhysicsRaycast& eventData) {
	eventData.bHit = false;
	float minDistance = FLT_MAX;
	std::set<eecs::Entity> potentialColliders = spatialHashmap.queryEntities(eventData.raycast);

	for (auto it = potentialColliders.begin(); it != potentialColliders.end(); ++it) {
		eecs::Entity currEntity = *it;
		Transform* currTransform = currEntity.getComponent<Transform>().get();
		Collider* currCollider = currEntity.getComponent<Collider>().get();

		if ((1 << static_cast<uint8_t>(currCollider->layer) & eventData.collisionLayer) == 0) {
			continue;
		}

		float currDistance;
		if (currCollider->collisionPrimitive->checkRayIntersection(eventData.raycast, currCollider->offset * currTransform->getWorldTransform(), currDistance)) {
			if (currDistance < minDistance) {
				minDistance = currDistance;
				eventData.hitEntity = currEntity;
				eventData.bHit = true;
				eventData.distance = minDistance;
			}
		}
	}
}

void ft_engine::ResolveConstraintsSystem::onTriggerColliderQuery(EventPhysicsTriggerCollider& eventData) {
	Matrix triggerColliderTransform = eventData.triggerCollider.offset * eventData.triggerWorldTransform;
	Vector3 center;
	center = DirectX::XMVector3Transform(center, triggerColliderTransform);
	Vector3 transformedHalfBounds = DirectX::XMVector3Transform(eventData.triggerCollider.halfBounds, triggerColliderTransform);
	transformedHalfBounds -= triggerColliderTransform.Translation();
	transformedHalfBounds = Vector3(std::abs(transformedHalfBounds.x), std::abs(transformedHalfBounds.y), std::abs(transformedHalfBounds.z));

	Vector3 minVertex = center - transformedHalfBounds;
	Vector3 maxVertex = center + transformedHalfBounds;

	std::set<Entity> potentialColliders = spatialHashmap.queryEntities(minVertex, maxVertex);
	for (auto it = potentialColliders.begin(); it != potentialColliders.end(); ++it) {
		eecs::Entity currEntity = *it;
		Transform* currTransform = currEntity.getComponent<Transform>().get();
		Collider* currCollider = currEntity.getComponent<Collider>().get();

		if ((1 << static_cast<uint8>(currCollider->layer) & eventData.triggerLayer) == 0) {
			continue;
		}

		if (currCollider->collisionPrimitive->checkTriggerColliderIntersection(eventData.triggerCollider, currCollider->offset * currTransform->getWorldTransform(), triggerColliderTransform)) {
			eventData.foundEntities.push_back(currEntity);
		}
	}
}

void ft_engine::ResolveConstraintsSystem::onComponentAdded(const EventComponentAdded& eventData) {
	if (eventData.componentType == static_cast<uint32>(reflection::ComponentEnum::Collider)
		|| eventData.componentType == static_cast<uint32>(reflection::ComponentEnum::Rigidbody)
		|| eventData.componentType == static_cast<uint32>(reflection::ComponentEnum::CharacterController)) {
		unhashedEntities.insert(eventData.entity);
	}
}

void ft_engine::ResolveConstraintsSystem::resolveStaticCollisions(std::vector<eecs::Entity>& colliders, std::set<int>& rigidbodies, std::set<int>& characterControllers, std::set<int>& fixedJoints) {
	// Resolve fixedJoints
	for (auto fixedJoint = fixedJoints.begin(); fixedJoint != fixedJoints.end(); ++fixedJoint) {
		const int currFixedJointIdx = *fixedJoint;
		Entity currFixedJointEntity = colliders[currFixedJointIdx];
		Transform* currTransform = currFixedJointEntity.getComponent<Transform>().get();
		Collider* currCollider = currFixedJointEntity.getComponent<Collider>().get();
		Transform* parentTransform = currFixedJointEntity.getComponent<FixedJoint>()->carryingEntity.getComponent<Transform>().get();
		BoundingBox currBox = currTransform->getTransformedBoundingBox();
		std::set<Entity> potentialColliders = spatialHashmap.queryEntities(currBox);
		potentialColliders.erase(currFixedJointEntity);

		for (auto it = potentialColliders.begin(); it != potentialColliders.end(); ++it) {
			Entity potentialCollider = *it;
			Transform* otherTransform = potentialCollider.getComponent<Transform>().get();
			Collider* otherCollider = potentialCollider.getComponent<Collider>().get();
			if (!Physics::collisionTree[static_cast<uint8>(currCollider->layer)][static_cast<uint8>(otherCollider->layer)]) { continue; }

			if (potentialCollider.hasComponent<Rigidbody>() && !potentialCollider.getComponent<Rigidbody>()->constraints.contains(ERigidbodyConstraints::Freeze)) {
				continue;
			}

			if (potentialCollider.hasComponent<CharacterController>()) {
				continue;
			}

			const BoundingBox otherBox = otherTransform->getTransformedBoundingBox();
			if (!currBox.Intersects(otherBox)) {
				continue;
			}

			Contact contactInfo;
			Contact secondInfo;
			if (CollisionTest::checkIntersect(currCollider->collisionPrimitive.get(),
				otherCollider->collisionPrimitive.get(),
				currCollider->offset * currTransform->getWorldTransform(),
				otherCollider->offset * otherTransform->getWorldTransform(),
				contactInfo,
				secondInfo)) {

				resolveFixedJoint(currTransform, parentTransform, contactInfo.contactNormal * contactInfo.penetrationDepth);
			}
		}
	}


	// Resolve static CharacterController collisions
	for (auto characterController = characterControllers.begin(); characterController != characterControllers.end(); ++characterController) {
		const int currCharacterControllerIdx = *characterController;
		Entity currCharacterControllerEntity = colliders[currCharacterControllerIdx];
		Transform* currTransform = currCharacterControllerEntity.getComponent<Transform>().get();
		Collider* currCollider = currCharacterControllerEntity.getComponent<Collider>().get();
		CharacterController* charController = currCharacterControllerEntity.getComponent<CharacterController>().get();
		BoundingBox currBox = currTransform->getTransformedBoundingBox();
		std::set<Entity> potentialColliders = spatialHashmap.queryEntities(currBox);
		potentialColliders.erase(currCharacterControllerEntity);

		charController->bOnGround = false;

		for (auto it = potentialColliders.begin(); it != potentialColliders.end(); ++it) {
			Entity potentialCollider = *it;
			Transform* otherTransform = potentialCollider.getComponent<Transform>().get();
			Collider* otherCollider = potentialCollider.getComponent<Collider>().get();
			if (!Physics::collisionTree[static_cast<uint8>(currCollider->layer)][static_cast<uint8>(otherCollider->layer)]) { continue; }

			if (potentialCollider.hasComponent<Rigidbody>() && !potentialCollider.getComponent<Rigidbody>()->constraints.contains(ERigidbodyConstraints::Freeze)) {
				continue;
			}

			if (potentialCollider.hasComponent<CharacterController>()) {
				continue;
			}

			const BoundingBox otherBox = otherTransform->getTransformedBoundingBox();
			if (!currBox.Intersects(otherBox)) {
				continue;
			}

			Contact contactInfo;
			Contact secondInfo;
			if (CollisionTest::checkIntersect(currCollider->collisionPrimitive.get(),
				otherCollider->collisionPrimitive.get(),
				currCollider->offset * currTransform->getWorldTransform(),
				otherCollider->offset * otherTransform->getWorldTransform(),
				contactInfo,
				secondInfo)) {

				if (Vector3::Up.Dot(contactInfo.contactNormal) > 0.1f) {
					charController->bOnGround = true;
					currTransform->translate(Vector3::Up * contactInfo.contactNormal.y * contactInfo.penetrationDepth);
					charController->lastHitGrounds.push_back(potentialCollider);
				} else {
					currTransform->translate(contactInfo.contactNormal * contactInfo.penetrationDepth);
				}

				invoke<EventCertainTransformUpdate>(currTransform, currTransform->parent_.get());
				spatialHashmap.updateDynamicEntity(currCharacterControllerEntity);
			}
		}
	}

	std::vector<CollisionInfo> staticCollisions;	// Collisions of rigidbodies with static geometry in the scene
	{
		for (auto rigidbody = rigidbodies.begin(); rigidbody != rigidbodies.end(); ++rigidbody) {
			const int currRigidbodyIdx = *rigidbody;
			Entity currRigidbodyEntity = colliders[currRigidbodyIdx];
			Transform* currTransform = currRigidbodyEntity.getComponent<Transform>().get();
			Collider* currCollider = currRigidbodyEntity.getComponent<Collider>().get();
			BoundingBox currBox = currTransform->getTransformedBoundingBox();
			Rigidbody* currRigidbody = currRigidbodyEntity.getComponent<Rigidbody>().get();

			std::set<Entity> potentialColliders = spatialHashmap.queryEntities(currBox);
			potentialColliders.erase(currRigidbodyEntity);

			for (auto it = potentialColliders.begin(); it != potentialColliders.end(); ++it) {
				Entity potentialCollider = *it;

				if (potentialCollider.hasComponent<Rigidbody>() && !potentialCollider.getComponent<Rigidbody>()->constraints.contains(ERigidbodyConstraints::Freeze)) {
					continue;
				}
				if (potentialCollider.hasComponent<CharacterController>()) {
					continue;
				}

				Transform* otherTransform = potentialCollider.getComponent<Transform>().get();
				Collider* otherCollider = potentialCollider.getComponent<Collider>().get();
				if (!Physics::collisionTree[static_cast<uint8>(currCollider->layer)][static_cast<uint8>(otherCollider->layer)]) { continue; }

				// Broad phase collision check
				const BoundingBox otherBox = otherTransform->getTransformedBoundingBox();
				if (!currBox.Intersects(otherBox)) {
					continue;
				}

				// Narrow phase collision check
				Contact contactInfo;
				Contact secondInfo;
				if (CollisionTest::checkIntersect(currCollider->collisionPrimitive.get(),
					otherCollider->collisionPrimitive.get(),
					currCollider->offset * currTransform->getWorldTransform(),
					otherCollider->offset * otherTransform->getWorldTransform(),
					contactInfo,
					secondInfo)
					) {
					CollisionInfo info;
					info.bodies[0] = currRigidbodyEntity;
					info.bodies[1] = potentialCollider;
					info.contact = contactInfo;
					info.secondBestContact = secondInfo;

					staticCollisions.push_back(info);
				}
			}
		}
	}

	const int staticRigidbodiesSize = staticCollisions.size();

	// Resolve rigidbody with static colliders
	for (int i = 0; i < staticRigidbodiesSize; i++) {
		CollisionInfo& currInfo = staticCollisions[i];

		if (std::abs(currInfo.contact.contactNormal.Dot(Vector3::Up)) > 0.98f) {
			currInfo.contact.contactNormal = Vector3::Up * (currInfo.contact.contactNormal.Dot(Vector3::Up) > 0.0f ? 1.0f : -1.0f);
		} else if (std::abs(currInfo.secondBestContact.contactNormal.Dot(Vector3::Up)) > 0.99f) {
			currInfo.contact.contactNormal = Vector3::Up * (currInfo.contact.contactNormal.Dot(Vector3::Up) > 0.0f ? 1.0f : -1.0f);
		}

		Rigidbody* currRigidbody = currInfo.bodies[0].getComponent<Rigidbody>().get();
		Transform* currTransform = currInfo.bodies[0].getComponent<Transform>().get();

		Vector3 coordinateVectors[3];
		coordinateVectors[0] = currTransform->getWorldRight();
		coordinateVectors[1] = currTransform->getWorldUp();
		coordinateVectors[2] = currTransform->getWorldForward();

		float biggestDotProduct = 0.0f;
		int biggestIdx = -1;

		Quaternion deltaRotation;
		Vector3 deltaRotationEuler;

		Vector3 deltaPosition = currInfo.contact.contactNormal * (currInfo.contact.penetrationDepth - 0.005f);

		// Calculate delta rotation due to collision
		{
			for (int i = 0; i < 3; i++) {
				coordinateVectors[i].Normalize();
				float dotProduct = currInfo.contact.contactNormal.Dot(coordinateVectors[i]);
				if (std::abs(dotProduct) > std::abs(biggestDotProduct)) {
					biggestDotProduct = dotProduct;
					biggestIdx = i;
				}
			}

			if (std::abs(biggestDotProduct) < 0.9999f) {
				Vector3 rotatedVector = coordinateVectors[biggestIdx] * (biggestDotProduct > 0.0f ? 1.0f : -1.0f);
				rotatedVector.Normalize();
				Vector3 rotationAxis = rotatedVector.Cross(currInfo.contact.contactNormal);

				float angle = std::abs(1.58f - std::acos(rotationAxis.Length()));

				float s = std::sqrt(1 + std::abs(biggestDotProduct) * 2.0f);
				float invs = 1.0f / s;
				deltaRotation = Quaternion(
					rotationAxis.x * invs,
					rotationAxis.y * invs,
					rotationAxis.z * invs,
					s * 0.5f
				);
				deltaRotation.Normalize();
				deltaRotation = deltaRotation.Slerp(Quaternion::Identity, deltaRotation, std::min(0.5f / angle, 0.1f));
				deltaRotationEuler = rotationAxis * std::min(0.5f / angle, 0.2f);

				Quaternion inverseRotation;
				currTransform->getWorldRotation().Inverse(inverseRotation);
				deltaRotation = Quaternion::Concatenate(Quaternion::Concatenate(inverseRotation, deltaRotation), currTransform->getWorldRotation());
				deltaRotation.Normalize();
			}
		}

		float dotProduct = currRigidbody->velocity.Dot(currInfo.contact.contactNormal);
		float currSign = (dotProduct > 0.0f ? 1.0f : -1.0f);
		Vector3 planeVelocity = currRigidbody->velocity - currInfo.contact.contactNormal * dotProduct;
		Vector3 normalVelocity = currRigidbody->velocity - planeVelocity;
		Vector3 finalNormalVelocity = (normalVelocity.Length() > 7.0f ? 6.5f * currSign * currInfo.contact.contactNormal : 0.8f * normalVelocity);

		currRigidbody->velocity = planeVelocity + finalNormalVelocity;

		currTransform->translate(deltaPosition);
		currTransform->rotate(deltaRotation);

		invoke<EventCertainTransformUpdate>(currTransform, currTransform->parent_.get());
		spatialHashmap.updateDynamicEntity(currInfo.bodies[0]);

		// Calculate angular momentum delta
		{
			Vector3 currTargetNormal = Vector3(biggestDotProduct > 0.0f ? 1.0f : -1.0f);
			switch (biggestIdx) {
			case 0:
				currTargetNormal *= currTransform->getWorldRight();
				break;
			case 1:
				currTargetNormal *= currTransform->getWorldUp();
				break;
			case 2:
				currTargetNormal *= currTransform->getWorldForward();
			}

			if (std::abs(currInfo.contact.contactNormal.Dot(currTargetNormal)) > 0.99f) {
				currRigidbody->angularMomentum = 0.2f * currInfo.contact.contactNormal * currRigidbody->angularMomentum.Dot(currInfo.contact.contactNormal);
			} else {
				Vector3 eulerDeltaRotation = deltaRotationEuler - currInfo.contact.contactNormal * std::abs(currInfo.contact.contactNormal.Dot(deltaRotationEuler));
				float biggestComponent = 0.0f;
				float biggestComponentIdx = -1;
				for (int i = 0; i < 3; i++) {
					if (eulerDeltaRotation[i] > PI) {
						eulerDeltaRotation[i] -= 2.0f * PI;
					}

					if (std::abs(eulerDeltaRotation[i] > std::abs(biggestComponent))) {
						biggestComponentIdx = i;
						biggestComponent = eulerDeltaRotation[i];
					}

					if (std::abs(eulerDeltaRotation[i]) < 0.05f) {
						eulerDeltaRotation[i] = 0.0f;
					}
				}

				Vector3 deltaAngularMomentum = eulerDeltaRotation;
				float deltaAngularMomentumLength = deltaAngularMomentum.Length();
				deltaAngularMomentum.Normalize();
				float finalMagnitude = std::min(deltaAngularMomentumLength * 24.0f, 12.5f * PI * FTime::fixedDeltaTime);
				finalMagnitude *= currRigidbody->mass * (1.0f - std::abs(currInfo.contact.contactNormal.Dot(Vector3::Up)));

				currRigidbody->angularMomentum += deltaAngularMomentum * finalMagnitude;
			}
		}
	}

	for (int i = 0; i < staticRigidbodiesSize; i++) {
		CollisionInfo& currInfo = staticCollisions[i];
		Rigidbody* currRigidbody = currInfo.bodies[0].getComponent<Rigidbody>().get();
		Transform* currTransform = currInfo.bodies[0].getComponent<Transform>().get();

		Vector3 planeVelocity = currRigidbody->velocity - currInfo.contact.contactNormal * std::abs(currRigidbody->velocity.Dot(currInfo.contact.contactNormal));
		Vector3 frictionVelocity = currInfo.friction * planeVelocity;
		Vector3 velocityChange = -frictionVelocity / std::min(currRigidbody->mass, 2.0f);

		currRigidbody->velocity += velocityChange;
	}
}

void ft_engine::ResolveConstraintsSystem::resolveDynamicCollisions(std::vector<eecs::Entity>& colliders, std::set<int>& rigidbodies, std::set<int>& characterControllers, std::set<int>& fixedJoints) {
	// Resolve fixedJoints
	for (auto& currFixedJointIdx : fixedJoints) {
		Entity currFixedJointEntity = colliders[currFixedJointIdx];
		Transform* currTransform = currFixedJointEntity.getComponent<Transform>().get();
		Collider* currCollider = currFixedJointEntity.getComponent<Collider>().get();
		Transform* parentTransform = currFixedJointEntity.getComponent<FixedJoint>()->carryingEntity.getComponent<Transform>().get();
		CharacterController* parentCharacterController = currFixedJointEntity.getComponent<FixedJoint>()->carryingEntity.getComponent<CharacterController>().get();
		BoundingBox currBox = currTransform->getTransformedBoundingBox();
		std::set<Entity> potentialColliders = spatialHashmap.queryEntities(currBox);
		potentialColliders.erase(currFixedJointEntity);

		for (auto potentialCollider : potentialColliders) {
			Transform* otherTransform = potentialCollider.getComponent<Transform>().get();
			Collider* otherCollider = potentialCollider.getComponent<Collider>().get();
			if (!Physics::collisionTree[static_cast<uint8>(currCollider->layer)][static_cast<uint8>(otherCollider->layer)]) { continue; }

			Rigidbody* otherRigidbody = nullptr;
			CharacterController* otherCharacterController = nullptr;

			bool bRigidbody = false;
			if (potentialCollider.hasComponent<Rigidbody>()) {
				otherRigidbody = potentialCollider.getComponent<Rigidbody>().get();
				if (otherRigidbody->constraints.contains(ERigidbodyConstraints::Freeze)) {
					continue;
				}
				bRigidbody = true;
			} else if (potentialCollider.hasComponent<CharacterController>()) {
				otherCharacterController = potentialCollider.getComponent<CharacterController>().get();
			} else {
				continue;
			}

			const BoundingBox otherBox = otherTransform->getTransformedBoundingBox();
			if (!currBox.Intersects(otherBox)) {
				continue;
			}

			Contact contactInfo;
			Contact secondInfo;
			if (CollisionTest::checkIntersect(currCollider->collisionPrimitive.get(),
				otherCollider->collisionPrimitive.get(),
				currCollider->offset * currTransform->getWorldTransform(),
				otherCollider->offset * otherTransform->getWorldTransform(),
				contactInfo,
				secondInfo)) {
				CollisionInfo info;
				info.bodies[0] = currFixedJointEntity;
				info.bodies[1] = potentialCollider;
				info.contact = contactInfo;
				info.secondBestContact = secondInfo;

				Vector3 firstVelocity = (parentCharacterController ? parentCharacterController->lastVelocity : Vector3::Zero);
				Vector3 secondVelocity = (bRigidbody ? otherRigidbody->velocity : otherCharacterController->lastVelocity);

				bool bIsMovingTowardsContact[2] = { true, true };
				if (firstVelocity.LengthSquared() < 0.05f || firstVelocity.Dot(-info.contact.contactNormal) < 0.1f) {
					bIsMovingTowardsContact[0] = false;
				}
				if (secondVelocity.LengthSquared() < 0.05f || secondVelocity.Dot(info.contact.contactNormal) < 0.1f) {
					bIsMovingTowardsContact[1] = false;
				}

				if (!bIsMovingTowardsContact[0] && !bIsMovingTowardsContact[1]) {
					auto hasCollided = pastImmobileEntityMobileEntityCollisions.find(std::make_pair(currFixedJointEntity, potentialCollider));
					if (hasCollided != pastImmobileEntityMobileEntityCollisions.end()) {
						bIsMovingTowardsContact[hasCollided->second] = true;
					}
					if (firstVelocity.LengthSquared() < secondVelocity.LengthSquared()) {
						bIsMovingTowardsContact[0] = true;
					} else {
						bIsMovingTowardsContact[1] = true;
					}
				}

				if (bIsMovingTowardsContact[0] && bIsMovingTowardsContact[1]) {
					float firstMagnitude = firstVelocity.Length();
					float secondMagnitude = secondVelocity.Length();
					float magnitudeSum = firstMagnitude + secondMagnitude;

					info.penetrationCoefficients[0] = firstMagnitude / magnitudeSum;
					info.penetrationCoefficients[1] = secondMagnitude / magnitudeSum;

					info.velocityDeltas[0] = info.contact.contactNormal * std::abs(firstVelocity.Dot(info.contact.contactNormal));
					info.velocityDeltas[1] = -info.contact.contactNormal * std::abs(secondVelocity.Dot(info.contact.contactNormal));
				} else if (bIsMovingTowardsContact[0]) {
					info.penetrationCoefficients[0] = 1.0f;
					info.penetrationCoefficients[1] = 0.0f;
					info.velocityDeltas[0] = info.contact.contactNormal * std::abs(firstVelocity.Dot(info.contact.contactNormal));

					pastImmobileEntityMobileEntityCollisions[std::make_pair(currFixedJointEntity, potentialCollider)] = 0;
				} else if (bIsMovingTowardsContact[1]) {
					info.penetrationCoefficients[0] = 0.0f;
					info.penetrationCoefficients[1] = 1.0f;
					info.velocityDeltas[1] = -info.contact.contactNormal * std::abs(secondVelocity.Dot(info.contact.contactNormal));

					pastImmobileEntityMobileEntityCollisions[std::make_pair(currFixedJointEntity, potentialCollider)] = 1;
				}

				resolveFixedJoint(currTransform, parentTransform, info.penetrationCoefficients[0] * contactInfo.contactNormal * contactInfo.penetrationDepth);
				if (bRigidbody) {
					resolveDynamicRigidbody(potentialCollider,
						potentialCollider.getComponent<Rigidbody>().get(),
						otherTransform,
						-1.0f * info.contact.contactNormal,
						info.velocityDeltas[1],
						info.contact.penetrationDepth,
						info.penetrationCoefficients[1]);
				} else {
					if (Vector3::Up.Dot(-1.0f * contactInfo.contactNormal) > 0.1f) {
						potentialCollider.getComponent<CharacterController>()->bOnGround = true;
						potentialCollider.getComponent<CharacterController>()->lastHitGrounds.push_back(currFixedJointEntity);
						otherTransform->translate(info.penetrationCoefficients[1] * Vector3::Up * -1.0f * contactInfo.contactNormal.y * contactInfo.penetrationDepth);
					} else {
						otherTransform->translate(info.penetrationCoefficients[1] * -contactInfo.contactNormal * contactInfo.penetrationDepth);
					}

					invoke<EventCertainTransformUpdate>(otherTransform, otherTransform->parent_.get());
					spatialHashmap.updateDynamicEntity(potentialCollider);
				}
			}
		}
	}

	std::vector<CollisionInfo> dynamicRigidbodyCollisions;	// Collisions of rigidbodies with rigidbodies
	{
		for (auto rigidbody = rigidbodies.begin(); rigidbody != rigidbodies.end(); ++rigidbody) {
			const int currRigidbodyIdx = *rigidbody;
			Entity currRigidbodyEntity = colliders[currRigidbodyIdx];
			Transform* currTransform = currRigidbodyEntity.getComponent<Transform>().get();
			Collider* currCollider = currRigidbodyEntity.getComponent<Collider>().get();
			BoundingBox currBox = currTransform->getTransformedBoundingBox();
			Rigidbody* currRigidbody = currRigidbodyEntity.getComponent<Rigidbody>().get();

			std::set<Entity> potentialColliders = spatialHashmap.queryEntities(currBox);
			potentialColliders.erase(currRigidbodyEntity);

			for (auto it = potentialColliders.begin(); it != potentialColliders.end(); ++it) {
				Entity potentialCollider = *it;

				// Make sure we don't have duplicates with the same rigidbodies
				bool bRigidbody = false;
				if (potentialCollider.hasComponent<Rigidbody>()) {
					if (potentialCollider < currRigidbodyEntity) {
						continue;
					}
					bRigidbody = true;
					if (potentialCollider.getComponent<Rigidbody>()->constraints.contains(ERigidbodyConstraints::Freeze)) {
						bRigidbody = false;
					}
				}

				if (!bRigidbody) {
					continue;
				}

				Transform* otherTransform = potentialCollider.getComponent<Transform>().get();
				Collider* otherCollider = potentialCollider.getComponent<Collider>().get();
				if (!Physics::collisionTree[static_cast<uint8>(currCollider->layer)][static_cast<uint8>(otherCollider->layer)]) { continue; }

				// Broad phase collision check
				const BoundingBox otherBox = otherTransform->getTransformedBoundingBox();
				if (!currBox.Intersects(otherBox)) {
					continue;
				}

				// Narrow phase collision check
				Contact contactInfo;
				Contact secondInfo;
				if (CollisionTest::checkIntersect(currCollider->collisionPrimitive.get(),
					otherCollider->collisionPrimitive.get(),
					currCollider->offset * currTransform->getWorldTransform(),
					otherCollider->offset * otherTransform->getWorldTransform(),
					contactInfo,
					secondInfo)
					) {

					CollisionInfo info;
					info.bodies[0] = currRigidbodyEntity;
					info.bodies[1] = potentialCollider;
					info.contact = contactInfo;
					info.secondBestContact = secondInfo;

					Rigidbody* otherRigidbody = potentialCollider.getComponent<Rigidbody>().get();

					bool bIsMovingTowardsContact[2] = { true, true };

					if (currRigidbody->velocity.LengthSquared() < 0.05f || currRigidbody->velocity.Dot(-info.contact.contactNormal) < 0.1f) {
						bIsMovingTowardsContact[0] = false;
					}
					if (otherRigidbody->velocity.LengthSquared() < 0.05f || otherRigidbody->velocity.Dot(info.contact.contactNormal) < 0.1f) {
						bIsMovingTowardsContact[1] = false;
					}

					if (!bIsMovingTowardsContact[0] && !bIsMovingTowardsContact[1]) {
						auto hasCollided = pastImmobileEntityMobileEntityCollisions.find(std::make_pair(currRigidbodyEntity, potentialCollider));
						if (hasCollided != pastImmobileEntityMobileEntityCollisions.end()) {
							bIsMovingTowardsContact[hasCollided->second] = true;
							//CLOG("IT HAPPENED ", hasCollided->second, '\n');
						} else if (currRigidbody->velocity.LengthSquared() < otherRigidbody->velocity.LengthSquared()) {
							bIsMovingTowardsContact[0] = true;
						} else {
							bIsMovingTowardsContact[1] = true;
						}
					}

					if (bIsMovingTowardsContact[0] && bIsMovingTowardsContact[1]) {
						float firstMagnitude = currRigidbody->velocity.Length();
						float secondMagnitude = otherRigidbody->velocity.Length();
						float magnitudeSum = firstMagnitude + secondMagnitude;

						info.penetrationCoefficients[0] = firstMagnitude / magnitudeSum;
						info.penetrationCoefficients[1] = secondMagnitude / magnitudeSum;

						info.velocityDeltas[0] = info.contact.contactNormal * std::abs(currRigidbody->velocity.Dot(info.contact.contactNormal));
						info.velocityDeltas[1] = -info.contact.contactNormal * std::abs(otherRigidbody->velocity.Dot(info.contact.contactNormal));
					} else if (bIsMovingTowardsContact[0]) {
						info.penetrationCoefficients[0] = 1.0f;
						info.penetrationCoefficients[1] = 0.0f;
						info.velocityDeltas[0] = info.contact.contactNormal * std::abs(currRigidbody->velocity.Dot(info.contact.contactNormal));

						pastImmobileEntityMobileEntityCollisions[std::make_pair(currRigidbodyEntity, potentialCollider)] = 0;
					} else if (bIsMovingTowardsContact[1]) {
						info.penetrationCoefficients[0] = 0.0f;
						info.penetrationCoefficients[1] = 1.0f;
						info.velocityDeltas[1] = -info.contact.contactNormal * std::abs(otherRigidbody->velocity.Dot(info.contact.contactNormal));

						pastImmobileEntityMobileEntityCollisions[std::make_pair(currRigidbodyEntity, potentialCollider)] = 1;
					}

					dynamicRigidbodyCollisions.push_back(info);
					resolveDynamicCollision(info);
				}
			}
		}
	}

	for (auto characterController = characterControllers.begin(); characterController != characterControllers.end(); ++characterController) {
		const int currCharacterControllerIdx = *characterController;
		Entity currCharacterControllerEntity = colliders[currCharacterControllerIdx];
		Transform* currTransform = currCharacterControllerEntity.getComponent<Transform>().get();
		Collider* currCollider = currCharacterControllerEntity.getComponent<Collider>().get();
		CharacterController* charController = currCharacterControllerEntity.getComponent<CharacterController>().get();
		BoundingBox currBox = currTransform->getTransformedBoundingBox();
		std::set<Entity> potentialColliders = spatialHashmap.queryEntities(currBox);
		potentialColliders.erase(currCharacterControllerEntity);

		for (auto it = potentialColliders.begin(); it != potentialColliders.end(); ++it) {
			Entity potentialCollider = *it;
			Transform* otherTransform = potentialCollider.getComponent<Transform>().get();
			Collider* otherCollider = potentialCollider.getComponent<Collider>().get();
			if (!Physics::collisionTree[static_cast<uint8>(currCollider->layer)][static_cast<uint8>(otherCollider->layer)]) { continue; }

			bool bRigidbody = false;
			if (potentialCollider.hasComponent<Rigidbody>()) {
				if (potentialCollider.getComponent<Rigidbody>()->constraints.contains(ERigidbodyConstraints::Freeze)) {
					continue;
				}
				bRigidbody = true;
			} else if (!potentialCollider.hasComponent<CharacterController>()) {
				continue;
			}

			const BoundingBox otherBox = otherTransform->getTransformedBoundingBox();
			if (!currBox.Intersects(otherBox)) {
				continue;
			}

			Contact contactInfo;
			Contact secondInfo;
			if (CollisionTest::checkIntersect(currCollider->collisionPrimitive.get(),
				otherCollider->collisionPrimitive.get(),
				currCollider->offset * currTransform->getWorldTransform(),
				otherCollider->offset * otherTransform->getWorldTransform(),
				contactInfo,
				secondInfo)) {

				CollisionInfo info;
				info.contact = contactInfo;
				info.secondBestContact = secondInfo;

				Vector3 firstVelocity = charController->lastVelocity;
				Vector3 secondVelocity = (bRigidbody ? potentialCollider.getComponent<Rigidbody>()->velocity : potentialCollider.getComponent<CharacterController>()->lastVelocity);

				bool bIsMovingTowardsContact[2] = { true, true };
				if (firstVelocity.LengthSquared() < 0.05f || firstVelocity.Dot(-info.contact.contactNormal) < 0.1f) {
					bIsMovingTowardsContact[0] = false;
				}

				if (secondVelocity.LengthSquared() < 0.05f || secondVelocity.Dot(info.contact.contactNormal) < 0.1f) {
					bIsMovingTowardsContact[1] = false;
				}

				if (!bIsMovingTowardsContact[0] && !bIsMovingTowardsContact[1]) {
					auto hasCollided = pastImmobileEntityMobileEntityCollisions.find(std::make_pair(currCharacterControllerEntity, potentialCollider));
					if (hasCollided != pastImmobileEntityMobileEntityCollisions.end()) {
						bIsMovingTowardsContact[hasCollided->second] = true;
						//CLOG("IT HAPPENED ", hasCollided->second, '\n');
					}
					if (firstVelocity.LengthSquared() < secondVelocity.LengthSquared()) {
						bIsMovingTowardsContact[0] = true;
					} else {
						bIsMovingTowardsContact[1] = true;
					}
				}

				if (bIsMovingTowardsContact[0] && bIsMovingTowardsContact[1]) {
					float firstMagnitude = firstVelocity.Length();
					float secondMagnitude = secondVelocity.Length();
					float magnitudeSum = firstMagnitude + secondMagnitude;

					info.penetrationCoefficients[0] = firstMagnitude / magnitudeSum;
					info.penetrationCoefficients[1] = secondMagnitude / magnitudeSum;

					info.velocityDeltas[0] = info.contact.contactNormal * std::abs(firstVelocity.Dot(info.contact.contactNormal));
					info.velocityDeltas[1] = -info.contact.contactNormal * std::abs(secondVelocity.Dot(info.contact.contactNormal));
				} else if (bIsMovingTowardsContact[0]) {
					info.penetrationCoefficients[0] = 1.0f;
					info.penetrationCoefficients[1] = 0.0f;
					info.velocityDeltas[0] = info.contact.contactNormal * std::abs(firstVelocity.Dot(info.contact.contactNormal));

					pastImmobileEntityMobileEntityCollisions[std::make_pair(currCharacterControllerEntity, potentialCollider)] = 0;
				} else if (bIsMovingTowardsContact[1]) {
					info.penetrationCoefficients[0] = 0.0f;
					info.penetrationCoefficients[1] = 1.0f;
					info.velocityDeltas[1] = -info.contact.contactNormal * std::abs(secondVelocity.Dot(info.contact.contactNormal));

					pastImmobileEntityMobileEntityCollisions[std::make_pair(currCharacterControllerEntity, potentialCollider)] = 1;
				}

				if (Vector3::Up.Dot(contactInfo.contactNormal) > 0.1f) {
					charController->bOnGround = true;
					currTransform->translate(info.penetrationCoefficients[0] * Vector3::Up * contactInfo.contactNormal.y * contactInfo.penetrationDepth);
					charController->lastHitGrounds.push_back(potentialCollider);
				} else {
					currTransform->translate(info.penetrationCoefficients[0] * contactInfo.contactNormal * contactInfo.penetrationDepth);
				}

				if (bRigidbody) {
					resolveDynamicRigidbody(potentialCollider,
						potentialCollider.getComponent<Rigidbody>().get(),
						otherTransform,
						-1.0f * info.contact.contactNormal,
						info.velocityDeltas[1],
						info.contact.penetrationDepth,
						info.penetrationCoefficients[1]);
				} else {
					if (Vector3::Up.Dot(-1.0f * contactInfo.contactNormal) > 0.1f) {
						potentialCollider.getComponent<CharacterController>()->bOnGround = true;
						potentialCollider.getComponent<CharacterController>()->lastHitGrounds.push_back(currCharacterControllerEntity);
						otherTransform->translate(info.penetrationCoefficients[1] * Vector3::Up * -1.0f * contactInfo.contactNormal.y * contactInfo.penetrationDepth);
					} else {
						otherTransform->translate(info.penetrationCoefficients[1] * -contactInfo.contactNormal * contactInfo.penetrationDepth);
					}

					invoke<EventCertainTransformUpdate>(otherTransform, otherTransform->parent_.get());
					spatialHashmap.updateDynamicEntity(potentialCollider);
				}

				invoke<EventCertainTransformUpdate>(currTransform, currTransform->parent_.get());
				spatialHashmap.updateDynamicEntity(currCharacterControllerEntity);
			}
		}
	}

	// Apply friction
	const int dynamicRigidbodiesSize = dynamicRigidbodyCollisions.size();
	{
		for (int i = 0; i < dynamicRigidbodiesSize; i++) {
			CollisionInfo& currInfo = dynamicRigidbodyCollisions[i];

			for (int j = 0; j < 2; j++) {
				Rigidbody* currRigidbody = currInfo.bodies[j].getComponent<Rigidbody>().get();
				Transform* currTransform = currInfo.bodies[j].getComponent<Transform>().get();

				float currSign = (j == 0 ? 1.0f : -1.0f);
				float dotProduct = currRigidbody->velocity.Dot(currSign * currInfo.contact.contactNormal);
				Vector3 planeVelocity = currRigidbody->velocity - currInfo.contact.contactNormal * (dotProduct * currSign);
				if (planeVelocity.LengthSquared() < 0.1f) {
					currRigidbody->velocity -= planeVelocity;
				} else {
					Vector3 frictionVelocity = std::min(currInfo.friction, 0.8f) * planeVelocity;
					Vector3 velocityChange = -frictionVelocity / std::min(currRigidbody->mass, 2.0f);

					currRigidbody->velocity += velocityChange;
				}
			}
		}
	}
}

void ft_engine::ResolveConstraintsSystem::resolveDynamicCollision(CollisionInfo& currInfo) {

	if (std::abs(currInfo.contact.contactNormal.Dot(Vector3::Up)) > 0.97f) {
		currInfo.contact.contactNormal = Vector3::Up * (currInfo.contact.contactNormal.Dot(Vector3::Up) > 0.0f ? 1.0f : -1.0f);
	} else if (std::abs(currInfo.secondBestContact.contactNormal.Dot(Vector3::Up)) > 0.97f) {
		currInfo.contact.contactNormal = Vector3::Up * (currInfo.contact.contactNormal.Dot(Vector3::Up) > 0.0f ? 1.0f : -1.0f);
	}

	for (int j = 0; j < 2; j++) {
		Rigidbody* currRigidbody = currInfo.bodies[j].getComponent<Rigidbody>().get();
		Transform* currTransform = currInfo.bodies[j].getComponent<Transform>().get();

		float sign = (j == 0 ? 1.0f : -1.0f);
		resolveDynamicRigidbody(currInfo.bodies[j],
			currRigidbody,
			currTransform,
			sign * currInfo.contact.contactNormal,
			currInfo.velocityDeltas[j],
			currInfo.contact.penetrationDepth,
			currInfo.penetrationCoefficients[j]);
	}
}

void ft_engine::ResolveConstraintsSystem::resolveDynamicRigidbody(Entity entity, Rigidbody * currRigidbody, Transform * currTransform, const Vector3 & contactNormal, const Vector3& velocityDelta, float penetrationDepth, float penetrationCoefficient) {
	Vector3 coordinateVectors[3];
	coordinateVectors[0] = currTransform->getWorldRight();
	coordinateVectors[1] = currTransform->getWorldUp();
	coordinateVectors[2] = currTransform->getWorldForward();

	float biggestDotProduct = 0.0f;
	int biggestIdx = -1;

	Quaternion deltaRotation;
	Vector3 deltaRotationEuler;

	Vector3 deltaPosition = penetrationCoefficient * contactNormal * penetrationDepth;

	// Calculate delta rotation due to collision
	{
		for (int i = 0; i < 3; i++) {
			coordinateVectors[i].Normalize();
			float dotProduct = contactNormal.Dot(coordinateVectors[i]);
			if (std::abs(dotProduct) > std::abs(biggestDotProduct)) {
				biggestDotProduct = dotProduct;
				biggestIdx = i;
			}
		}

		if (std::abs(biggestDotProduct) < 0.9999f) {
			Vector3 rotatedVector = coordinateVectors[biggestIdx] * (biggestDotProduct > 0.0f ? 1.0f : -1.0f);
			rotatedVector.Normalize();
			Vector3 rotationAxis = rotatedVector.Cross(contactNormal);

			float angle = std::abs(1.58f - std::acos(rotationAxis.Length()));

			float s = std::sqrt(1 + std::abs(biggestDotProduct) * 2.0f);
			float invs = 1.0f / s;
			deltaRotation = Quaternion(
				rotationAxis.x * invs,
				rotationAxis.y * invs,
				rotationAxis.z * invs,
				s * 0.5f
			);
			deltaRotation.Normalize();
			deltaRotation = deltaRotation.Slerp(Quaternion::Identity, deltaRotation, std::min((0.5f / angle) * penetrationCoefficient, 0.1f));
			deltaRotationEuler = penetrationCoefficient * rotationAxis * std::min(0.5f / angle, 0.2f);

			Quaternion inverseRotation;
			currTransform->getWorldRotation().Inverse(inverseRotation);
			deltaRotation = Quaternion::Concatenate(Quaternion::Concatenate(inverseRotation, deltaRotation), currTransform->getWorldRotation());
			deltaRotation.Normalize();
		}
	}

	currRigidbody->velocity += 0.9f * velocityDelta;

	currTransform->translate(deltaPosition);
	currTransform->rotate(deltaRotation);

	invoke<EventCertainTransformUpdate>(currTransform, currTransform->parent_.get());
	spatialHashmap.updateDynamicEntity(entity);
}

void ft_engine::ResolveConstraintsSystem::resolveFixedJoint(Transform* fixedJointTransform, Transform* parentTransform, const Vector3& delta) {
	fixedJointTransform->translate(delta);
	Vector3 deltaParent = parentTransform->getWorldForward() * std::max(0.0f, delta.Dot(parentTransform->getWorldForward()));
	deltaParent.y = 0.0f;
	parentTransform->translate(deltaParent);

	invoke<EventCertainTransformUpdate>(fixedJointTransform, fixedJointTransform->parent_.get());
	invoke<EventCertainTransformUpdate>(parentTransform, parentTransform->parent_.get());
}