#include "Physics/Transform.h"
#include "SceneManager.h"
#include "Assertion.h"
#include "Logger.h"

namespace ft_engine
{
	void Transform::removeChild(eecs::ComponentHandle<Transform>& child) {
		ASSERT_CRITICAL(!children_.empty(), "Couldn't remove child; Parent has no children");
		for (auto& it : children_) {
			if (it == child) {
				std::swap(it, children_.back());
				children_.pop_back();
				return;
			}
		}

		LOG_W("Couldn't remove child");
	}

	Transform::Transform() {
		bDirty_ = true;
		localTransform_ = Matrix::Identity;
		worldTransform_ = Matrix::Identity;
	}

	Transform::~Transform() {
		if (parent_.isValid()) {
			eecs::ComponentHandle<Transform> thisTransform = SceneManager::getInstance().getScene().getEntityManager().getComponent<Transform>(assignedTo_);
			parent_->removeChild(thisTransform);
		}
	}

	void Transform::setLocalTransform(Matrix localTransform) {
		localTransform_ = std::move(localTransform);
		bDirty_ = true;
	}

	void Transform::setBoundingBox(const BoundingBox& boundingBox) {
		boundingBox_ = boundingBox;
	}

	void Transform::translate(const Vector3& translation) {
		localTransform_.Translation(localTransform_.Translation() + translation);
		bDirty_ = true;
	}

	void Transform::rotate(Quaternion rotation) {
		rotation.Normalize();
		localTransform_ = Matrix::CreateFromQuaternion(rotation) * localTransform_;
		bDirty_ = true;
	}

	void Transform::rotate(float angle, const Vector3& rotationAxis) {
		Quaternion quat = Quaternion::CreateFromAxisAngle(rotationAxis, angle);
		quat.Normalize();
		localTransform_ = Matrix::CreateFromQuaternion(quat) * localTransform_;
		bDirty_ = true;
	}

	void Transform::scale(const Vector3& scale) {
		localTransform_ = Matrix::CreateScale(scale) * localTransform_;
		bDirty_ = true;
	}

	const std::vector<eecs::ComponentHandle<Transform>>& Transform::getChildren() const {
		return children_;
	}

	eecs::ComponentHandle<Transform>& Transform::getParent() {
		return parent_;
	}

	nlohmann::json Transform::serialize() {
		return {
			{ "local", localTransform_ },
			{ "boundingBox", boundingBox_ }
		};
	}

	void Transform::deserialize(const nlohmann::json& json) {
		localTransform_ = json.at("local").get<Matrix>();
		boundingBox_ = json.at("boundingBox").get<BoundingBox>();
	}

	void Transform::applyTransformation(const Matrix& transformation) {
		localTransform_ = transformation * localTransform_;
		bDirty_ = true;
	}

	Matrix Transform::getLocalTransform() const {
		return localTransform_;
	}

	Vector3 Transform::getLocalPosition() const {
		return localTransform_.Translation();
	}

	Quaternion Transform::getLocalRotation() const {
		Quaternion quat = Quaternion::CreateFromRotationMatrix(localTransform_);
		quat.Normalize();
		return quat;
	}

	Vector3 Transform::getLocalScale() const {
		return localTransform_.Scale();
	}

	Vector3 Transform::getLocalRight() const {
		return localTransform_.Right();
	}

	Vector3 Transform::getLocalForward() const {
		return localTransform_.Forward();
	}

	Vector3 Transform::getLocalUp() const {
		return localTransform_.Up();
	}

	Matrix Transform::getWorldTransform() const {
		return worldTransform_;
	}

	Vector3 Transform::getWorldPosition() const {
		return worldTransform_.Translation();
	}

	Quaternion Transform::getWorldRotation() const {
		Quaternion quat = Quaternion::CreateFromRotationMatrix(worldTransform_);
		quat.Normalize();
		return quat;
	}

	Vector3 Transform::getWorldScale() const {
		return worldTransform_.Scale();
	}

	Vector3 Transform::getWorldRight() const {
		return worldTransform_.Right();
	}

	Vector3 Transform::getWorldForward() const {
		return worldTransform_.Forward();
	}

	Vector3 Transform::getWorldUp() const {
		return worldTransform_.Up();
	}

	Vector3 Transform::getWorldVector(int idx) const {
		switch (idx) {
		case 0:
			return worldTransform_.Right();
		case 1:
			return worldTransform_.Up();
		case 2:
			return worldTransform_.Forward();
		}
		LOG_W("Incorrect index!\n");
		return Vector3::Zero;
	}

	Vector3 Transform::getWorldVectorNormalized(int idx) const {
		Vector3 temp;
		switch (idx) {
		case 0:
			temp = worldTransform_.Right();
			break;
		case 1:
			temp = worldTransform_.Up();
			break;
		case 2:
			temp = worldTransform_.Forward();
			break;
		}
		temp.Normalize();
		return temp;
	}

	BoundingBox Transform::getTransformedBoundingBox() const {
		BoundingBox transformedBoundingBox;
		boundingBox_.Transform(transformedBoundingBox, getWorldTransform());
		return transformedBoundingBox;
	}

	bool Transform::isDirty() const {
		return bDirty_;
	}
}
