#pragma once

#include "Global.h"
#include "EECS.h"
#include "Maths/Maths.h"

FLAVO_COMPONENT(ft_game, WeaponGun)
namespace ft_game
{
	class WeaponGun : public eecs::Component<WeaponGun>
	{
	public:
		nlohmann::json serialize() override;
		void deserialize(const nlohmann::json& json) override;

		static const float cooldown;

		float timeSinceLastGunShot = 1.0f;
		float attackRange = 10.0f;

		Matrix ownGunMatrix;
		Matrix otherGunMatrix;
	};
}