#include "FlavoRootsGame/Water.h"
#include "DxtkString.h";

ft_game::Water::Water() {


}

nlohmann::json ft_game::Water::serialize() {
	return {
		{ "maxForce", maxForce },
		{ "damper", damper},
		{ "waterSurfaceWorldPosition", waterSurfaceWorldPosition }
	};
}

void ft_game::Water::deserialize(const nlohmann::json & json) {
	maxForce = json.at("maxForce");
	damper = json.at("damper");
	waterSurfaceWorldPosition = json.at("waterSurfaceWorldPosition").get<Vector3>();
}