#pragma once

#include <Global.h>
#include <set>
#include <map>
#include "TransformSystem.h"
#include "SpatialHashmap.h"
#include "Collision.h"
#include "EECS.h"

struct PhysicsBulletData;

FLAVO_SYSTEM(ft_engine, ResolveConstraintsSystem)
namespace ft_engine
{
	class ResolveConstraintsSystem : public eecs::System<ResolveConstraintsSystem>, public eecs::IReceiver<ResolveConstraintsSystem>, public eecs::IInvoker
	{
	public:
		ResolveConstraintsSystem();
		~ResolveConstraintsSystem();

		void fixedUpdate(eecs::EntityManager& entities, double deltaTime) override;

		void onRaycastQuery(EventPhysicsRaycast& eventData);
		void onTriggerColliderQuery(EventPhysicsTriggerCollider& eventData);

	private:
		void onComponentAdded(const EventComponentAdded& eventData);

		void resolveStaticCollisions(std::vector<eecs::Entity>& colliders, std::set<int>& rigidbodies, std::set<int>& characterControllers, std::set<int>& fixedJoints);
		void resolveDynamicCollisions(std::vector<eecs::Entity>& colliders, std::set<int>& rigidbodies, std::set<int>& characterControllers, std::set<int>& fixedJoints);
		void resolveDynamicCollision(CollisionInfo& currInfo);
		void resolveDynamicRigidbody(Entity entity, Rigidbody* currRigidbody, Transform* currTransform, const Vector3& contactNormal, const Vector3& velocityDelta, float penetrationDepth, float penetrationCoefficient);
		void resolveFixedJoint(Transform* fixedJointTransform, Transform* parentTransform, const Vector3& delta);

		SpatialHashmap spatialHashmap;
		std::set<eecs::Entity> unhashedEntities;
		std::map<std::pair<eecs::Entity, eecs::Entity>, int8_t> pastImmobileEntityMobileEntityCollisions;

		std::unique_ptr<PhysicsBulletData> physicsBulletData;
	};
}