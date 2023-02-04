#include "Physics/RigidbodySystem.h"
#include "Physics/FixedJoint.h"
#include "FTime.h"
#include "Assertion.h"
#include "Logger.h"

using framework::FTime;

ft_engine::RigidbodySystem::RigidbodySystem() {
}

ft_engine::RigidbodySystem::~RigidbodySystem() {
}

void ft_engine::RigidbodySystem::fixedUpdate(eecs::EntityManager& entities, double deltaTime) {
	std::vector<eecs::Entity> rigidbodies = entities.getEntitiesWithComponents<Rigidbody, Transform>();

	const auto rigidbodiesSize = rigidbodies.size();

	for (int i = 0; i < rigidbodiesSize; i++) {
		Entity currRigidbody = rigidbodies[i];
		eecs::ComponentHandle<Transform> transformHandle = currRigidbody.getComponent<Transform>();
		Transform* transform = transformHandle.get();
		Rigidbody* rigidbody = currRigidbody.getComponent<Rigidbody>().get();

		if (ASSERT_FAIL(transform, "Entity with invalid Transform") || ASSERT_FAIL(rigidbody, "Entity with invalid Rigidbody")) {
			continue;
		}

		if (rigidbody->constraints.contains(ERigidbodyConstraints::Freeze)) {
			rigidbody->accumulatedForce = Vector3::Zero;
			rigidbody->accumulatedTorque = Vector3::Zero;
			rigidbody->velocity = Vector3::Zero;
			rigidbody->angularMomentum = Vector3::Zero;
			continue;
		}

		if (rigidbody->constraints.contains(ERigidbodyConstraints::FixedJoint)) {
			rigidbody->accumulatedForce = Vector3::Zero;
			rigidbody->accumulatedTorque = Vector3::Zero;
			rigidbody->velocity = Vector3::Zero;
			rigidbody->angularMomentum = Vector3::Zero;

			eecs::ComponentHandle<FixedJoint> pickableHandle = currRigidbody.getComponent<FixedJoint>();
			FixedJoint* pickable = pickableHandle.get();
			eecs::ComponentHandle<Transform> fixedParentHandle = pickable->carryingEntity.getComponent<Transform>();
			Transform* fixedParent = fixedParentHandle.get();
			Vector3 forwardVector = -pickable->lookWorldMatrix.Forward();
			forwardVector.Normalize();

			const float ZXRatio = forwardVector.z / forwardVector.x;
			forwardVector.y = MathsHelper::Clamp(forwardVector.y, pickable->minimumRelativeOffsetY, 1.0f);
			const float restOfIdentityVector = 1.0f - forwardVector.y * forwardVector.y;
			forwardVector.x = std::sqrt(restOfIdentityVector / (1 + ZXRatio * ZXRatio)) * (forwardVector.x > 0.0f ? 1.0f : -1.0f);
			forwardVector.z = std::sqrt(restOfIdentityVector - forwardVector.x * forwardVector.x) * (forwardVector.z > 0.0f ? 1.0f : -1.0f);

			const Vector3 newPosition = fixedParent->getWorldPosition() + Vector3::Up * pickable->localTargetOffset.y + forwardVector * pickable->localTargetOffset.z;

			if (!pickable->bHandledToDestination) {
				if (!CoroutineManager::isRunning(pickable->pullCoroutine)) {
					PullFixedJointCoroutineData* data = new PullFixedJointCoroutineData();
					data->pickable = pickableHandle;
					data->pickable->bLookMatrixAssigned = false;
					data->parentTransform = fixedParentHandle;
					data->pickableTransform = transformHandle;
					pickable->pullCoroutine = START_COROUTINE(
						&RigidbodySystem::pullFixedJointCoroutine,
						PullFixedJointCoroutineData*,
						data
					);
				}
			} else {
				if ((transform->getLocalPosition() - newPosition).LengthSquared() < pickable->maximumJointRange) {
					transform->setLocalTransform(Matrix::Compose(newPosition, Quaternion::CreateFromRotationMatrix(Matrix::CreateWorld(
						Vector3::Zero,
						pickableHandle->worldVectorSigns[0] * pickableHandle->lookWorldMatrix.GetVectorNormalized(pickableHandle->worldVectorIndices[0]),
						pickableHandle->worldVectorSigns[1] * pickableHandle->lookWorldMatrix.GetVectorNormalized(pickableHandle->worldVectorIndices[1])
					)),
						transform->getWorldScale()));
				} else {
					// Drop fixed jointed object due to too big difference between current position and destination
					pickable->bBroken = true;
				}
			}

			continue;
		}

		// Linear movement
		if (!rigidbody->constraints.contains(ERigidbodyConstraints::FreezeGravity)) {
			rigidbody->accumulatedForce += Physics::gravity * rigidbody->mass;
		}

		rigidbody->velocity += rigidbody->accumulatedForce * (FTime::fixedDeltaTime / rigidbody->mass) - rigidbody->drag * rigidbody->velocity;
		rigidbody->velocity = MathsHelper::Hadamard(rigidbody->velocity, rigidbody->constraints.linearMovementConstraints());
		transform->translate(rigidbody->velocity * FTime::fixedDeltaTime);
		rigidbody->lastAccumulatedForce = rigidbody->accumulatedForce;
		rigidbody->accumulatedForce = Vector3::Zero;

		// Angular movement
		Quaternion inverseRotation;
		transform->getWorldRotation().Inverse(inverseRotation);

		rigidbody->angularMomentum += rigidbody->accumulatedTorque * FTime::fixedDeltaTime - rigidbody->angularDrag * rigidbody->angularMomentum;
		rigidbody->angularMomentum = MathsHelper::Hadamard(rigidbody->angularMomentum, rigidbody->constraints.angularMovementConstraints());
		Vector3 angularVelocity = rigidbody->angularMomentum * FTime::fixedDeltaTime / rigidbody->mass;
		Quaternion deltaRotation = Quaternion::CreateFromEuler(angularVelocity);
		deltaRotation = Quaternion::Concatenate(Quaternion::Concatenate(inverseRotation, deltaRotation), transform->getWorldRotation());

		transform->rotate(deltaRotation);
		rigidbody->accumulatedTorque = Vector3::Zero;
	}

	invoke<EventTransformUpdate>();
}

IEnumerator ft_engine::RigidbodySystem::pullFixedJointCoroutine(CoroutineArg arg) {
	PullFixedJointCoroutineData* data = static_cast<PullFixedJointCoroutineData*>(arg);
	while (!data->pickable->bLookMatrixAssigned) {
		YIELD_RETURN_NULL();
	}

	const Vector3 initialPosition = data->pickableTransform->getWorldPosition();
	const Quaternion initialRotation = data->pickableTransform->getWorldRotation();

	Vector3 cubesWorldVectors[2];
	cubesWorldVectors[0] = data->pickableTransform->getWorldVectorNormalized(2);
	cubesWorldVectors[1] = data->pickableTransform->getWorldVectorNormalized(1);

	for (int i = 0; i < 2; i++) {
		float biggestDotProduct = 0.0f;
		int bestIdx;
		for (int j = 0; j < 3; j++) {
			const float currDotProduct = data->pickable->lookWorldMatrix.GetVectorNormalized(j).Dot(cubesWorldVectors[i]);
			if (std::abs(currDotProduct) > std::abs(biggestDotProduct)) {
				biggestDotProduct = currDotProduct;
				bestIdx = j;
			}
		}
		data->pickable->worldVectorSigns[i] = (biggestDotProduct > 0.0f ? 1.0f : -1.0f);
		data->pickable->worldVectorIndices[i] = bestIdx;
	}

	const double maxPullTime = data->pickable->pullTime;
	double pullTimer = 0.0f;
	while (pullTimer < maxPullTime) {
		if (ASSERT_FAIL(data->pickableTransform.isValid(), "FixedJoint Transform is not valid"))
			break;
		if (ASSERT_FAIL(data->parentTransform.isValid(), "Parent Transform is not valid"))
			break;
		if (ASSERT_FAIL(data->pickable.isValid(), "FixedJoint is not valid"))
			break;

		Vector3 forwardVector = -data->pickable->lookWorldMatrix.Forward();
		forwardVector.Normalize();

		const float ZXRatio = forwardVector.z / forwardVector.x;
		forwardVector.y = MathsHelper::Clamp(forwardVector.y, data->pickable->minimumRelativeOffsetY, 1.0f);
		const float restOfIdentityVector = 1.0f - forwardVector.y * forwardVector.y;
		forwardVector.x = std::sqrt(restOfIdentityVector / (1 + ZXRatio * ZXRatio)) * (forwardVector.x > 0.0f ? 1.0f : -1.0f);
		forwardVector.z = std::sqrt(restOfIdentityVector - forwardVector.x * forwardVector.x) * (forwardVector.z > 0.0f ? 1.0f : -1.0f);

		const Vector3 targetPosition = data->parentTransform->getWorldPosition() + Vector3::Up * data->pickable->localTargetOffset.y + forwardVector * data->pickable->localTargetOffset.z;
		const float currLerpCoefficient = pullTimer / maxPullTime;
		const Vector3 newPosition = Vector3::Lerp(initialPosition, targetPosition, currLerpCoefficient);
		const Quaternion newRotation = Quaternion::Slerp(
			initialRotation,
			Quaternion::CreateFromRotationMatrix(
				Matrix::CreateWorld(
					Vector3::Zero,
					data->pickable->worldVectorSigns[0] * data->pickable->lookWorldMatrix.GetVectorNormalized(data->pickable->worldVectorIndices[0]),
					data->pickable->worldVectorSigns[1] * data->pickable->lookWorldMatrix.GetVectorNormalized(data->pickable->worldVectorIndices[1])
				)),
			currLerpCoefficient);

		data->pickableTransform->setLocalTransform(
			Matrix::Compose(
				newPosition,
				newRotation,
				data->pickableTransform->getWorldScale()
			));

		YIELD_RETURN_NULL();
		pullTimer += FTime::deltaTime;
	}

	if (ASSERT_FAIL(data->pickable.isValid(), "FixedJoint is not valid"))
		YIELD_BREAK();

	data->pickable->bHandledToDestination = true;
}
