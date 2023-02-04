#pragma once

#include <Global.h>
#include "EECS.h"
#include <memory>
#include "Maths/Maths.h"

FLAVO_COMPONENT(ft_engine, TriggerCollider)
namespace ft_engine
{
	class TriggerCollider : public eecs::Component<TriggerCollider>
	{
	public:
		nlohmann::json serialize() override;
		void deserialize(const nlohmann::json& json) override;

		Matrix offset = Matrix::Identity;
		Vector3 halfBounds = Vector3::Zero;
	};
}