#include "Rendering/PointLight.h"

ft_render::PointLight::PointLight() : radius(1.0f), attenuation(1.0f), baseLight(std::make_unique<Light>()) {

}

nlohmann::json ft_render::PointLight::serialize() {
	return {
		{ "radius", radius },
		{ "baseLight", *baseLight },
		{ "attenuation", attenuation }
	};
}

void ft_render::PointLight::deserialize(const nlohmann::json& json) {
	radius = json.at("radius");
	*baseLight = json.at("baseLight").get<Light>();
	attenuation = json.at("attenuation");
}
