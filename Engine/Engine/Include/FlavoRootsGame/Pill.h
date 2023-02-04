#pragma once

#include "Global.h"
#include "EECS.h"
#include "Maths/Maths.h"

FLAVO_COMPONENT(ft_game, Pill)
namespace ft_game
{
	class Pill : public eecs::Component<Pill>
	{
	public:
		nlohmann::json serialize() override;
		void deserialize(const nlohmann::json& json) override;

		const float rotationSpeedDeg = 30.0f;
		const std::string meshPath = "../Data/Models/Pill.FBX";
		const std::string diffusePath = "../Data/Textures/Pill_Diffuse.png";
		const std::string normalPath = "../Data/Textures/Pill_Normal.png";
	};
}
