#pragma once

#include "Global.h"
#include "EECS.h"
#include "EngineEvent.h"
#include "CoroutineManager.h"

FLAVO_SYSTEM(ft_game, RespawnSystem)
namespace ft_game
{
	class RespawnSystem : public eecs::System<RespawnSystem>, eecs::IReceiver<RespawnSystem>, eecs::IInvoker
	{
	public:
		RespawnSystem();
		~RespawnSystem();

		void update(eecs::EntityManager& entities, double deltaTime) override;

	private:
		struct RespawnState;
		struct FadeScreenData;

		void onLevelRestart(const EventLevelRestart& event);
		void onComponentAdded(const EventComponentAdded& event);
		void onPostSceneLoaded(const EventPostSceneLoaded& event);
		void onEntityRespawn(const EventEntityRespawn& event);

		void respawnEntity(Entity entity, bool bToCheckpoint);

		/* nullptr */
		IEnumerator fadeScreen(CoroutineArg arg);

		const float fadeScreenDuration = 1.5f;
		std::unordered_map<Entity, RespawnState> respawnableStateMap_;
	};

	struct RespawnSystem::RespawnState
	{
		Matrix startTransformMatrix = Matrix();
	};

	struct RespawnSystem::FadeScreenData
	{
		Entity toRespawn;
		bool bRespawnAll;
	};
}