#pragma once

#include "Global.h"
#include "EECS.h"
#include "Maths/Maths.h"

FLAVO_COMPONENT(ft_game, WeaponKnife)
namespace ft_game
{
	class WeaponKnife : public eecs::Component<WeaponKnife>
	{
	public:
		nlohmann::json serialize() override;
		void deserialize(const nlohmann::json& json) override;

		float attackRange = 1.0f;

		Matrix ownGunMatrix;
		Matrix otherGunMatrix;
	};
}