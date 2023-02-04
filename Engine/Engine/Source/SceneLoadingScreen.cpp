#include "SceneLoadingScreen.h"
#include "FTime.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "ImGuiExtension.h"
#include "FRendererD3D11.h"
#include <chrono>
#include "Logger.h"
#include "FInput.h"
#include "FResourceManager.h"

ft_engine::SceneLoadingScreen::SceneLoadingScreen() {

}

ft_engine::SceneLoadingScreen::~SceneLoadingScreen() {

}

void ft_engine::SceneLoadingScreen::startLoading(uint32 sceneIndex, bool bShowSplash) {
	loadingTimer_ = 0.0f;
	bShowSplash_ = bShowSplash;

	switch(sceneIndex) { //load different backgrounds
	default:
		levelBackground_ = IMAGE("LoadingScreens/Company.png");
		break;
	}
}

bool ft_engine::SceneLoadingScreen::finishLoading() {
	loadingThreadLoop(1.0f, true);

	loadingTimer_ += framework::FTime::deltaTime;
	if (bShowSplash_) {
		//Customization via different splash images based on time since game start
		//if (loadingTimer_ > splashScreenDuration_ * 0.5f) {
		//	levelBackground_ = IMAGE("LoadingScreens/Quotation.png");
		//} else if (loadingTimer_ > splashScreenDuration_ * 0.25f) {
		//	levelBackground_ = IMAGE("LoadingScreens/splash.png");
		//}

		return loadingTimer_ > splashScreenDuration_;
	}

	const Keyboard::State keyboard = framework::FInput::getKeyboard().getState();
	const GamePad::State gamepad = framework::FInput::getGamepad().getState(0);
	return keyboard.E || keyboard.Enter || gamepad.IsAPressed() || keyboard.Space;
}

void ft_engine::SceneLoadingScreen::loadingThreadLoop(float progress, bool bFinished) {
	bool bWindowOpen = true;
	ImGui::SetNextWindowSize(REL(1.0f, 1.0f), ImGuiCond_Always);
	ImGui::SetNextWindowPos(REL(0.0f, 0.0f));
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGui::Begin("Loading window", &bWindowOpen, INVISIBLE());

	//Background
	//ImGui::SetCursorPos(REL(0.0f, 0.0f));
	//ImGui::Image(IMAGE("Backgrounds/Transit.png"), REL(1.0f, 1.0f));

	//Level Background with Text
	ImGui::SetCursorPos(REL(0.0f, 0.0f));
	ImGui::Image(levelBackground_, REL(1.0f, 1.0f));

	//if (!bShowSplash_) {
	//	//Slider
	//	ImGui::SetCursorPos(REL(0.1026f, 0.8537f));
	//	ImGui::Image(IMAGE("LoadingScreens/LoadingBarFill.png"), REL(0.79479f * progress, 0.07222f));

	//	//Background_Slider
	//	ImGui::SetCursorPos(REL(0.0f, 0.0f));
	//	ImGui::Image(IMAGE("LoadingScreens/LoadingBarBck.png"), REL(1.0f, 1.0f));

	//	//Loading Text
	//	ImGui::SetCursorPos(REL(0.0f, 0.0f));
	//	if (bFinished) {
	//		ImGui::Image(IMAGE("LoadingScreens/ContinueText.png"), REL(1.0f, 1.0f));
	//	} else {
	//		ImGui::Image(IMAGE("LoadingScreens/LoadingText.png"), REL(1.0f, 1.0f));
	//	}
	//}

	ImGui::End(); //Loading window
}
