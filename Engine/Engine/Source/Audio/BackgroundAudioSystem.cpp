#include "Audio/BackgroundAudioSystem.h"
#include "FAudio.h"
#include "FlavoRootsGame/SceneSpecificData.h"
#include "SceneManager.h"

ft_game::BackgroundAudioSystem::BackgroundAudioSystem() {
	subscribe<EventPostSceneLoaded>(this, &BackgroundAudioSystem::onPostLevelLoad);
}

ft_game::BackgroundAudioSystem::~BackgroundAudioSystem() {
	unsubscribe<EventPostSceneLoaded>();
}

void ft_game::BackgroundAudioSystem::fixedUpdate(eecs::EntityManager& entities, double deltaTime) {

}

void ft_game::BackgroundAudioSystem::onPostLevelLoad(const EventPostSceneLoaded& event) {
	Entity backgroundMusicEntity = ft_engine::SceneManager::getInstance().getScene().getEntityManager().getEntityWithComponents<ft_game::SceneSpecificData>();
	if (backgroundMusicEntity.isValid()) {
		SceneSpecificData* data = backgroundMusicEntity.getComponent<SceneSpecificData>().get();
		if (data) {
			framework::FAudio::getInstance().playBackgroundMusic(data->backgroundMusic);
		}
	}
}

void ft_game::BackgroundAudioSystem::update(eecs::EntityManager& entities, double deltaTime) {
}