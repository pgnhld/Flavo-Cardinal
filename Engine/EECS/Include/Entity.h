#pragma once

#include "Global.h"

#include "Id.h"
#include "Component.h"

namespace eecs
{
	class EntityManager;

	class Entity
	{
	public:
		Entity();
		Entity(EntityManager* manager, Id id);

		template<typename T>
		ComponentHandle<T> addComponent();
		template<typename T>
		bool hasComponent();
		template<typename T>
		ComponentHandle<T> getComponent();
		template<typename T>
		bool removeComponent();

		bool isValid() const;
		Id getId() const;
		bool operator==(const Entity& another) const;
		bool operator!=(const Entity& another) const;
		bool operator<(const Entity& another) const;
		string toString() const;

	private:
		friend class EntityManager;
		Id id_;
		EntityManager* manager_;
	};

	template<typename T>
	ComponentHandle<T> Entity::addComponent() {
		return manager_->addComponent<T>(id_);
	}

	template<typename T>
	bool Entity::hasComponent() {
		return manager_->hasComponent<T>(id_);
	}

	template<typename T>
	ComponentHandle<T> Entity::getComponent() {
		return manager_->getComponent<T>(id_);
	}

	template<typename T>
	bool Entity::removeComponent() {
		return manager_->removeComponent<T>(id_);
	}
}

namespace std
{
	template <>
	struct hash<eecs::Entity>
	{
		std::size_t operator()(const eecs::Entity& e) const {
			return e.getId().index_;
		}
	};

}
