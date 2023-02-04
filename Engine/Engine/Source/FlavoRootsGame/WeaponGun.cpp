#include "FlavoRootsGame/WeaponGun.h"

nlohmann::json ft_game::WeaponGun::serialize() {
	return {
		{ "attackRange", attackRange }
	};
}

void ft_game::WeaponGun::deserialize(const nlohmann::json& json) {
	attackRange = json.at("attackRange");
}
