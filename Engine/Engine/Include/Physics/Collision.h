#pragma once

#include <Global.h>
#include "Maths/Maths.h"
#include "Collider.h"

namespace ft_engine
{
	using namespace utils;

	class Contact
	{
	public:
		Vector3 contactPoint;
		Vector3 contactNormal;
		float penetrationDepth;
	};

	class CollisionInfo
	{
	public:
		Entity bodies[2];
		Contact contact;
		Contact secondBestContact;

		Vector3 velocityDeltas[2];
		float penetrationCoefficients[2];

		float friction = 0.3f;
	};

	typedef uint8 intersectFunction_t(const void* lhsCollider, const void* rhsCollider, const Matrix& lhsTransform, const Matrix& rhsTransform, OUT Contact& info, OUT Contact& secondBestInfo);

	class CollisionTest
	{
	public:
		static uint8 boxCollisionBox(const void* lhsBox, const void* rhsBox, const Matrix& lhsTransform, const Matrix& rhsTransform, OUT Contact& info, OUT Contact& secondBestInfo);

		static uint8 checkIntersect(const CollisionPrimitive* lhs, const ft_engine::CollisionPrimitive* rhs, const Matrix& lhsTransform, const Matrix& rhsTransform, OUT Contact& info, OUT Contact& secondBestInfo);

		static const intersectFunction_t* intersectFunctions[1][1];
	};
}