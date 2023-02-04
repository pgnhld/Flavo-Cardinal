#pragma once

#include "Global.h"
#include <memory>
#include "Physics/Transform.h"
#include "EECS.h"
#include "SystemManager.h"

namespace ft_engine
{
	class Scene
	{
	public:
		Scene();
		~Scene();

	public:
		void update(double deltaTime) const;
		void fixedUpdate(double deltaTime) const;

		void addSystem(eecs::SystemBase* system, double period) const;
		eecs::Entity instantiate() const;
		eecs::Entity instantiate(uint64 index) const;
		eecs::Entity instantiateNetwork() const;
		eecs::Entity getEntity(eecs::Id id) const;
		bool destroy();
		bool destroyRecursively(eecs::Id entityId);
		bool destroyNetwork();
		bool assignEntityParent(uint64 child, uint64 parent);
		bool assignEntityParent(eecs::Entity child, eecs::Entity parent);
		bool assignTransformParent(eecs::ComponentHandle<Transform> child, eecs::ComponentHandle<Transform> parent);
		eecs::ComponentHandle<Transform> getRootTransform();
		eecs::EntityManager& getEntityManager() const;
		std::string& getScenePath();
		uint32& getSceneIndex();

	private:
		friend class SceneManager;

		std::string scenePath_;
		uint32 sceneIndex_;
		std::unique_ptr<eecs::EntityManager> entityManager_;
		std::unique_ptr<eecs::SystemManager> systemManager_;
		eecs::Entity rootEntity_;
	};
}
