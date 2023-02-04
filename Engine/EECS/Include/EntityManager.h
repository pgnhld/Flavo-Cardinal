#pragma once

#include "Global.h"

#include <vector>
#include <bitset>

#include "Entity.h"
#include "Id.h"
#include "ByteArray.h"
#include "Component.h"
#include "Events/IInvoker.h"
#include "EECSEvents.h"
#include "Assertion.h"

namespace eecs
{
	namespace impl
	{
		template<typename ...T> struct VariadicParameterlessHelper {};
	}

	class EntityManager : public IInvoker
	{
	public:
		EntityManager();

		bool isValid(Id entityId);
		Entity create();
		Entity get(Id entityId);
		Entity get(uint64 index);
		bool destroy(Entity entity);
		bool destroy(Id entityId);

		template<typename T>
		ComponentHandle<T> addComponent(Entity entity);
		template<typename T>
		ComponentHandle<T> addComponent(Id entityId);
		template<typename T>
		ComponentHandle<T> getComponent(Entity entity);
		template<typename T>
		ComponentHandle<T> getComponent(Id entityId);
		template<typename T>
		bool removeComponent(Entity entity);
		template<typename T>
		bool removeComponent(Id entityId);
		template<typename T>
		bool hasComponent(Entity entity);
		template<typename T>
		bool hasComponent(Id entityId);
		template<typename T>
		T* getComponentPtr(Id entityId);
		template<typename... T>
		std::vector<Entity> getEntitiesWithComponents();
		template<typename... T>
		Entity getEntityWithComponents();

		/* [INTERNAL] To be used when deserializing */
		bool addComponent(uint64 index, ComponentBase* component, uint32 type);
		/* [INTERNAL] To be used when deserializing */
		bool removeComponent(uint64 index, uint32 type);
		/* [INTERNAL] */
		std::vector<uint32> getEntityActiveComponentsType(Id entityId);
		/* [INTERNAL] */
		ComponentBase* getComponentBase(Id entityId, uint32 type);
		/* [INTERNAL] Special case for root entity */
		Entity createRoot();
		/* [INTERNAL] To be used when deserializing */
		Entity create(uint64 index);

	private:
		template<typename T, typename... DT>
		void setComponentMask(std::bitset<MAX_COMPONENTS>& mask, impl::VariadicParameterlessHelper<T, DT...> args);
		template<typename T>
		void setComponentMask(std::bitset<MAX_COMPONENTS>& mask, impl::VariadicParameterlessHelper<T> arg);

		std::vector<Entity> getEntitiesWithComponentsNonTemplate(std::bitset<MAX_COMPONENTS> mask);
		Entity getEntityWithComponentsNonTemplate(std::bitset<MAX_COMPONENTS> mask);

	private:
		uint64 indexCounter_ = 0;

		/**
		 * \brief Mask used to quickly tell whether Entity has specific component
		 * \remarks Index of Vector: entity id
		 * \remarks Index of bitset: component type id
		 */
		std::vector<std::bitset<MAX_COMPONENTS>> componentMask_;

		/**
		* \brief Stores Components inside smart byte arrays
		* \remarks Index of Vector: component type id
		* \remarks Index of bitset: entity id
		*/
		std::vector<utils::ByteArrayBase*> componentArrays_;
		std::vector<uint32> entityIdCounters_;
		std::vector<uint64> availableIndexes_;
	};

	template<typename T>
	ComponentHandle<T> EntityManager::addComponent(Entity entity) {
		return addComponent<T>(entity.id_);
	}

	template<typename T>
	ComponentHandle<T> EntityManager::addComponent(Id entityId) {
		if (ASSERT_FAIL(isValid(entityId), "Couldn't add component; Entity is not valid"))
			return ComponentHandle<T>();

		const uint32 componentType = T::type;
		if (ASSERT_FAIL(componentMask_[entityId.index_].test(componentType) == 0, "Couldn't add component; Entity already has this component type"))
			return ComponentHandle<T>();

		const uint32 componentArraysSize = componentArrays_.size();
		if (componentType >= componentArraysSize) {
			for (uint32 i = componentArraysSize; i < componentType + 1; i++)
				componentArrays_.push_back(nullptr);
		}

		if (componentArrays_[componentType] == nullptr) {
			componentArrays_[componentType] = new utils::ByteArray<T>();
			componentArrays_[componentType]->reserve(indexCounter_);
		}

		T* newComponent = new T();
		newComponent->assignedTo_ = entityId;
		componentArrays_[componentType]->reserve(entityId.index_ + 1);
		componentArrays_[componentType]->assign(entityId.index_, static_cast<void*>(newComponent));
		componentMask_[entityId.index_].set(componentType);

		invoke<EventComponentAdded>(componentType, get(entityId));

		ComponentHandle<T> handle(this, entityId);
		return handle;
	}

	template<typename T>
	ComponentHandle<T> EntityManager::getComponent(Entity entity) {
		return getComponent<T>(entity.id_);
	}

	template<typename T>
	ComponentHandle<T> EntityManager::getComponent(Id entityId) {
		if (ASSERT_FAIL(isValid(entityId), "Couldn't getComponent(); Entity is not valid"))
			return ComponentHandle<T>();

		uint32 componentType = T::type;
		if (ASSERT_FAIL(componentArrays_.size() > componentType, "Couldn't getComponent(); This kind of Component does not have corresponsing ByteArray allocated"))
			return ComponentHandle<T>();

		if (ASSERT_FAIL(componentArrays_[componentType] != nullptr, "Couldn't getComponent(); This kind of Component has null ByteArray"))
			return ComponentHandle<T>();

		if (componentMask_[entityId.index_].test(componentType) == 0)
			return ComponentHandle<T>();

		return ComponentHandle<T>(this, entityId);
	}

	template<typename T>
	bool EntityManager::removeComponent(Entity entity) {
		return removeComponent<T>(entity.id_);
	}

	template<typename T>
	bool EntityManager::removeComponent(Id entityId) {
		if (ASSERT_FAIL(isValid(entityId), "Couldn't removeComponent(); Entity is not valid"))
			return false;

		uint32 componentType = T::type;
		if (ASSERT_FAIL(componentArrays_.size() > componentType, "Couldn't removeComponent(); This kind of Component does not have corresponsing ByteArray allocated"))
			return false;

		if (ASSERT_FAIL(componentArrays_[componentType] != nullptr, "Couldn't removeComponent(); This kind of Component has null ByteArray"))
			return false;

		if (ASSERT_FAIL(componentMask_[entityId.index_].test(componentType) != 0, "Couldn't removeComponent(); Entity does not have this component type"))
			return false;

		utils::ByteArray<T>* componentArray = static_cast<utils::ByteArray<T>*>(componentArrays_[componentType]);
		componentArray->destroy(entityId.index_);
		componentMask_[entityId.index_].reset(componentType);

		return true;
	}

	template<typename T>
	bool EntityManager::hasComponent(Entity entity) {
		return hasComponent<T>(entity.id_);
	}

	template<typename T>
	bool EntityManager::hasComponent(Id entityId) {
		if (ASSERT_FAIL(isValid(entityId), "Couldn't check whether Entity hasComponent(); Entity is not valid"))
			return false;

		uint32 componentType = T::type;
		if (ASSERT_FAIL(componentArrays_.size() > componentType, "Couldn't check whether Entity hasComponent(); This kind of Component does not have corresponsing ByteArray allocated"))
			return false;

		if (ASSERT_FAIL(componentArrays_[componentType] != nullptr, "Couldn't check whether Entity hasComponent(); This kind of Component has null ByteArray"))
			return false;

		return (componentMask_[entityId.index_].test(componentType) == 0) ? false : true;
	}

	template<typename T>
	T* EntityManager::getComponentPtr(Id entityId) {
		if (ASSERT_FAIL(isValid(entityId), "Couldn't getComponentPtr(); Entity is not valid"))
			return nullptr;

		const uint32 componentType = T::type;
		if (ASSERT_FAIL(componentArrays_.size() > componentType, "Couldn't getComponentPtr(); This kind of Component does not have corresponsing ByteArray allocated"))
			return nullptr;

		if (ASSERT_FAIL(componentArrays_[componentType] != nullptr, "Couldn't getComponentPtr(); This kind of Component has null ByteArray"))
			return nullptr;

		if (ASSERT_FAIL(componentMask_[entityId.index_].test(componentType) != 0, "Couldn't getComponentPtr(); Entity does not have this component type"))
			return nullptr;

		utils::ByteArray<T>* componentArray = static_cast<utils::ByteArray<T>*>(componentArrays_[componentType]);
		return componentArray->get(entityId.index_);
	}

	template<typename ...T>
	std::vector<Entity> EntityManager::getEntitiesWithComponents() {
		std::bitset<MAX_COMPONENTS> mask;
		setComponentMask(mask, impl::VariadicParameterlessHelper<T...>());
		return getEntitiesWithComponentsNonTemplate(mask);
	}

	template <typename ... T>
	Entity EntityManager::getEntityWithComponents() {
		std::bitset<MAX_COMPONENTS> mask;
		setComponentMask(mask, impl::VariadicParameterlessHelper<T...>());
		return getEntityWithComponentsNonTemplate(mask);
	}

	template<typename T, typename ...DT>
	void EntityManager::setComponentMask(std::bitset<MAX_COMPONENTS>& mask, impl::VariadicParameterlessHelper<T, DT...> args) {
		const uint32 index = T::type;
		mask.set(index);
		setComponentMask(mask, impl::VariadicParameterlessHelper<DT...>());
	}

	template <typename T>
	void EntityManager::setComponentMask(std::bitset<MAX_COMPONENTS>& mask, impl::VariadicParameterlessHelper<T> arg) {
		const uint32 index = T::type;
		mask.set(index);
	}
}
