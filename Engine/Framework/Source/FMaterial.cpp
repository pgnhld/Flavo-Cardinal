#include "FMaterial.h"
#include "FExternalTexture.h"
#include "FResourceManager.h"
#include "DxtkString.h"
#include "Assertion.h"

framework::FMaterial::FMaterial()
	: diffuse(nullptr), normal(nullptr), roughness(nullptr), metallic(nullptr)
	, uvTiling(1.0f, 1.0f), uvOffset(0.0f, 0.0f), colorTint(1.0f, 1.0f, 1.0f, 1.0f), specialEffect(0.0f) {

}

framework::FMaterial::~FMaterial() {

}

void framework::to_json(nlohmann::json& json, const FMaterial& obj) {
	json = {
		{ "#diffuse", (obj.diffuse != nullptr) ? obj.diffuse->path : "-" },
		{ "#normal", (obj.normal != nullptr) ? obj.normal->path : "-" },
		{ "#roughness", (obj.roughness != nullptr) ? obj.roughness->path : "-" },
		{ "#metallic", (obj.metallic != nullptr) ? obj.metallic->path : "-" },
		{ "uvTiling", obj.uvTiling },
		{ "uvOffset", obj.uvOffset },
		{ "colorTint", obj.colorTint },
		{ "specialEffect", obj.specialEffect },
		{ "smoothness", obj.smoothness }
	};
}

void framework::from_json(const nlohmann::json& json, FMaterial& obj) {
	FResourceManager& resourceManager = FResourceManager::getInstance();
	obj.diffuse = resourceManager.getTexture(json.at("#diffuse"));
	obj.normal = resourceManager.getTexture(json.at("#normal"));
	obj.roughness = resourceManager.getTexture(json.at("#roughness"));
	obj.metallic = resourceManager.getTexture(json.at("#metallic"));

	obj.uvTiling = json.at("uvTiling").get<Vector2>();
	obj.uvOffset = json.at("uvOffset").get<Vector2>();
	obj.colorTint = json.at("colorTint").get<Color>();

	obj.smoothness = json.at("smoothness");
	obj.specialEffect = json.at("specialEffect");
}
