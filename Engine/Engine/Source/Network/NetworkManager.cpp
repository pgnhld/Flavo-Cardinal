#include "Network/NetworkManager.h"
#include <fstream>

ft_engine::NetworkManager& ft_engine::NetworkManager::getInstance() {
	static NetworkManager instance;
	return instance;
}

ft_engine::NetworkManager::NetworkManager() : config_(std::make_unique<Config>()) {
	config_->load();
}

bool ft_engine::NetworkManager::isLan() const {
	return config_->bLan;
}

bool ft_engine::NetworkManager::isHost() const {
	return config_->bHost;
}

void ft_engine::NetworkManager::Config::load() {
	const char* configPath = "../Config/NetworkSettings.json";
	std::ifstream inFile(configPath);
	nlohmann::json inJson;
	inFile >> inJson;
	from_json(inJson, *this);
}

void ft_engine::NetworkManager::Config::save() const {
	const char* configPath = "../Config/NetworkSettings.json";
	std::ofstream outFile(configPath);
	nlohmann::json outJson;
	to_json(outJson, *this);
	outFile << std::setw(4) << outJson << std::endl;
}

void ft_engine::to_json(nlohmann::json& json, const NetworkManager::Config& config) {
	json = nlohmann::json{
		{ "bLan", config.bLan },
		{ "bHost", config.bHost }
	};
}

void ft_engine::from_json(const nlohmann::json& json, NetworkManager::Config& config) {
	config.bLan = json.at("bLan").get<bool>();
	config.bHost = json.at("bHost").get<bool>();
}
