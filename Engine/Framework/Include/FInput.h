#pragma once

#include "Global.h"
#include <memory>
#include "Mouse.h"
#include "Keyboard.h"
#include "GamePad.h"
#include "Json.h"

using DirectX::Mouse;
using DirectX::Keyboard;
using DirectX::GamePad;

namespace framework
{
	class FInput
	{
	public:
		class Config;

		static Config& getConfig();
		static Mouse& getMouse();
		static Keyboard& getKeyboard();
		static GamePad& getGamepad();

	private:
		static std::unique_ptr<Config> config_;
		static std::unique_ptr<Mouse> mouse_;
		static std::unique_ptr<Keyboard> keyboard_;
		static std::unique_ptr<GamePad> gamepad_;
	};

	class FInput::Config
	{
	public:
		enum class InputType;

		Config();
		Config(const Config& another) = delete;
		Config(Config&& another) = delete;
		~Config() = default;

		Config& operator=(const Config& another) = delete;
		Config& operator=(Config&& another) = delete;

		void load();
		void save() const;

	public:
		InputType typeLocal;
		InputType typeRemote; //used when splitscreen
	};

	enum class FInput::Config::InputType : int32
	{
		NONE = -1,
		GAMEPAD_0,
		GAMEPAD_1,
		MOUSE
	};

	void to_json(OUT nlohmann::json& json, const FInput::Config& config);
	void from_json(const nlohmann::json& json, OUT FInput::Config& config);
}
