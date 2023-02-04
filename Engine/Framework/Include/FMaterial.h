#pragma once

#include "Global.h"
#include "Json.h"
#include "Maths/Maths.h"

namespace framework
{
	class FExternalTexture;

	struct FMaterial
	{
		FMaterial();
		~FMaterial();

		//TODO: Some shader
		FExternalTexture* diffuse;
		FExternalTexture* normal;
		FExternalTexture* roughness;
		FExternalTexture* metallic;

		Vector2 uvTiling;
		Vector2 uvOffset;

		// RGB: colorTint
		// A: alpha
		Color colorTint;
		float specialEffect;

		float smoothness = 1.0f;
	};

	void to_json(nlohmann::json& json, const FMaterial& obj);
	void from_json(const nlohmann::json& json, FMaterial& obj);
}
