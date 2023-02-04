#include "Physics/Collider.h"
#include "DxtkString.h"
#include "SceneManager.h"
#include "Assertion.h"
#include "Physics/TriggerCollider.h"
#include "Logger.h"

#define CMP(x, y) \
	(fabsf(x - y) <= FLT_EPSILON * fmaxf(1.0f, fmaxf(fabsf(x), fabsf(y))))

void ft_engine::to_json(nlohmann::json& json, const CollisionPrimitive& obj) {
	LOG_R("to_json CollisionPrimitive base class; It shouldn't be ever invoked!");
}

void ft_engine::from_json(const nlohmann::json& json, CollisionPrimitive& obj) {
	LOG_R("from_json CollisionPrimitive base class; It shouldn't be ever invoked!");
}

uint8_t ft_engine::CollisionBox::getPrimitive() const {
	return static_cast<uint8_t>(ECollisionPrimitive::Box);
}

bool ft_engine::CollisionBox::checkRayIntersection(const Raycast& raycast, const Matrix& boxTransform, float& intersection) const {
	Vector3 minimum = -halfSize;
	Vector3 maximum = halfSize;

	Matrix modelInverse = DirectX::XMMatrixInverse(nullptr, boxTransform);

	Vector3 rayDirection = DirectX::XMVector3Transform(raycast.direction, modelInverse);
	rayDirection -= modelInverse.Translation();
	float scale = rayDirection.Length();
	rayDirection.Normalize();
	Vector3 rayOrigin = DirectX::XMVector3Transform(raycast.origin, modelInverse);

	Vector3 dirfrac(1.0f / rayDirection.x, 1.0f / rayDirection.y, 1.0f / rayDirection.z);

	float t1 = (minimum.x - rayOrigin.x) * dirfrac.x;
	float t2 = (maximum.x - rayOrigin.x) * dirfrac.x;
	float t3 = (minimum.y - rayOrigin.y) * dirfrac.y;
	float t4 = (maximum.y - rayOrigin.y) * dirfrac.y;
	float t5 = (minimum.z - rayOrigin.z) * dirfrac.z;
	float t6 = (maximum.z - rayOrigin.z) * dirfrac.z;

	float tmin = std::max(std::max(std::min(t1, t2), std::min(t3, t4)), std::min(t5, t6));
	float tmax = std::min(std::min(std::max(t1, t2), std::max(t3, t4)), std::max(t5, t6));

	if (tmax < 0) {
		intersection = tmax / scale;
		return false;
	}

	if (tmin > tmax) {
		intersection = tmax / scale;
		return false;
	}

	intersection = tmin / scale;

	if (intersection > raycast.maxLength) {
		return false;
	}

	return true;
}

inline float getComponent(const Vector3& v, uint8_t idx) {
	switch (idx) {
	case 0:
		return v.x;
	case 1:
		return v.y;
	case 2:
		return v.z;
	}
	return 0.0f;
}

inline void setComponent(Vector3& v, float value, uint8_t idx) {
	switch (idx) {
	case 0:
		v.x = value;
		break;
	case 1:
		v.y = value;
		break;
	case 2:
		v.z = value;
		break;
	}
}

inline Vector3 getAxis(const Matrix& m, uint8_t idx) {
	switch (idx) {
	case 0:
		return m.Right();
	case 1:
		return m.Up();
	case 2:
		return m.Forward();
	}
	return Vector3::Zero;
}

inline float transformToAxis(const Vector3& halfSize, const Matrix& transform, const Vector3 &axis) {
	return
		halfSize.x * std::abs(axis.Dot(transform.Right())) +
		halfSize.y * std::abs(axis.Dot(transform.Up())) +
		halfSize.z * std::abs(axis.Dot(transform.Forward()));
}

inline float penetrationOnAxis(const Vector3& lhsHalfSize, const Vector3& rhsHalfSize, const Matrix& lhsTransform, const Matrix& rhsTransform, const Vector3& axis, const Vector3& toCentre) {
	// Project the half-size of one onto axis
	float oneProject = transformToAxis(lhsHalfSize, lhsTransform, axis);
	float twoProject = transformToAxis(rhsHalfSize, rhsTransform, axis);

	// Project this onto the axis
	float distance = std::abs((toCentre.Dot(axis)));

	// Return the overlap (i.e. positive indicates
	// overlap, negative indicates separation).

	return oneProject + twoProject - distance;
}

inline bool tryAxis(const Vector3& lhsHalfSize, const Vector3& rhsHalfSize, const Matrix& lhsTransform, const Matrix& rhsTransform, Vector3 axis, const Vector3& toCentre, unsigned index) {
	// Make sure we have a normalized axis, and don't check almost parallel axes
	if (axis.LengthSquared() < 0.0001) {
		return true;
	}
	axis.Normalize();

	float penetration = penetrationOnAxis(lhsHalfSize, rhsHalfSize, lhsTransform, rhsTransform, axis, toCentre);

	if (penetration < 0) return false;

	return true;
}

bool ft_engine::CollisionBox::checkTriggerColliderIntersection(const TriggerCollider & triggerCollider, const Matrix& colliderTransform, const Matrix& triggerTransform) const {
	Vector3 toCentre = triggerTransform.Translation() - colliderTransform.Translation();

	Vector3 lhs = halfSize;
	Vector3 rhs = triggerCollider.halfBounds;

	if (!tryAxis(lhs, rhs, colliderTransform, triggerTransform, getAxis(colliderTransform, 0), toCentre, 0)) return false;
	if (!tryAxis(lhs, rhs, colliderTransform, triggerTransform, getAxis(colliderTransform, 1), toCentre, 1)) return false;
	if (!tryAxis(lhs, rhs, colliderTransform, triggerTransform, getAxis(colliderTransform, 2), toCentre, 2)) return false;

	if (!tryAxis(lhs, rhs, colliderTransform, triggerTransform, getAxis(triggerTransform, 0), toCentre, 3)) return false;
	if (!tryAxis(lhs, rhs, colliderTransform, triggerTransform, getAxis(triggerTransform, 1), toCentre, 4)) return false;
	if (!tryAxis(lhs, rhs, colliderTransform, triggerTransform, getAxis(triggerTransform, 2), toCentre, 5)) return false;

	if (!tryAxis(lhs, rhs, colliderTransform, triggerTransform, getAxis(colliderTransform, 0).Cross(getAxis(triggerTransform, 0)), toCentre, 6)) return false;
	if (!tryAxis(lhs, rhs, colliderTransform, triggerTransform, getAxis(colliderTransform, 0).Cross(getAxis(triggerTransform, 1)), toCentre, 7)) return false;
	if (!tryAxis(lhs, rhs, colliderTransform, triggerTransform, getAxis(colliderTransform, 0).Cross(getAxis(triggerTransform, 2)), toCentre, 8)) return false;
	if (!tryAxis(lhs, rhs, colliderTransform, triggerTransform, getAxis(colliderTransform, 1).Cross(getAxis(triggerTransform, 0)), toCentre, 9)) return false;
	if (!tryAxis(lhs, rhs, colliderTransform, triggerTransform, getAxis(colliderTransform, 1).Cross(getAxis(triggerTransform, 1)), toCentre, 10)) return false;
	if (!tryAxis(lhs, rhs, colliderTransform, triggerTransform, getAxis(colliderTransform, 1).Cross(getAxis(triggerTransform, 2)), toCentre, 11)) return false;
	if (!tryAxis(lhs, rhs, colliderTransform, triggerTransform, getAxis(colliderTransform, 2).Cross(getAxis(triggerTransform, 0)), toCentre, 12)) return false;
	if (!tryAxis(lhs, rhs, colliderTransform, triggerTransform, getAxis(colliderTransform, 2).Cross(getAxis(triggerTransform, 1)), toCentre, 13)) return false;
	if (!tryAxis(lhs, rhs, colliderTransform, triggerTransform, getAxis(colliderTransform, 2).Cross(getAxis(triggerTransform, 2)), toCentre, 14)) return false;

	return true;
}

Vector3 ft_engine::CollisionBox::getHalfSize() const {
	return halfSize;
}


void ft_engine::to_json(nlohmann::json& json, const CollisionBox& obj) {
	json = nlohmann::json{
		{ "halfSize", obj.halfSize }
	};
}

void ft_engine::from_json(const nlohmann::json& json, CollisionBox& obj) {
	obj.halfSize = json.at("halfSize").get<Vector3>();
}

ft_engine::Collider::Collider(): collisionPrimitive(std::make_unique<CollisionBox>()), layer(ELayer::Default) {

}

nlohmann::json ft_engine::Collider::serialize() {
	nlohmann::json primitiveJson;
	switch (static_cast<ECollisionPrimitive>(collisionPrimitive->getPrimitive())) {
	case ECollisionPrimitive::Box:
		to_json(primitiveJson, *static_cast<CollisionBox*>(collisionPrimitive.get()));
		break;
	default:
		ASSERT_CRITICAL(false, format("Incorrect collision primitive type: ", static_cast<uint8>(collisionPrimitive->getPrimitive())));
		break;
	}

	nlohmann::json mainJson{
		{ "offset", offset },
		{ "primitiveType", collisionPrimitive->getPrimitive() },
		{ "primitive", primitiveJson },
		{ "layer", static_cast<int32>(layer) }
	};

	return mainJson;
}

void ft_engine::Collider::deserialize(const nlohmann::json& json) {
	offset = json.at("offset").get<Matrix>();
	layer = static_cast<ELayer>(json.at("layer"));

	const ECollisionPrimitive primitiveType = static_cast<ECollisionPrimitive>(json.at("primitiveType"));
	switch (primitiveType) {
	case ECollisionPrimitive::Box:
		collisionPrimitive = std::make_unique<CollisionBox>(json.at("primitive").get<CollisionBox>());
		break;
	default:
		ASSERT_CRITICAL(false, format("Incorrect collision primitive type: ", static_cast<uint8>(primitiveType)));
		break;
	}
}
