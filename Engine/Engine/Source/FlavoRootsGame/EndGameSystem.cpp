#include "FlavoRootsGame/EndGameSystem.h"
#include "Physics/Transform.h"
#include "Physics/TriggerCollider.h"
#include "EngineEvent.h"
#include "Logger.h"
#include "imgui.h"
#include "FWindow.h"
#include "FResourceManager.h"
#include "FInput.h"
#include "SceneManager.h"
#include "ImGuiExtension.h"
#include "imgui_internal.h"

ft_game::EndGameSystem::EndGameSystem() : currentEscapeButton_(EEscapeButtonType::RESUME) {
	subscribe<EventPlayerInput>(this, &EndGameSystem::onPlayerInput);
	subscribe<EventAllPillsCollected>(this, &EndGameSystem::onAllPillsCollected);
	subscribe<EventTimeRanOut>(this, &EndGameSystem::OnTimeRanOut);
}

ft_game::EndGameSystem::~EndGameSystem() {
	unsubscribe<EventPlayerInput>();
	unsubscribe<EventAllPillsCollected>();
	unsubscribe<EventTimeRanOut>();
}

void ft_game::EndGameSystem::update(eecs::EntityManager& entities, double deltaTime) {
	if (bEscapeMenuVisible_)
		drawEscapeMenu();
}

void ft_game::EndGameSystem::onAllPillsCollected(const EventAllPillsCollected& event)
{
	EndGameScreenCoroutineData* coroutineData = new EndGameScreenCoroutineData();
	coroutineData->oldPapaWon = false;

	bEnded_ = true;
	START_COROUTINE(
		&EndGameSystem::endGameScreenCoroutine, 
		EndGameScreenCoroutineData*,
		coroutineData
	);
}

void ft_game::EndGameSystem::OnTimeRanOut(const EventTimeRanOut& event)
{
	EndGameScreenCoroutineData* coroutineData = new EndGameScreenCoroutineData();
	coroutineData->oldPapaWon = true;

	bEnded_ = true;
	START_COROUTINE(
		&EndGameSystem::endGameScreenCoroutine, 
		EndGameScreenCoroutineData*,
		coroutineData
	);
}

void ft_game::EndGameSystem::fixedUpdate(EntityManager& entities, double fixedDeltaTime) {

}

void ft_game::EndGameSystem::onPlayerInput(const EventPlayerInput& event) {
	if (event.bEscapeButton) {
		bEscapeMenuVisible_ = !bEscapeMenuVisible_;
		invoke<EventFreezeInput>(new EventFreezeInput(bEscapeMenuVisible_, true));
		invoke<EventFreezeInput>(new EventFreezeInput(bEscapeMenuVisible_, false));
		return;
	}

	if (!bEscapeMenuVisible_)
		return;

	const Keyboard::State keyboard = framework::FInput::getKeyboard().getState();
	const GamePad::State gamepad = framework::FInput::getGamepad().getState(0);

	if (keyboard.E || keyboard.Enter || gamepad.IsAPressed() || keyboard.Space) {
		switch(currentEscapeButton_) {
		case EEscapeButtonType::RESUME: 
			bEscapeMenuVisible_ = false;
			invoke<EventFreezeInput>(new EventFreezeInput(bEscapeMenuVisible_, true));
			invoke<EventFreezeInput>(new EventFreezeInput(bEscapeMenuVisible_, false));
			break;
		case EEscapeButtonType::RESTART: 
			invoke<EventLevelRestart>(new EventLevelRestart());
			bEscapeMenuVisible_ = false;
			break;
		case EEscapeButtonType::QUIT: 
			ft_engine::SceneManager::getInstance().load(0);
			break;
		default: 
			LOG_R("Invalid Escape button type");
		}
	}

	if (event.forwardPlayer) {
		if (bVerticalAxisReleased_[event.bLocalPlayer]) {
			int32 currentButton = static_cast<int32>(currentEscapeButton_);
			currentButton += (event.forwardPlayer > 0.0f) ? -1 : 1;
			if (currentButton < 0) currentButton = 2;
			else if (currentButton > 2) currentButton = 0;
			currentEscapeButton_ = static_cast<EEscapeButtonType>(currentButton);
			bVerticalAxisReleased_[event.bLocalPlayer] = false;
		}
	} else {
		bVerticalAxisReleased_[event.bLocalPlayer] = true;
	}
}

void ft_game::EndGameSystem::drawEscapeMenu() {
	const ImVec2 buttonSize(REL(0.2f, 0.05f));
	const ImVec2 bitcoinSize(REL(0.036f, 0.05f));

	bool bWindowOpen = true;
	ImGui::SetNextWindowSize(REL(1.0f, 1.0f), ImGuiCond_Always);
	ImGui::SetNextWindowPos(REL(0.0f, 0.0f));
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGui::Begin("Escape Menu", &bWindowOpen, INVISIBLE());

	ImGui::SetCursorPos(REL(0.0f, 0.0f));
	ImGui::Image(IMAGE("Pure_Black_Small.png"), REL(1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.8f));

	ImGui::SetCursorPos(REL(0.25f, 0.25f));
	ImGui::Image(IMAGE("Backgrounds/MainMenu.png"), REL(0.5f, 0.5f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

	ImGui::SetCursorPos(REL(0.40f, 0.3f));
	if (currentEscapeButton_ == EEscapeButtonType::RESUME) {
		ImGui::Image(IMAGE("Buttons/continue_c.png"), buttonSize, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::SetCursorPos(REL(0.35f, 0.3f));
		ImGui::Image(IMAGE("Buttons/bitcoin_mini.png"), bitcoinSize, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	} else {
		ImGui::Image(IMAGE("Buttons/continue.png"), buttonSize, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	}

	ImGui::SetCursorPos(REL(0.4f, 0.45f));
	if (currentEscapeButton_ == EEscapeButtonType::RESTART) {
		ImGui::Image(IMAGE("Buttons/restart_c.png"), buttonSize, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::SetCursorPos(REL(0.35f, 0.45f));
		ImGui::Image(IMAGE("Buttons/bitcoin_mini.png"), bitcoinSize, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	} else {
		ImGui::Image(IMAGE("Buttons/restart.png"), buttonSize, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	}

	ImGui::SetCursorPos(REL(0.4f, 0.6f));
	if (currentEscapeButton_ == EEscapeButtonType::QUIT) {
		ImGui::Image(IMAGE("Buttons/menu_c.png"), buttonSize, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::SetCursorPos(REL(0.35f, 0.6f));
		ImGui::Image(IMAGE("Buttons/bitcoin_mini.png"), bitcoinSize, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	} else {
		ImGui::Image(IMAGE("Buttons/menu.png"), buttonSize, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	}

	ImGui::End();
}

IEnumerator ft_game::EndGameSystem::endGameScreenCoroutine(CoroutineArg arg) {
	EndGameScreenCoroutineData* data = static_cast<EndGameScreenCoroutineData*>(arg);

	invoke<EventFreezeInput>(new EventFreezeInput(true, true));
	invoke<EventFreezeInput>(new EventFreezeInput(true, false));

	const double appearTime = 2.0f;
	double timer = 0.0;
	bool bReload = false;

	int32 currentButton = 0;
	while (!bReload) {
		const float alpha = std::min(static_cast<float>(timer / appearTime), 1.0f);
		timer += framework::FTime::deltaTime;

		const ImVec2 buttonSize(REL(0.3f, 0.04f));

		bool bWindowOpen = true;
		ImGui::SetNextWindowSize(REL(1.0f, 1.0f), ImGuiCond_Always);
		ImGui::SetNextWindowPos(REL(0.0f, 0.0f));
		ImGui::SetNextWindowBgAlpha(0.0f);
		ImGui::Begin("Main Menu Background", &bWindowOpen, INVISIBLE());
		ImGui::SetCursorPos(REL(0.0f, 0.0f));
		ImGui::Image(IMAGE("Backgrounds/MainMenu.png"), REL(1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, alpha));

		ImGui::SetCursorPos(REL(0.0f, 0.0f));
		ImGui::Image(IMAGE((data->oldPapaWon ? "LoadingScreens/OldPapaWon.png" : "LoadingScreens/YoungMagickWon.png")), REL(1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, alpha));

		ImGui::SetCursorPos(REL(0.2f, 0.7f));
		if (currentButton == 0) {
			ImGui::Image(IMAGE("Buttons/menu_c.png"), buttonSize, ImVec4(1.0f, 1.0f, 1.0f, alpha));
		} else {
			ImGui::Image(IMAGE("Buttons/menu.png"), buttonSize, ImVec4(1.0f, 1.0f, 1.0f, alpha));
		}

		ImGui::SetCursorPos(REL(0.5f, 0.7f));
		if (currentButton == 1) {
			ImGui::Image(IMAGE("Buttons/restart_c.png"), buttonSize, ImVec4(1.0f, 1.0f, 1.0f, alpha));
		} else {
			ImGui::Image(IMAGE("Buttons/restart.png"), buttonSize, ImVec4(1.0f, 1.0f, 1.0f, alpha));
		}

		ImGui::End();

		const Keyboard::State keyboard = framework::FInput::getKeyboard().getState();
		const GamePad::State gamepad = framework::FInput::getGamepad().getState(0);

		if (keyboard.A || keyboard.Left) {
			if (bVerticalAxisReleased_[0]) {
				currentButton = (((currentButton - 1) % 2) + 2) % 2;
				bVerticalAxisReleased_[0] = false;
			}
		} else if (keyboard.D || keyboard.Right) {
			if (bVerticalAxisReleased_[0]) {
				currentButton = (((currentButton + 1) % 2) + 2) % 2;
				bVerticalAxisReleased_[0] = false;
			}
		} else {
			bVerticalAxisReleased_[0] = true;
		}

		if (gamepad.thumbSticks.leftX < -0.2f || gamepad.IsDPadLeftPressed()) {
			if (bVerticalAxisReleased_[1]) {
				currentButton = (((currentButton - 1) % 2) + 2) % 2;
				bVerticalAxisReleased_[1] = false;
			}
		} else if (gamepad.thumbSticks.leftX > 0.2f || gamepad.IsDPadRightPressed()) {
			if (bVerticalAxisReleased_[1]) {
				currentButton = (((currentButton + 1) % 2) + 2) % 2;
				bVerticalAxisReleased_[1] = false;
			}
		} else {
			bVerticalAxisReleased_[1] = true;
		}

		if (keyboard.E || gamepad.IsAPressed() || keyboard.Space || keyboard.Enter) {
			invoke<EventFreezeInput>(new EventFreezeInput(false, true));
			invoke<EventFreezeInput>(new EventFreezeInput(false, false));

			if (currentButton == 0) {
				ft_engine::SceneManager::getInstance().load(0);
			} else if (currentButton == 1) {
				ft_engine::SceneManager::getInstance().load(1);
			}

			bReload = true;
		}

		YIELD_RETURN_NULL();
	}
}
