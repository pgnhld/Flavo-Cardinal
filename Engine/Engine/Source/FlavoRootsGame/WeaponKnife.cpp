#include "FlavoRootsGame/WeaponKnife.h"

nlohmann::json ft_game::WeaponKnife::serialize() {
	return {
		{ "attackRange", attackRange }
	};
}

void ft_game::WeaponKnife::deserialize(const nlohmann::json& json) {
	attackRange = json.at("attackRange");
}
