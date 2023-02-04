#pragma once

#include "Global.h"
#include "EECS.h"
#include "Light.h"

FLAVO_COMPONENT(ft_render, DirectionalLight)
namespace ft_render
{
	class DirectionalLight : public eecs::Component<DirectionalLight>
	{
	public:
		DirectionalLight();

		nlohmann::json serialize() override;
		void deserialize(const nlohmann::json& json) override;

		std::unique_ptr<Light> baseLight;
	};
}