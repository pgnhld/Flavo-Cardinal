#pragma once

#include "Global.h"
#include "EECS.h"

FLAVO_COMPONENT(ft_game, Hologram)
namespace ft_game
{
	class Hologram : public eecs::Component<Hologram>
	{
	public:
		Hologram();

		nlohmann::json serialize() override;
		void deserialize(const nlohmann::json& json) override;

		//Serialized
		std::string animationImagePaths[4];
		float animationSpeed;

		float animationTimer = animationSpeed;
		size_t currentAnimationIndex = 0;
		size_t actualAnimationImageSize = 0;
	};
}