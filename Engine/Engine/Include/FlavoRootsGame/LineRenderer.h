#pragma once

#include "Global.h"
#include "EECS.h"
#include "Maths/Maths.h"

FLAVO_COMPONENT(ft_game, LineRenderer)
namespace ft_game
{
	class LineRenderer : public eecs::Component<LineRenderer>
	{
	public:
		nlohmann::json serialize() override;
		void deserialize(const nlohmann::json& json) override;

		float timer = 0.0f;
		const float maxTime = 0.1f;

		const Color lineColor = Color(5.0f, 0.0f, 0.0f, 0.8f);
		const float diameter = 0.02f;

		const std::string cylinderMeshPath = "../Data/Models/LineRenderer.FBX";
		const std::string diffusePath = "../Data/Textures/Pure_White_Small.png";
	};
}
