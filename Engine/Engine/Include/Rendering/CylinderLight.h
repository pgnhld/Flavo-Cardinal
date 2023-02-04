#pragma once

#include "Global.h"
#include "EECS.h"
#include "Light.h"

FLAVO_COMPONENT(ft_render, CylinderLight)
namespace ft_render
{
	class CylinderLight : public eecs::Component<CylinderLight>
	{
	public:
		CylinderLight();

		nlohmann::json serialize() override;
		void deserialize(const nlohmann::json& json) override;

		Vector3 start;
		Vector3 end;
		float radius;
		float attenuation;
		std::unique_ptr<Light> baseLight;
	};
}