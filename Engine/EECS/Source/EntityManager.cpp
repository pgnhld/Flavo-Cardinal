#include "EntityManager.h"
#include "Assertion.h"
#include "Logger.h"

eecs::EntityManager::EntityManager() {

}

bool eecs::EntityManager::isValid(Id entityId) {
	if (entityId.index_ >= entityIdCounters_.size())
		return false;

	const bool bInRange = entityId.index_ <= indexCounter_;
	const bool bUpToDate = entityId.counter_ == entityIdCounters_[entityId.index_];
	return bInRange && bUpToDate;
}

eecs::Entity eecs::EntityManager::create() {
	uint64 newIndex = 0;
	if (!availableIndexes_.empty()) {
		newIndex = availableIndexes_[availableIndexes_.size() - 1];
		availableIndexes_.pop_back();
	} else {
		newIndex = ++indexCounter_;
		componentMask_.emplace_back();
		entityIdCounters_.push_back(0);
	}

	const Id newId(newIndex, entityIdCounters_[newIndex]);
	const Entity newEntity(this, newId);
	return newEntity;
}

eecs::Entity eecs::EntityManager::create(uint64 index) {
	const bool bInAvailable = index < indexCounter_;
	while (indexCounter_ < index - 1) {
		indexCounter_++;
		availableIndexes_.push_back(indexCounter_);
		componentMask_.emplace_back();
		entityIdCounters_.push_back(0);
	}

	if (bInAvailable) {
		const auto it = std::find(availableIndexes_.begin(), availableIndexes_.end(), index);
		ASSERT_CRITICAL(it != availableIndexes_.end(), "Entity index is already in use");
		std::swap(*it, availableIndexes_.back());
		availableIndexes_.pop_back();
		//LOG_W("Inefficient Entity creation; consider sorting scene file by Entity id");
	} else {
		indexCounter_++;
		componentMask_.emplace_back();
		entityIdCounters_.push_back(0);
	}

	const Id newId(index, entityIdCounters_[index]);
	const Entity newEntity(this, newId);
	return newEntity;
}

eecs::Entity eecs::EntityManager::get(Id entityId) {
	return {this, entityId};
}

eecs::Entity eecs::EntityManager::get(uint64 index) {
	if (index < 0 || index >= entityIdCounters_.size())
		return { };

	return {this, Id(index, entityIdCounters_[index])};
}

bool eecs::EntityManager::destroy(Entity entity) {
	return destroy(entity.id_);
}

bool eecs::EntityManager::destroy(Id entityId) {
	if (ASSERT_FAIL(isValid(entityId), "Cannot remove Entity; Entity is not valid")) {
		return false;
	}

	std::bitset<MAX_COMPONENTS> mask = componentMask_[entityId.index_];
	for (uint32 i = 0; i < componentArrays_.size(); i++) {
		utils::ByteArrayBase* componentArray = componentArrays_[i];
		if (ASSERT_FAIL(componentArray != nullptr, format("Component array null: ", i)))
			continue;
		if (mask.test(i)) {
			componentArray->destroy(entityId.index_);
		}
	}

	componentMask_[entityId.index_].reset();
	entityIdCounters_[entityId.index_]++;
	availableIndexes_.push_back(entityId.index_);
	return true;
}

bool eecs::EntityManager::addComponent(uint64 index, ComponentBase* component, uint32 type) {
	const Entity entity = get(index);
	const Id entityId = entity.id_;
	if (ASSERT_FAIL(isValid(entityId), "Couldn't add serialized component; Entity is not valid"))
		return false;

	if (ASSERT_FAIL(componentMask_[entityId.index_].test(type) == 0, "Couldn't add serialized component; Entity already has this component type"))
		return false;

	const uint32 componentArraysSize = componentArrays_.size();
	if (type >= componentArraysSize) {
		for (uint32 i = componentArraysSize; i < type + 1; i++)
			componentArrays_.push_back(nullptr);
	}

	if (ASSERT_FAIL(componentArrays_[type] != nullptr, "Couldn't add serialized component; Component array is nullptr"))
		return false;

	component->assignedTo_ = entity.id_;
	componentArrays_[type]->reserve(entityId.index_ + 1);
	componentArrays_[type]->assign(entityId.index_, static_cast<void*>(component));
	componentMask_[entityId.index_].set(type);

	invoke<EventComponentAdded>(type, entity);

	return true;
}

bool eecs::EntityManager::removeComponent(uint64 index, uint32 type) {
	const Entity entity = get(index);
	const Id entityId = entity.id_;
	if (ASSERT_FAIL(isValid(entityId), "Couldn't remove component; Entity is not valid"))
		return false;

	if (ASSERT_FAIL(componentArrays_.size() > type, "Couldn't removeComponent(); This kind of Component does not have corresponsing ByteArray allocated"))
		return false;

	if (ASSERT_FAIL(componentArrays_[type] != nullptr, "Couldn't removeComponent(); This kind of Component has null ByteArray"))
		return false;

	if (ASSERT_FAIL(componentMask_[entityId.index_].test(type) != 0, "Couldn't removeComponent(); Entity does not have this component type"))
		return false;

	utils::ByteArrayBase* componentArray = componentArrays_[type];
	componentArray->destroy(entityId.index_);
	componentMask_[entityId.index_].reset(type);

	return true;
}

std::vector<uint32> eecs::EntityManager::getEntityActiveComponentsType(Id entityId) {
	std::vector<uint32> retVector;
	const std::bitset<MAX_COMPONENTS>& bitset = componentMask_[entityId.index_];
	for (uint32 i = 0; i < MAX_COMPONENTS; ++i)
		if (bitset.test(i))	retVector.push_back(i);
	return retVector;
}

eecs::ComponentBase* eecs::EntityManager::getComponentBase(Id entityId, uint32 type) {
	if (!isValid(entityId))
		return nullptr;

	if (componentArrays_.size() <= type)
		return nullptr;

	if (componentArrays_[type] == nullptr)
		return nullptr;

	if (componentMask_[entityId.index_].test(type) == 0)
		return nullptr;

	utils::ByteArrayBase* componentArray = static_cast<utils::ByteArrayBase*>(componentArrays_[type]);
	return static_cast<ComponentBase*>(componentArray->getPtr(entityId.index_));
}

eecs::Entity eecs::EntityManager::createRoot() {
	ASSERT_CRITICAL(indexCounter_ == 0, "Root entity has to be created before other Entities");
	componentMask_.emplace_back();
	entityIdCounters_.push_back(0);

	const Id newId(0, entityIdCounters_[0]);
	const Entity newEntity(this, newId);
	return newEntity;
}

std::vector<eecs::Entity> eecs::EntityManager::getEntitiesWithComponentsNonTemplate(std::bitset<MAX_COMPONENTS> mask) {
	std::vector<Entity> entities;
	for (uint32 i = 1; i <= indexCounter_; i++) { //0 omitted because it should be rootEntity
		if ((mask & componentMask_[i]) == mask) {
			entities.emplace_back(this, Id(i, entityIdCounters_[i]));
		}
	}

	return entities;
}

eecs::Entity eecs::EntityManager::getEntityWithComponentsNonTemplate(std::bitset<MAX_COMPONENTS> mask) {
	for (uint32 i = 1; i <= indexCounter_; i++) { //0 omitted because it should be rootEntity
		if ((mask & componentMask_[i]) == mask) {
			return { this, Id(i, entityIdCounters_[i]) };
		}
	}

	//has to be invalid
	return {this, Id(4294967295, 4294967295)};
}

