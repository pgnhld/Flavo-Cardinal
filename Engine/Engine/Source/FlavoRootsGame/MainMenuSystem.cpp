#include <utility>
#include "FlavoRootsGame/MainMenuSystem.h"
#include "ImGuiExtension.h"
#include "Maths/Maths.h"
#include "FWindow.h"
#include "FResourceManager.h"
#include "Logger.h"
#include "SceneManager.h"
#include "FInput.h"

ft_game::MainMenuSystem::MainMenuSystem() : currentState_(MainMenuState::NEW_GAME), resources_(framework::FResourceManager::getInstance()) {
	framework::FInput::getMouse().SetMode(Mouse::MODE_ABSOLUTE);
}

ft_game::MainMenuSystem::~MainMenuSystem() {

}

void ft_game::MainMenuSystem::update(eecs::EntityManager& entities, double deltaTime) {
	bool bWindowOpen = true;
	ImGui::SetNextWindowSize(REL(1.0f, 1.0f), ImGuiCond_Always);
	ImGui::SetNextWindowPos(REL(0.0f, 0.0f));
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGui::Begin("Main Menu Background", &bWindowOpen, INVISIBLE());
	ImGui::SetCursorPos(REL(0.0f, 0.0f));
	ImGui::Image(IMAGE("Backgrounds/MainMenu.png"), REL(1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	ImGui::End(); //Main Menu Window

	if (bDrawCredits_) {
		drawCredits();
		return;
	} 

	const Keyboard::State keyboard = framework::FInput::getKeyboard().getState();
	const GamePad::State gamepad = framework::FInput::getGamepad().getState(0);
	if (keyboard.E || keyboard.Enter || gamepad.IsAPressed() || keyboard.Space) {
		switch (currentState_) {
		case MainMenuState::NEW_GAME:
			ft_engine::SceneManager::getInstance().load(1);
			break;

		case MainMenuState::CREDITS:
			bDrawCredits_ = true;
			break;

		case MainMenuState::QUIT:
			framework::FWindow::getInstance().bQuitGame = true;
			break;

		default:
			LOG_R("Abnormal main menu state: ", uint32(currentState_));
		}
	}

	int32 currentStateInteger = static_cast<int32>(currentState_);
	const int32 maxStateInteger = static_cast<int32>(MainMenuState::MAX_STATES);
	if (keyboard.W || keyboard.Up) {
		if (bButtonReleased_[0]) {
			currentStateInteger--;
			bButtonReleased_[0] = false;
		}
	} else if (keyboard.S || keyboard.Down) {
		if (bButtonReleased_[0]) {
			currentStateInteger++;
			bButtonReleased_[0] = false;
		}
	} else {
		bButtonReleased_[0] = true;
	}

	if (gamepad.IsDPadUpPressed() || gamepad.thumbSticks.leftY > 0.2f) {
		if (bButtonReleased_[1]) {
			currentStateInteger--;
			bButtonReleased_[1] = false;
		}
	} else if (gamepad.IsDPadDownPressed() || gamepad.thumbSticks.leftY < -0.2f) {
		if (bButtonReleased_[1]) {
			currentStateInteger++;
			bButtonReleased_[1] = false;
		}
	} else {
		bButtonReleased_[1] = true;
	}

	currentStateInteger = (maxStateInteger + (currentStateInteger % maxStateInteger)) % maxStateInteger;
	currentState_ = static_cast<MainMenuState>(currentStateInteger);

	drawMenuButtons();
}

void ft_game::MainMenuSystem::drawMenuButtons() {
	const ImVec2 buttonSize(REL(0.2f, 0.05f));
	const ImVec2 bitcoinSize(REL(0.036f, 0.05f));

	bool bWindowOpen = true;
	ImGui::SetNextWindowSize(REL(1.0f, 1.0f), ImGuiCond_Always);
	ImGui::SetNextWindowPos(REL(0.0f, 0.0f));
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGui::Begin("Main Buttons", &bWindowOpen, INVISIBLE() & ~ImGuiWindowFlags_NoInputs);

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

	ImVec2 buttonPos = REL(0.4f, 0.4f);
	ImGui::SetCursorPos(buttonPos);
	if (currentState_ == MainMenuState::NEW_GAME) {
		ImGui::Image(IMAGE("Buttons/start_c.png"), buttonSize, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::SetCursorPos(REL(0.35f, 0.4f));
		ImGui::Image(IMAGE("Buttons/bitcoin_mini.png"), bitcoinSize, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	} else {
		ImGui::Image(IMAGE("Buttons/start.png"), buttonSize, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	}

	buttonPos = REL(0.4f, 0.5f);
	ImGui::SetCursorPos(buttonPos);
	if (currentState_ == MainMenuState::CREDITS) {
		ImGui::Image(IMAGE("Buttons/credits_c.png"), buttonSize, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::SetCursorPos(REL(0.35f, 0.5f));
		ImGui::Image(IMAGE("Buttons/bitcoin_mini.png"), bitcoinSize, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	} else {
		ImGui::Image(IMAGE("Buttons/credits.png"), buttonSize, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	}

	buttonPos = REL(0.4f, 0.6f);
	ImGui::SetCursorPos(buttonPos);
	if (currentState_ == MainMenuState::QUIT) {
		ImGui::Image(IMAGE("Buttons/exit_c.png"), buttonSize, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::SetCursorPos(REL(0.35f, 0.6f));
		ImGui::Image(IMAGE("Buttons/bitcoin_mini.png"), bitcoinSize, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	} else {
		ImGui::Image(IMAGE("Buttons/exit.png"), buttonSize, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	}

	ImGui::PopStyleColor(3);

	ImGui::End(); //Main Menu Window
}

void ft_game::MainMenuSystem::drawCredits() {
	bool bWindowOpen = true;
	ImGui::SetNextWindowSize(REL(1.0f, 1.0f), ImGuiCond_Always);
	ImGui::SetNextWindowPos(REL(0.0f, 0.0f));
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGui::Begin("Credits", &bWindowOpen, INVISIBLE() & ~ImGuiWindowFlags_NoInputs);

	ImGui::SetCursorPos(REL(0.25f, 0.25f));
	ImGui::Image(IMAGE("LoadingScreens/credits.png"), REL(0.5f, 0.5f));

	ImGui::End();

	const Keyboard::State keyboard = framework::FInput::getKeyboard().getState();
	const GamePad::State gamepad = framework::FInput::getGamepad().getState(0);
	if (keyboard.Escape || gamepad.IsBackPressed() || gamepad.IsBPressed())
		bDrawCredits_ = false;
}
