#pragma once

#include "Global.h"
#include "EECS.h"
#include "EngineEvent.h"
#include "Rigidbody.h"
#include "CoroutineManager.h"
#include "Physics/FixedJoint.h"

FLAVO_SYSTEM(ft_engine, RigidbodySystem)
namespace ft_engine
{
	class RigidbodySystem : public eecs::System<RigidbodySystem>, public eecs::IReceiver<RigidbodySystem>, public eecs::IInvoker {
	public:
		RigidbodySystem();
		~RigidbodySystem();

		void fixedUpdate(eecs::EntityManager& entities, double deltaTime) override;

	private:
		struct PullFixedJointCoroutineData;
		IEnumerator pullFixedJointCoroutine(CoroutineArg arg);
	};

	struct RigidbodySystem::PullFixedJointCoroutineData
	{
		eecs::ComponentHandle<FixedJoint> pickable;
		eecs::ComponentHandle<Transform> pickableTransform;
		eecs::ComponentHandle<Transform> parentTransform;
	};
}
