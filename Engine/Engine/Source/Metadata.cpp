#include "Metadata.h"

nlohmann::json ft_engine::Metadata::serialize() {
	return {
		{ "name", name }
	};
}

void ft_engine::Metadata::deserialize(const nlohmann::json& json) {
	name = json.at("name").get<string>();
}
