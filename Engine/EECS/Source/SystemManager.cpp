#include "SystemManager.h"

void eecs::SystemManager::update(EntityManager& entities, const double deltaTime) {
	for (auto& system : systems_) {
		if (bDeleted)
			return;
		system.update(entities, deltaTime);
	}
}

void eecs::SystemManager::fixedUpdate(EntityManager& entities, const double deltaTime) {
	for (auto& system : systems_) {
		system.fixedUpdate(entities, deltaTime);
	}
}

void eecs::SystemManager::add(SystemBase* system, double period) {
	systems_.emplace_back(std::unique_ptr<SystemBase>(system), period);
}
