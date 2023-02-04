#pragma once

#include "Global.h"
#include "System.h"
#include "SceneManager.h"
#include "Events/IReceiver.h"
#include "EngineEvent.h"
#include "Transform.h"

FLAVO_SYSTEM(ft_engine, TransformSystem)
namespace ft_engine {
	class TransformSystem : public eecs::System<TransformSystem>, public eecs::IReceiver<TransformSystem> {
	public:
		TransformSystem();
		~TransformSystem();

		void update(eecs::EntityManager& entities, double deltaTime) override;
		void fixedUpdate(eecs::EntityManager& entities, double deltaTime) override;

	private:
		void updateTransform(const EventTransformUpdate& eventData);

		void updateCertainTransform(const EventCertainTransformUpdate& eventData);

		void updateAllTransforms();
		void updateTransforms(Transform* transform, const Matrix& parentWorldTransform, bool bIsDirty);
	};
}
