#pragma once

#include "Global.h"
#include "Maths/Maths.h"
#include "Json.h"

namespace ft_render
{
	struct LightAttenuation
	{
		float constant = 1.0f;
		float linear = 0.5f;
		float quadratic = 0.1f;
	};

	void to_json(nlohmann::json& json, const LightAttenuation& obj);
	void from_json(const nlohmann::json& json, LightAttenuation& obj);

	class Light
	{
	public:
		Light();

		float intensity;
		Color color;
	};

	void to_json(nlohmann::json& json, const Light& obj);
	void from_json(const nlohmann::json& json, Light& obj);
}