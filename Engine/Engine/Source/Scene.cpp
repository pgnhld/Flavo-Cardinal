#include "Scene.h"
#include "Rendering/RenderSystem.h"
#include "Physics/TransformSystem.h"
#include "FWindow.h"
#include "Assertion.h"
#include "FResourceManager.h"

ft_engine::Scene::Scene() 
	: entityManager_(std::make_unique<eecs::EntityManager>())
	, systemManager_(std::make_unique<eecs::SystemManager>())
	, rootEntity_(entityManager_->createRoot()) {

}

ft_engine::Scene::~Scene() {
	framework::FResourceManager::getInstance().releaseResources();
	entityManager_.reset();
	systemManager_.reset();
}

void ft_engine::Scene::update(double deltaTime) const {
	systemManager_->update(*entityManager_, deltaTime);
}

void ft_engine::Scene::fixedUpdate(double deltaTime) const {
	systemManager_->fixedUpdate(*entityManager_, deltaTime);
}

void ft_engine::Scene::addSystem(eecs::SystemBase* system, double period) const {
	systemManager_->add(system, period);
}

eecs::Entity ft_engine::Scene::instantiate() const {
	return entityManager_->create();
}

eecs::Entity ft_engine::Scene::instantiate(uint64 index) const {
	return entityManager_->create(index);
}

eecs::Entity ft_engine::Scene::instantiateNetwork() const {
	//TODO: implementation
	return instantiate();
}

eecs::Entity ft_engine::Scene::getEntity(eecs::Id id) const {
	return entityManager_->get(id);
}

bool ft_engine::Scene::destroy() {
	//TODO: implementation
	return false;
}

bool ft_engine::Scene::destroyRecursively(eecs::Id entityId) {
	eecs::ComponentHandle<Transform> transform = entityManager_->getComponent<Transform>(entityId);
	if (transform.isValid()) {
		std::vector<eecs::ComponentHandle<Transform>> childrenTransforms = transform->getChildren();
		for (auto& it : childrenTransforms)
			destroyRecursively(it->assignedTo_);
	}

	entityManager_->destroy(entityId);
	return true;
}

bool ft_engine::Scene::destroyNetwork() {
	//TODO: implementation
	return false;
}

bool ft_engine::Scene::assignEntityParent(uint64 child, uint64 parent) {
	const Entity entChild = entityManager_->get(child);
	const Entity entParent = entityManager_->get(parent);
	return assignEntityParent(entChild, entParent);
}

bool ft_engine::Scene::assignEntityParent(eecs::Entity child, eecs::Entity parent) {
	return assignTransformParent(child.getComponent<Transform>(), parent.getComponent<Transform>());
}

bool ft_engine::Scene::assignTransformParent(eecs::ComponentHandle<Transform> child, eecs::ComponentHandle<Transform> parent) {
	if (ASSERT_FAIL(child.isValid(), "Added Transform Child is invalid"))
		return false;
	if (ASSERT_FAIL(parent.isValid(), "Added Transform Parent is invalid"))
		return false;

	Transform* childPtr = child.get();
	Transform* parentPtr = parent.get();
	if (childPtr->parent_.isValid()) {
		childPtr->parent_.get()->removeChild(child);
		//change local pos
		const Matrix changeMatrix = childPtr->getWorldTransform() * parentPtr->getWorldTransform().Invert();
		childPtr->setLocalTransform(changeMatrix);
	}

	parentPtr->children_.push_back(child);
	childPtr->parent_ = parent;
	childPtr->bDirty_ = true;

	return true;
}

eecs::ComponentHandle<ft_engine::Transform> ft_engine::Scene::getRootTransform() {
	return rootEntity_.getComponent<Transform>();
}

eecs::EntityManager& ft_engine::Scene::getEntityManager() const {
	return *entityManager_;
}

std::string& ft_engine::Scene::getScenePath() {
	return scenePath_;
}

uint32& ft_engine::Scene::getSceneIndex() {
	return sceneIndex_;
}
