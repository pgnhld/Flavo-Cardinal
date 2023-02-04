#include "Physics/Physics.h"

namespace ft_engine
{
	const Vector3 Physics::defaultGravity = Vector3(0.0f, -20.0f, 0.0f);
	Vector3 Physics::gravity = Physics::defaultGravity;

	const bool Physics::collisionTree[numOfLayers][numOfLayers] = {
		{ true, true, true, true, true, true, false, false },
		{ true, true, true, true, true, true, false, false },
		{ true, true, true, true, true, true, false, false },
		{ true, true, true, true, true, true, false, false },
		{ true, true, true, true, true, true, false, false },
		{ true, true, true, true, true, true, false, false },
		{ false, false, false, false, false, false, false, false },
		{ false, false, false, false, false, false, false, false }
	};
}