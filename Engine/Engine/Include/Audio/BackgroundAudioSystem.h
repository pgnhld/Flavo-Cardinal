#pragma once

#include "Global.h"
#include "EECS.h"
#include "EngineEvent.h"

FLAVO_SYSTEM(ft_game, BackgroundAudioSystem)
namespace ft_game
{
	class BackgroundAudioSystem : public eecs::System<BackgroundAudioSystem>, eecs::IReceiver<BackgroundAudioSystem> {
	public:
		BackgroundAudioSystem();
		~BackgroundAudioSystem();

		void update(eecs::EntityManager& entities, double deltaTime) override;
		void fixedUpdate(eecs::EntityManager& entities, double deltaTime) override;
		void onPostLevelLoad(const EventPostSceneLoaded& event);
	};
}