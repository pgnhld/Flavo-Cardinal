#pragma once

#include <Global.h>
#include "Maths/Maths.h"
#include "EECS.h"
#include <vector>
#include "Json.h"

FLAVO_COMPONENT(ft_engine, Transform)
namespace ft_engine
{
	using DirectX::BoundingBox;
	using namespace utils;

	class Transform : public eecs::Component<Transform>
	{
	public:
		Transform();
		~Transform();

		Matrix getLocalTransform() const;
		Vector3 getLocalPosition() const;
		Quaternion getLocalRotation() const;
		Vector3 getLocalScale() const;

		Vector3 getLocalRight() const;
		Vector3 getLocalForward() const;
		Vector3 getLocalUp() const;

		Matrix getWorldTransform() const;
		Vector3 getWorldPosition() const;
		Quaternion getWorldRotation() const;
		Vector3 getWorldScale() const;

		Vector3 getWorldRight() const;
		Vector3 getWorldForward() const;
		Vector3 getWorldUp() const;
		Vector3 getWorldVector(int idx) const;
		Vector3 getWorldVectorNormalized(int idx) const;

		BoundingBox getTransformedBoundingBox() const;

		bool isDirty() const;

		void setLocalTransform(Matrix localTransform);
		void setBoundingBox(const BoundingBox& boundingBox);

		void applyTransformation(const Matrix& transformation);
		void translate(const Vector3& translation);
		void rotate(Quaternion rotation);
		void rotate(float angle, const Vector3& rotationAxis);
		void scale(const Vector3& scale);

		const std::vector<eecs::ComponentHandle<Transform>>& getChildren() const;
		eecs::ComponentHandle<Transform>& getParent();

		nlohmann::json serialize() override;
		void deserialize(const nlohmann::json& json) override;

	private:
		void removeChild(eecs::ComponentHandle<Transform>& child);

		Matrix localTransform_;
		Matrix worldTransform_;

		BoundingBox boundingBox_;

		std::vector<eecs::ComponentHandle<Transform>> children_;
		eecs::ComponentHandle<Transform> parent_;

		bool bDirty_ = false;

		friend class Scene;
		friend class TransformSystem;
		friend class ResolveConstraintsSystem;
	};
}