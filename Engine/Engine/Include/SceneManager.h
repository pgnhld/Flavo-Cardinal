#pragma once

#include "Global.h"
#include <memory>
#include "Scene.h"
#include "Serializer.h"
#include "SceneLoadingScreen.h"

namespace reflection { class Serializer; }

namespace ft_engine
{
	class SceneManager : public eecs::IInvoker, public eecs::IReceiver<SceneManager>
	{
	private:
		SceneManager();

	public:
		~SceneManager();
		class Config;

		static SceneManager& getInstance();
		reflection::Serializer& getSerializer();
		Scene& getScene();
		bool load(uint32 sceneIndex);
		void load(const std::string& scenePath);
		/* To be used in main game loop, outside of systems! */
		bool update();

	private:
		bool isLoadedSceneLineSkippable(REF std::string& currentLine) const;
		void createSerializedSystem(const std::string& systemString, double period) const;
		void createSerializedComponent(const std::string& componentString, const std::string& paramString, uint64 lastEntityId) const;

		std::unique_ptr<Scene> scene_;
		std::unique_ptr<reflection::Serializer> serializer_;
		std::unique_ptr<Config> config_;
		/* Separated from Scene, always present in game */
		std::unique_ptr<ft_render::RenderSystem> renderSystem_;

		std::string nextSceneToLoad_;
		uint32 currentlyLoadedIndex_;
		bool bNeedAcceptOnNextSceneLoad_;

		SceneLoadingScreen loadingScreen_;
	};

	class SceneManager::Config
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
		std::unordered_map<uint32, std::string> sceneIndexToPath;
	};

	void to_json(OUT nlohmann::json& json, const SceneManager::Config& config);
	void from_json(const nlohmann::json& json, OUT SceneManager::Config& config);
}
