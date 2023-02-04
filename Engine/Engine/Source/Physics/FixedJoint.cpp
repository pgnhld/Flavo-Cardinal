#include "Physics/FixedJoint.h"

nlohmann::json ft_engine::FixedJoint::serialize() {
	return {
		{ "bEnabled", bEnabled }
	};
}

void ft_engine::FixedJoint::deserialize(const nlohmann::json& json) {
	bEnabled = json.at("bEnabled").get<bool>();
}
