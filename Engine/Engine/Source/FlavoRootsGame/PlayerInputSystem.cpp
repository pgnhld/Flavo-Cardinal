#include "FlavoRootsGame/PlayerInputSystem.h"
#include "FInput.h"
#include "EngineEvent.h"
#include "Network/NetworkManager.h"
#include "Physics/Transform.h"
#include "Assertion.h"
#include "Logger.h"
#include <chrono>
#include "FTime.h"

ft_engine::PlayerInputSystem::PlayerInputSystem() {
	framework::FInput::getMouse().SetMode(Mouse::MODE_RELATIVE);
	bPickButtonReleased_[0] = true; bPickButtonReleased_[1] = true;
	bLeftActionReleased_[0] = true; bLeftActionReleased_[1] = true;
	bRightActionReleased_[0] = true; bRightActionReleased_[1] = true;
	bEscapeButtonReleased_[0] = true; bEscapeButtonReleased_[1] = true;
}

ft_engine::PlayerInputSystem::~PlayerInputSystem() {

}

void ft_engine::PlayerInputSystem::update(eecs::EntityManager& entities, double deltaTime) {
	if (!framework::FTime::bSceneStarted)
		return;

	updateLocal(deltaTime);
	updateRemote(deltaTime);
}

void ft_engine::PlayerInputSystem::fixedUpdate(eecs::EntityManager& entities, double deltaTime) {

}

void ft_engine::PlayerInputSystem::updateLocal(double deltaTime) {
	switch(framework::FInput::getConfig().typeLocal) {
	case framework::FInput::Config::InputType::MOUSE:
		updateMouse(true, deltaTime);
		break;
	case framework::FInput::Config::InputType::NONE: 
		//LOG_W("No input specified for Player 1");
		break;
	case framework::FInput::Config::InputType::GAMEPAD_0: 
		updateGamepad(true, 0, deltaTime);
		break;
	case framework::FInput::Config::InputType::GAMEPAD_1: 
		updateGamepad(true, 1, deltaTime);
		break;
	default: ;
	} 
}

void ft_engine::PlayerInputSystem::updateRemote(double deltaTime) {
	if (NetworkManager::getInstance().isLan())
		return;

	switch (framework::FInput::getConfig().typeRemote) {
	case framework::FInput::Config::InputType::MOUSE:
		updateMouse(false, deltaTime);
		break;
	case framework::FInput::Config::InputType::NONE:
		//LOG_W("No input specified for Player 2");
		break;
	case framework::FInput::Config::InputType::GAMEPAD_0:
		updateGamepad(false, 0, deltaTime);
		break;
	case framework::FInput::Config::InputType::GAMEPAD_1:
		updateGamepad(false, 1, deltaTime);
		break;
	default:;
	}
}

void ft_engine::PlayerInputSystem::updateMouse(bool bLocalPlayer, double deltaTime) {
	Mouse& mouse = framework::FInput::getMouse();
	const Keyboard& keyboard = framework::FInput::getKeyboard();
	if (ASSERT_FAIL(mouse.IsConnected(), "Mouse not connected"))
		return;
	if (ASSERT_FAIL(keyboard.IsConnected(), "Keboard not connected"))
		return;

	EventPlayerInput* eventPlayerInput = new EventPlayerInput();
	eventPlayerInput->bLocalPlayer = bLocalPlayer;

	mouse.SetMode(Mouse::MODE_RELATIVE);
	const Mouse::State mouseState = mouse.getState();
	eventPlayerInput->horizontalScreen = mouseState.x;
	eventPlayerInput->verticalScreen = mouseState.y;
	
	const Keyboard::State keyboardState = keyboard.getState();
	eventPlayerInput->bJump = keyboardState.Space;
	eventPlayerInput->forwardPlayer = (keyboardState.W) ? 1.0f : (keyboardState.Up) ? 1.0f : 0.0f;
	eventPlayerInput->forwardPlayer -= (keyboardState.S) ? 1.0f : (keyboardState.Down) ? 1.0f : 0.0f;
	eventPlayerInput->sidesPlayer = (keyboardState.D) ? 1.0f : (keyboardState.Right) ? 1.0f : 0.0f;
	eventPlayerInput->sidesPlayer -= (keyboardState.A) ? 1.0f : (keyboardState.Left) ? 1.0f : 0.0f;
	eventPlayerInput->deltaTime = deltaTime;

	//Handle discrete inputs differently to prevent holding
	const size_t playerNumber = bLocalPlayer ? 0 : 1;
	if (keyboardState.E) {
		if (bPickButtonReleased_[playerNumber]) eventPlayerInput->bPick = true;
		bPickButtonReleased_[playerNumber] = false;
	} else {
		eventPlayerInput->bPick = false;
		bPickButtonReleased_[playerNumber] = true;
	}

	if (mouseState.leftButton) {
		if (bLeftActionReleased_[playerNumber]) eventPlayerInput->bLeftAction = true;
		bLeftActionReleased_[playerNumber] = false;
	} else {
		eventPlayerInput->bLeftAction = false;
		bLeftActionReleased_[playerNumber] = true;
	}

	if (mouseState.rightButton) {
		if (bRightActionReleased_[playerNumber]) eventPlayerInput->bRightAction = true;
		bRightActionReleased_[playerNumber] = false;
	} else {
		eventPlayerInput->bRightAction = false;
		bRightActionReleased_[playerNumber] = true;
	}

	if (keyboardState.Escape) {
		if (bEscapeButtonReleased_[playerNumber]) eventPlayerInput->bEscapeButton = true;
		bEscapeButtonReleased_[playerNumber] = false;
	} else {
		eventPlayerInput->bEscapeButton = false;
		bEscapeButtonReleased_[playerNumber] = true;
	}

	invoke<EventPlayerInput>(eventPlayerInput);
}

void ft_engine::PlayerInputSystem::updateGamepad(bool bLocalPlayer, int32 padNumber, double deltaTime) {
	GamePad& pad = framework::FInput::getGamepad();
	const GamePad::State state = pad.getState(padNumber, GamePad::DEAD_ZONE_INDEPENDENT_AXES);
	if (ASSERT_FAIL(state.IsConnected(), format("Gamepad number ", padNumber, " not connected")))
		return;

	EventPlayerInput* eventPlayerInput = new EventPlayerInput();
	eventPlayerInput->bLocalPlayer = bLocalPlayer;
	eventPlayerInput->horizontalScreen = state.thumbSticks.rightX;
	eventPlayerInput->verticalScreen = -state.thumbSticks.rightY;
	eventPlayerInput->bJump = state.IsAPressed();
	eventPlayerInput->forwardPlayer = state.thumbSticks.leftY + state.IsDPadUpPressed() - state.IsDPadDownPressed();
	eventPlayerInput->sidesPlayer = state.thumbSticks.leftX - state.IsDPadLeftPressed() + state.IsDPadRightPressed();
	eventPlayerInput->deltaTime = deltaTime;

	//Handle discrete inputs differently to prevent holding
	const size_t playerNumber = bLocalPlayer ? 0 : 1;
	if (state.IsBPressed()) {
		if (bPickButtonReleased_[playerNumber]) eventPlayerInput->bPick = true;
		bPickButtonReleased_[playerNumber] = false;
	} else {
		eventPlayerInput->bPick = false;
		bPickButtonReleased_[playerNumber] = true;
	}

	if (state.IsLeftShoulderPressed() || state.IsLeftTriggerPressed()) {
		if (bLeftActionReleased_[playerNumber]) eventPlayerInput->bLeftAction = true;
		bLeftActionReleased_[playerNumber] = false;
	} else {
		eventPlayerInput->bLeftAction = false;
		bLeftActionReleased_[playerNumber] = true;
	}

	if (state.IsRightShoulderPressed() || state.IsRightTriggerPressed()) {
		if (bRightActionReleased_[playerNumber]) eventPlayerInput->bRightAction = true;
		bRightActionReleased_[playerNumber] = false;
	} else {
		eventPlayerInput->bRightAction = false;
		bRightActionReleased_[playerNumber] = true;
	}

	if (state.IsMenuPressed()) {
		if (bEscapeButtonReleased_[playerNumber]) eventPlayerInput->bEscapeButton = true;
		bEscapeButtonReleased_[playerNumber] = false;
	} else {
		eventPlayerInput->bEscapeButton = false;
		bEscapeButtonReleased_[playerNumber] = true;
	}

	invoke<EventPlayerInput>(eventPlayerInput);
}
