#pragma once

#include <Global.h>
#include "Maths/Maths.h"

namespace ft_engine
{
	enum class ELayer
	{
		Default = 0,
		Player = 1,
		Paintable = 2,
		Pickable = 3,
		Button = 4,
		Raft = 5,
		PlayerAdditional_1 = 6,
		PlayerAdditional_2 = 7
	};
	constexpr int numOfLayers = 8;

	struct Physics
	{
		static const Vector3 defaultGravity;
		static Vector3 gravity;

		static const bool collisionTree[numOfLayers][numOfLayers];
	};
}