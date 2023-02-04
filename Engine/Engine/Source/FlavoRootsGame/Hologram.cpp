#include "FlavoRootsGame/Hologram.h"

ft_game::Hologram::Hologram(): animationSpeed(1.0f) {

}

nlohmann::json ft_game::Hologram::serialize() {
	return {
		{ "animationSpeed", animationSpeed },
		{ "#animationImagePath0", animationImagePaths[0] },
		{ "#animationImagePath1", animationImagePaths[1] },
		{ "#animationImagePath2", animationImagePaths[2] },
		{ "#animationImagePath3", animationImagePaths[3] }
	};
}

void ft_game::Hologram::deserialize( const nlohmann::json& json ) {
	animationSpeed = json.at("animationSpeed");
	animationImagePaths[0] = json.at("#animationImagePath0").get<std::string>();
	animationImagePaths[1] = json.at("#animationImagePath1").get<std::string>();
	animationImagePaths[2] = json.at("#animationImagePath2").get<std::string>();
	animationImagePaths[3] = json.at("#animationImagePath3").get<std::string>();

	for (auto& it : animationImagePaths) {
		if (!it.empty()) {
			++actualAnimationImageSize;
		}
	}
}
