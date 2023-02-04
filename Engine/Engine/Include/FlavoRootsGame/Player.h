#pragma once

#include "Global.h"
#include "EECS.h"

FLAVO_COMPONENT(ft_engine, Player)
namespace ft_engine
{
	class Player : public eecs::Component<Player>
	{
	public:
		nlohmann::json serialize() override;
		void deserialize(const nlohmann::json& json) override;

		bool bLocal = true;
	};
}