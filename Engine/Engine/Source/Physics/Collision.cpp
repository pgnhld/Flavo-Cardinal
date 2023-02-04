#include "Physics/Collision.h"

namespace ft_engine
{
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

	inline float transformToAxis(const CollisionBox &box, const Matrix& transform, const Vector3 &axis) {
		return
			box.halfSize.x * std::abs(axis.Dot(transform.Right())) +
			box.halfSize.y * std::abs(axis.Dot(transform.Up())) +
			box.halfSize.z * std::abs(axis.Dot(transform.Forward()));
	}

	inline float penetrationOnAxis(const CollisionBox &lhs, const CollisionBox &rhs, const Matrix& lhsTransform, const Matrix& rhsTransform, const Vector3& axis, const Vector3& toCentre) {
		float oneProject = transformToAxis(lhs, lhsTransform, axis);
		float twoProject = transformToAxis(rhs, rhsTransform, axis);
		float distance = std::abs((toCentre.Dot(axis)));

		return oneProject + twoProject - distance;
	}

	inline bool tryAxis(const CollisionBox &lhs, const CollisionBox &rhs, const Matrix& lhsTransform, const Matrix& rhsTransform, Vector3 axis, const Vector3& toCentre, unsigned index, float& smallestPenetration, uint8_t &smallestCase, float& secondSmallestPenetration, uint8_t& secondSmallestCase) {
		if (axis.LengthSquared() < 0.0001) {
			return true;
		}
		axis.Normalize();

		float penetration = penetrationOnAxis(lhs, rhs, lhsTransform, rhsTransform, axis, toCentre);

		if (penetration < 0) return false;
		if (penetration <= smallestPenetration) {
			if (smallestPenetration <= secondSmallestPenetration) {
				secondSmallestPenetration = smallestPenetration;
				secondSmallestCase = smallestCase;
			}

			smallestPenetration = penetration;
			smallestCase = index;
		}
		else if (penetration <= secondSmallestPenetration) {
			secondSmallestPenetration = penetration;
			secondSmallestCase = index;
		}
		return true;
	}

	void fillPointFaceBoxBox(const CollisionBox& lhs, const CollisionBox& rhs, const Matrix& lhsTransform, const Matrix& rhsTransform, const Vector3& toCentre, Contact& info, uint8_t best, float pen) {
		Vector3 normal = getAxis(lhsTransform, best);
		if (getAxis(lhsTransform, best).Dot(toCentre) > 0) {
			normal = normal * -1.0f;
		}

		Vector3 vertex = rhs.halfSize;
		if (getAxis(rhsTransform, 0).Dot(normal) < 0) vertex.x = -vertex.x;
		if (getAxis(rhsTransform, 1).Dot(normal) < 0) vertex.y = -vertex.y;
		if (getAxis(rhsTransform, 2).Dot(normal) < 0) vertex.z = -vertex.z;

		normal.Normalize();
		info.contactNormal = normal;
		info.penetrationDepth = pen;
		info.contactPoint = Vector3::Transform(vertex, rhsTransform);
	}

	inline Vector3 contactPoint(const Vector3 &pOne, const Vector3 &dOne, float oneSize, const Vector3& pTwo, const Vector3& dTwo, float twoSize, bool useOne) {
		Vector3 toSt, cOne, cTwo;
		float dpStaOne, dpStaTwo, dpOneTwo, smOne, smTwo;
		float denom, mua, mub;

		smOne = dOne.LengthSquared();
		smTwo = dTwo.LengthSquared();
		dpOneTwo = dTwo.Dot(dOne);

		toSt = pOne - pTwo;
		dpStaOne = dOne.Dot(toSt);
		dpStaTwo = dTwo.Dot(toSt);

		denom = smOne * smTwo - dpOneTwo * dpOneTwo;

		if (std::abs(denom) < 0.0001f) {
			return useOne ? pOne : pTwo;
		}

		mua = (dpOneTwo * dpStaTwo - smTwo * dpStaOne) / denom;
		mub = (smOne * dpStaTwo - dpOneTwo * dpStaOne) / denom;

		if (mua > oneSize ||
			mua < -oneSize ||
			mub > twoSize ||
			mub < -twoSize) {
			return useOne ? pOne : pTwo;
		} else {
			cOne = pOne + dOne * mua;
			cTwo = pTwo + dTwo * mub;

			return cOne * 0.5 + cTwo * 0.5;
		}
	}
}

uint8 ft_engine::CollisionTest::checkIntersect(const CollisionPrimitive* lhs, const CollisionPrimitive* rhs, const Matrix & lhsTransform, const Matrix & rhsTransform, OUT Contact& info, OUT Contact& secondBestInfo) {

	return intersectFunctions[lhs->getPrimitive()][rhs->getPrimitive()]((const void*)lhs, (const void*)rhs, lhsTransform, rhsTransform, info, secondBestInfo);
}

uint8_t ft_engine::CollisionTest::boxCollisionBox(const void* lhsBox, const void* rhsBox, const Matrix& lhsTransform, const Matrix& rhsTransform, OUT Contact& info, OUT Contact& secondBestInfo) {

	CollisionBox lhs = *static_cast<const CollisionBox*>(lhsBox);
	CollisionBox rhs = *static_cast<const CollisionBox*>(rhsBox);

	Vector3 toCentre = rhsTransform.Translation() - lhsTransform.Translation();

	// We start assuming there is no contact
	float pen = FLT_MAX;
	float secondPen = FLT_MAX;
	uint8_t best = 0xff;
	uint8_t secondBest = 0xff;

	if (!tryAxis(lhs, rhs, lhsTransform, rhsTransform, getAxis(lhsTransform, 0), toCentre, 0, pen, best, secondPen, secondBest)) return 0;
	if (!tryAxis(lhs, rhs, lhsTransform, rhsTransform, getAxis(lhsTransform, 1), toCentre, 1, pen, best, secondPen, secondBest)) return 0;
	if (!tryAxis(lhs, rhs, lhsTransform, rhsTransform, getAxis(lhsTransform, 2), toCentre, 2, pen, best, secondPen, secondBest)) return 0;

	if (!tryAxis(lhs, rhs, lhsTransform, rhsTransform, getAxis(rhsTransform, 0), toCentre, 3, pen, best, secondPen, secondBest)) return 0;
	if (!tryAxis(lhs, rhs, lhsTransform, rhsTransform, getAxis(rhsTransform, 1), toCentre, 4, pen, best, secondPen, secondBest)) return 0;
	if (!tryAxis(lhs, rhs, lhsTransform, rhsTransform, getAxis(rhsTransform, 2), toCentre, 5, pen, best, secondPen, secondBest)) return 0;

	unsigned bestSingleAxis = best;

	if (!tryAxis(lhs, rhs, lhsTransform, rhsTransform, getAxis(lhsTransform, 0).Cross(getAxis(rhsTransform, 0)), toCentre, 6, pen, best, secondPen, secondBest)) return 0;
	if (!tryAxis(lhs, rhs, lhsTransform, rhsTransform, getAxis(lhsTransform, 0).Cross(getAxis(rhsTransform, 1)), toCentre, 7, pen, best, secondPen, secondBest)) return 0;
	if (!tryAxis(lhs, rhs, lhsTransform, rhsTransform, getAxis(lhsTransform, 0).Cross(getAxis(rhsTransform, 2)), toCentre, 8, pen, best, secondPen, secondBest)) return 0;
	if (!tryAxis(lhs, rhs, lhsTransform, rhsTransform, getAxis(lhsTransform, 1).Cross(getAxis(rhsTransform, 0)), toCentre, 9, pen, best, secondPen, secondBest)) return 0;
	if (!tryAxis(lhs, rhs, lhsTransform, rhsTransform, getAxis(lhsTransform, 1).Cross(getAxis(rhsTransform, 1)), toCentre, 10, pen, best, secondPen, secondBest)) return 0;
	if (!tryAxis(lhs, rhs, lhsTransform, rhsTransform, getAxis(lhsTransform, 1).Cross(getAxis(rhsTransform, 2)), toCentre, 11, pen, best, secondPen, secondBest)) return 0;
	if (!tryAxis(lhs, rhs, lhsTransform, rhsTransform, getAxis(lhsTransform, 2).Cross(getAxis(rhsTransform, 0)), toCentre, 12, pen, best, secondPen, secondBest)) return 0;
	if (!tryAxis(lhs, rhs, lhsTransform, rhsTransform, getAxis(lhsTransform, 2).Cross(getAxis(rhsTransform, 1)), toCentre, 13, pen, best, secondPen, secondBest)) return 0;
	if (!tryAxis(lhs, rhs, lhsTransform, rhsTransform, getAxis(lhsTransform, 2).Cross(getAxis(rhsTransform, 2)), toCentre, 14, pen, best, secondPen, secondBest)) return 0;

	if (best > 14) {
		return 0;
	}

	// Calculate second best penetration
	if (secondBest < 3) {
		fillPointFaceBoxBox(lhs, rhs, lhsTransform, rhsTransform, toCentre, secondBestInfo, secondBest, secondPen);
		if (secondBestInfo.contactNormal.Dot(toCentre) > 0.0f) {
			secondBestInfo.contactNormal = secondBestInfo.contactNormal * -1.0f;
		}

	}
	else if (secondBest < 6) {
		fillPointFaceBoxBox(rhs, lhs, rhsTransform, lhsTransform, toCentre*-1.0f, secondBestInfo, secondBest - 3, secondPen);
		if (secondBestInfo.contactNormal.Dot(toCentre) > 0.0f) {
			secondBestInfo.contactNormal = secondBestInfo.contactNormal * -1.0f;
		}
	}
	else {
		secondBest -= 6;
		unsigned lhsAxisIndex = secondBest / 3;
		unsigned rhsAxisIndex = secondBest % 3;

		Vector3 lhsAxis = getAxis(lhsTransform, lhsAxisIndex);
		Vector3 rhsAxis = getAxis(rhsTransform, rhsAxisIndex);
		Vector3 axis = lhsAxis.Cross(rhsAxis);
		axis.Normalize();

		if (axis.Dot(toCentre) > 0.0f) axis = axis * -1.0f;

		secondBestInfo.penetrationDepth = secondPen;
		secondBestInfo.contactNormal = axis;
	}

	// Calculates best penetration
	if (best < 3) {
		fillPointFaceBoxBox(lhs, rhs, lhsTransform, rhsTransform, toCentre, info, best, pen);
		if (info.contactNormal.Dot(toCentre) > 0.0f) { 
			info.contactNormal = info.contactNormal * -1.0f; 
		}
		return 1;
	} else if (best < 6) {
		fillPointFaceBoxBox(rhs, lhs, rhsTransform, lhsTransform, toCentre*-1.0f, info, best - 3, pen);
		if (info.contactNormal.Dot(toCentre) > 0.0f) { 
			info.contactNormal = info.contactNormal * -1.0f; 
		}
		return 1;
	} else {
		best -= 6;
		unsigned lhsAxisIndex = best / 3;
		unsigned rhsAxisIndex = best % 3;

		Vector3 lhsAxis = getAxis(lhsTransform, lhsAxisIndex);
		Vector3 rhsAxis = getAxis(rhsTransform, rhsAxisIndex);
		Vector3 axis = lhsAxis.Cross(rhsAxis);
		axis.Normalize();

		if (axis.Dot(toCentre) > 0.0f) axis = axis * -1.0f;

		Vector3 ptOnlhsEdge = lhs.halfSize;
		Vector3 ptOnrhsEdge = rhs.halfSize;
		for (unsigned i = 0; i < 3; i++) {
			if (i == lhsAxisIndex) setComponent(ptOnlhsEdge, 0.0f, i);
			else if (getAxis(lhsTransform, i).Dot(axis) > 0) setComponent(ptOnlhsEdge, -getComponent(ptOnlhsEdge, i), i);

			if (i == rhsAxisIndex) setComponent(ptOnrhsEdge, 0.0f, i);
			else if (getAxis(rhsTransform, i).Dot(axis) < 0) setComponent(ptOnrhsEdge, -getComponent(ptOnrhsEdge, i), i);
		}

		ptOnlhsEdge = Vector3::Transform(ptOnlhsEdge, lhsTransform);
		ptOnrhsEdge = Vector3::Transform(ptOnrhsEdge, rhsTransform);

		Vector3 vertex = contactPoint(
			ptOnlhsEdge, lhsAxis, getComponent(lhs.halfSize, lhsAxisIndex),
			ptOnrhsEdge, rhsAxis, getComponent(rhs.halfSize, rhsAxisIndex),
			bestSingleAxis > 2
		);

		info.penetrationDepth = pen;
		info.contactNormal = axis;
		info.contactPoint = vertex;

		return 1;
	}
	return 0;
}

ft_engine::intersectFunction_t* ft_engine::CollisionTest::intersectFunctions[1][1] = { { ft_engine::CollisionTest::boxCollisionBox } };