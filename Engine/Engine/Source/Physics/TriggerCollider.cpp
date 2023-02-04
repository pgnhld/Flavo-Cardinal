#include "Physics/TriggerCollider.h"
#include "DxtkString.h"

nlohmann::json ft_engine::TriggerCollider::serialize() {
	return {
		{ "offset", offset },
		{ "halfBounds", halfBounds }
	};
}

void ft_engine::TriggerCollider::deserialize(const nlohmann::json& json) {
	offset = json.at("offset").get<Matrix>();
	halfBounds = json.at("halfBounds").get<Vector3>();
}
