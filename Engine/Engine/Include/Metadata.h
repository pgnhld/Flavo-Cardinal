#pragma once

#include "Global.h"
#include "EECS.h"

FLAVO_COMPONENT(ft_engine, Metadata)
namespace ft_engine
{
	class Metadata : public eecs::Component<Metadata>
	{
	public:
		std::string name;

		nlohmann::json serialize() override;
		void deserialize(const nlohmann::json& json) override;
	};
}