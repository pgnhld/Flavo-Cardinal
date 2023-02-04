#pragma once

#include "Global.h"
#include "EECS.h"
#include "FMaterial.h"

FLAVO_COMPONENT(ft_game, Water)
namespace ft_game
{
	class Water : public eecs::Component<Water> {
	public:
		Water();

		float maxForce;
		float damper;
		Vector3 waterSurfaceWorldPosition;

		nlohmann::json serialize() override;
		void deserialize(const nlohmann::json& json) override;

		std::vector<Entity> playersInWater;
	};
}