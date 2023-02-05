#pragma once

#include "Global.h"
#include "EECS.h"
#include "CoroutineManager.h"

struct EventPostSceneLoaded;

FLAVO_SYSTEM(ft_game, PillSystem)
namespace ft_game
{
	class PillSystem : public eecs::System<PillSystem>, public eecs::IInvoker, public eecs::IReceiver<PillSystem>
	{
	public:
		PillSystem();
		~PillSystem();

		void update(eecs::EntityManager& entities, double deltaTime) override;
		void fixedUpdate(EntityManager& entities, double fixedDeltaTime) override;

	private:
		void onSceneLoaded(const EventPostSceneLoaded& event);

		int pillsCollectedSoFar = 0;
		const int maxPillsToCollect = 5;

		float currentGameTime = 0.0f;
		const float maxGameTime = 120.0f;

		bool bEnded = false;
	};
}
