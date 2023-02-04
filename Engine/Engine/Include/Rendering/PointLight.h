#pragma once

#include "Global.h"
#include "EECS.h"
#include "Light.h"

FLAVO_COMPONENT(ft_render, PointLight)
namespace ft_render
{
	class PointLight : public eecs::Component<PointLight>
	{
	public:
		PointLight();

		nlohmann::json serialize() override;
		void deserialize(const nlohmann::json& json) override;

		float radius;
		float attenuation;
		std::unique_ptr<Light> baseLight;
	};
}