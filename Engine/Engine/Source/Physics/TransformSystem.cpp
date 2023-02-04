#include "Physics/TransformSystem.h"
#include "EngineEvent.h"
#include "Assertion.h"
#include "Logger.h"

ft_engine::TransformSystem::TransformSystem() {
	subscribe<EventTransformUpdate>(this, &TransformSystem::updateTransform);
	subscribe<EventCertainTransformUpdate>(this, &TransformSystem::updateCertainTransform);
}

ft_engine::TransformSystem::~TransformSystem() {
	unsubscribe<EventTransformUpdate>();
	unsubscribe<EventCertainTransformUpdate>();
}

void ft_engine::TransformSystem::update(eecs::EntityManager& entities, double deltaTime) {
	//updateAllTransforms();
}

void ft_engine::TransformSystem::fixedUpdate(eecs::EntityManager& entities, double deltaTime) {
	//updateAllTransforms();
}

void ft_engine::TransformSystem::updateTransform(const EventTransformUpdate& eventData) {
	updateAllTransforms();
}

void ft_engine::TransformSystem::updateCertainTransform(const EventCertainTransformUpdate& eventData) {
	if (ASSERT_FAIL(eventData.transformToUpdate, "Invalid TransformToUpdate") || ASSERT_FAIL(eventData.parent, "Invalid TransformToUpdateParent")) {
		return;
	}
	updateTransforms(eventData.transformToUpdate, eventData.parent->worldTransform_, false);
}

void ft_engine::TransformSystem::updateAllTransforms() {
	eecs::ComponentHandle<Transform> rootTransform = SceneManager::getInstance().getScene().getRootTransform();
	if (!ASSERT_FAIL(rootTransform.isValid(), "Root Transform is invalid")) {
		updateTransforms(rootTransform.get(), Matrix(), false);
	}
}

void ft_engine::TransformSystem::updateTransforms(Transform* transform, const Matrix & parentWorldTransform, bool bIsDirty) {
	bIsDirty |= transform->bDirty_;
	if (bIsDirty) {
		transform->worldTransform_ = transform->localTransform_ * parentWorldTransform;
		transform->bDirty_ = false;
	}

	for (int i = transform->children_.size() - 1; i >= 0; i--) {
		if (ASSERT_FAIL(transform->children_[i].isValid(), "Updated Transform is invalid")) {
			transform->children_.erase(transform->children_.begin() + i);
		} else {
			updateTransforms(transform->children_[i].get(), transform->worldTransform_, bIsDirty);
		}
	}
}
