#pragma once

#include "Global.h"
#include "imgui.h"

namespace ft_engine
{
	class SceneLoadingScreen
	{
	public:
		SceneLoadingScreen();
		~SceneLoadingScreen();

		void startLoading(uint32 sceneIndex, bool bShowSplash);
		bool finishLoading();

		void loadingThreadLoop(float progress, bool bFinished = false);

	private:
		const float splashScreenDuration_ = 8.0f;

		float loadingTimer_ = 0.0f;
		bool bShowSplash_ = false;
		ImTextureID levelBackground_;
	};
}
