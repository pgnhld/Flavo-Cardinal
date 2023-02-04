#pragma once

#include "Global.h"

namespace framework
{
	struct FTime
	{
	public:
		static float deltaTime;

		static const float defaultFixedDeltaTime;
		static float fixedDeltaTime;

		static const float defaultTimeScale;
		static float timeScale;

		static float timeSinceStart;

		static bool bSceneStarted;
	};
}