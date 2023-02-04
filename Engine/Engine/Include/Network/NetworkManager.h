#pragma once

#include "Global.h"
#include "Json.h"

namespace ft_engine
{
	class NetworkManager
	{
	public:
		class Config;
		static NetworkManager& getInstance();

		NetworkManager();

		bool isLan() const;
		bool isHost() const;

	private:
		std::unique_ptr<Config> config_;
	};

	class NetworkManager::Config
	{
	public:
		Config() = default;
		Config(const Config& another) = delete;
		Config(Config&& another) = delete;
		~Config() = default;

		Config& operator=(const Config& another) = delete;
		Config& operator=(Config&& another) = delete;

		void load();
		void save() const;

	public:
		bool bLan;
		bool bHost;
	};

	void to_json(OUT nlohmann::json& json, const NetworkManager::Config& config);
	void from_json(const nlohmann::json& json, OUT NetworkManager::Config& config);
}
