#include "Rendering/DirectionalLight.h"

ft_render::DirectionalLight::DirectionalLight() : baseLight(std::make_unique<Light>()) {

}

nlohmann::json ft_render::DirectionalLight::serialize() {
	return {
		{ "baseLight", *baseLight }
	};
}

void ft_render::DirectionalLight::deserialize(const nlohmann::json& json) {
	*baseLight = json.at("baseLight").get<Light>();
}
