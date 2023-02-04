#include "FInput.h"
#include <fstream>

std::unique_ptr<framework::FInput::Config> framework::FInput::config_ = std::make_unique<Config>();
std::unique_ptr<Mouse> framework::FInput::mouse_ = std::make_unique<Mouse>();
std::unique_ptr<Keyboard> framework::FInput::keyboard_ = std::make_unique<Keyboard>();
std::unique_ptr<GamePad> framework::FInput::gamepad_ = std::make_unique<GamePad>();

framework::FInput::Config& framework::FInput::getConfig() {
	return *config_;
}

Mouse& framework::FInput::getMouse() {
	return *mouse_;
}

Keyboard& framework::FInput::getKeyboard() {
	return *keyboard_;
}

GamePad& framework::FInput::getGamepad() {
	return *gamepad_;
}

framework::FInput::Config::Config() {
	load();
}

void framework::FInput::Config::load() {
	const char* configPath = "../Config/InputSettings.json";
	std::ifstream inFile(configPath);
	nlohmann::json inJson;
	inFile >> inJson;
	from_json(inJson, *this);
}

void framework::FInput::Config::save() const {
	const char* configPath = "../Config/InputSettings.json";
	std::ofstream outFile(configPath);
	nlohmann::json outJson;
	to_json(outJson, *this);
	outFile << std::setw(4) << outJson << std::endl;
}

void framework::to_json(nlohmann::json& json, const FInput::Config& config) {
	json = {
		{ "typeLocal", static_cast<int32>(config.typeLocal) },
		{ "typeRemote", static_cast<int32>(config.typeRemote) }
	};
}

void framework::from_json(const nlohmann::json& json, FInput::Config& config) {
	config.typeLocal = static_cast<FInput::Config::InputType>(json.at("typeLocal").get<int32>());
	config.typeRemote = static_cast<FInput::Config::InputType>(json.at("typeRemote").get<int32>());
}