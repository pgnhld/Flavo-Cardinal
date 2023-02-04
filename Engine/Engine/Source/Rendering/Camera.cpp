#include "Rendering/Camera.h"

ft_render::Camera::Camera() {

}

nlohmann::json ft_render::Camera::serialize() {
	return {
		{ "fovDegrees", fovDegrees },
		{ "aspectRatio", aspectRatio },
		{ "nearPlane", nearPlane },
		{ "farPlane", farPlane }
	};
}

void ft_render::Camera::deserialize(const nlohmann::json& json) {
	fovDegrees = json.at("fovDegrees");
	aspectRatio = json.at("aspectRatio");
	nearPlane = json.at("nearPlane");
	farPlane = json.at("farPlane");
}
