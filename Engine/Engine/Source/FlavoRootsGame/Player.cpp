#include "FlavoRootsGame/Player.h"

nlohmann::json ft_engine::Player::serialize() {
	return {
		{ "bLocal", bLocal }
	};
}

void ft_engine::Player::deserialize(const nlohmann::json& json) {
	bLocal = json.at("bLocal");
}
