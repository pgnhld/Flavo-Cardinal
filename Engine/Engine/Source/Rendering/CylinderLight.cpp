#include "Rendering/CylinderLight.h"
#include "DxtkString.h"

ft_render::CylinderLight::CylinderLight() : start(Vector3::Zero), end(Vector3::Zero), radius(1.0f), attenuation(1.0f),
                                            baseLight(std::make_unique<Light>()) {

}

nlohmann::json ft_render::CylinderLight::serialize() {
	return {
		{ "start", start },
		{ "end", end },
		{ "baseLight", *baseLight },
		{ "attenuation", attenuation },
		{ "radius", radius }
	};
}

void ft_render::CylinderLight::deserialize(const nlohmann::json& json) {
	start = json.at("start").get<Vector3>();
	end = json.at("end").get<Vector3>();
	*baseLight = json.at("baseLight").get<Light>();
	attenuation = json.at("attenuation");
	radius = json.at("radius");
}
