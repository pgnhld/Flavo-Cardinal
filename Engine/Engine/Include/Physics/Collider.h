#pragma once

#include <Global.h>
#include "EECS.h"
#include <memory>
#include "Maths/Maths.h"
#include "Physics.h"

FLAVO_COMPONENT(ft_engine, Collider)
namespace ft_engine
{
	using namespace utils;

	class TriggerCollider;

	struct Raycast {
		Vector3 origin;
		Vector3 direction; // Must be normalized
		float maxLength;
	};

	enum class ECollisionPrimitive
	{
		Box = 0
	};

	class CollisionPrimitive
	{
	public:
		virtual ~CollisionPrimitive() = default;

		virtual uint8_t getPrimitive() const = 0;
		virtual bool checkRayIntersection(const Raycast& raycast, const Matrix& boxTransform, float& intersection) const = 0;
		virtual bool checkTriggerColliderIntersection(const TriggerCollider& triggerCollider, const Matrix& colliderTransform, const Matrix& triggerTransform) const = 0;
	};

	void to_json(nlohmann::json& json, const CollisionPrimitive& obj);
	void from_json(const nlohmann::json& json, CollisionPrimitive& obj);

	class CollisionBox : public CollisionPrimitive
	{
	public:
		Vector3 halfSize;
		uint8_t getPrimitive() const override;
		bool checkRayIntersection(const Raycast& raycast, const Matrix& boxTransform, float& intersection) const override;
		bool checkTriggerColliderIntersection(const TriggerCollider& triggerCollider, const Matrix& colliderTransform, const Matrix& triggerTransform) const override;
	};

	void to_json(nlohmann::json& json, const CollisionBox& obj);
	void from_json(const nlohmann::json& json, CollisionBox& obj);

	class Collider : public eecs::Component<Collider>, public eecs::IInvoker
	{
	public:
		Collider();
		Matrix offset;
		std::unique_ptr<CollisionPrimitive> collisionPrimitive;
		ELayer layer;

		nlohmann::json serialize() override;
		void deserialize(const nlohmann::json& json) override;
	};
}