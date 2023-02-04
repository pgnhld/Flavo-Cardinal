#pragma once

#include "Global.h"
#include "EECS.h"

FLAVO_COMPONENT(ft_render, Camera)
namespace ft_render
{
	class Camera : public eecs::Component<Camera>
	{
	public:
		Camera();

		float fovDegrees = 70.0f;
		float aspectRatio = 16.0f / 9.0f;
		float nearPlane = 0.025f;
		float farPlane = 200.0f;

		bool bEnabled = true;

		nlohmann::json serialize() override;
		void deserialize(const nlohmann::json& json) override;
	};
}