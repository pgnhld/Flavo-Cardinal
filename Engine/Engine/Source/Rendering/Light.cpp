#include "Rendering/Light.h"
#include "DxtkString.h"

void ft_render::to_json(nlohmann::json& json, const LightAttenuation& obj) {
	json = {
		{ "constant", obj.constant },
		{ "linear", obj.linear },
		{ "quadratic", obj.quadratic }
	};
}

void ft_render::from_json(const nlohmann::json& json, LightAttenuation& obj) {
	obj.constant = json.at("constant");
	obj.linear = json.at("linear");
	obj.quadratic = json.at("quadratic");
}

ft_render::Light::Light() : intensity(1.0f), color(1.0f, 1.0f, 1.0f, 1.0f) {

}

void ft_render::to_json(nlohmann::json& json, const Light& obj) {
	json = {
		{ "intensity", obj.intensity },
		{ "color", obj.color }
	};
}

void ft_render::from_json(const nlohmann::json& json, Light& obj) {
	obj.intensity = json.at("intensity");
	obj.color = json.at("color").get<Color>();
}
