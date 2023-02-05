#include "FlavoRootsGame/WeaponGun.h"

const float ft_game::WeaponGun::cooldown = 0.4f;

nlohmann::json ft_game::WeaponGun::serialize() {
	return {
		{ "attackRange", attackRange }
	};
}

void ft_game::WeaponGun::deserialize(const nlohmann::json& json) {
	attackRange = json.at("attackRange");
}
