#include "Physics/CharacterController.h"
#include "SceneManager.h"
#include "Logger.h"

ft_engine::CharacterController::CharacterController() {
	Entity entity = SceneManager::getInstance().getScene().getEntity(this->assignedTo_);
}

nlohmann::json ft_engine::CharacterController::serialize() {
	return {
		{ "jumpCooldownInAir", jumpCooldownInAir },
		{ "jumpSpeed", jumpSpeed },
		{ "forwardSpeed", forwardSpeed },
		{ "sidesSpeed", sidesSpeed },
		{ "horizontalSpeed", horizontalSpeed },
		{ "verticalSpeed", verticalSpeed },
		{ "backwardSpeed", backwardSpeed }
	};
}

void ft_engine::CharacterController::deserialize(const nlohmann::json& json) {
	jumpCooldownInAir = json.at("jumpCooldownInAir");
	jumpSpeed = json.at("jumpSpeed");
	forwardSpeed = json.at("forwardSpeed");
	sidesSpeed = json.at("sidesSpeed");
	horizontalSpeed = json.at("horizontalSpeed");
	verticalSpeed = json.at("verticalSpeed");
	backwardSpeed = json.at("backwardSpeed");
}
