#pragma once

#include "Global.h"
#include "EECS.h"
#include "CoroutineManager.h"

struct EventPlayerInput;
struct EventAllPillsCollected;
struct EventTimeRanOut;

FLAVO_SYSTEM(ft_game, EndGameSystem)
namespace ft_game
{
	class EndGameSystem : public eecs::System<EndGameSystem>, public eecs::IInvoker, public eecs::IReceiver<EndGameSystem>
	{
	public:
		EndGameSystem();
		~EndGameSystem();

		void update(eecs::EntityManager& entities, double deltaTime) override;
		void fixedUpdate(EntityManager& entities, double fixedDeltaTime) override;

	private:
		struct EndGameScreenCoroutineData;
		enum class EEscapeButtonType;

		void onPlayerInput(const EventPlayerInput& event);
		void onAllPillsCollected(const EventAllPillsCollected& event);
		void OnTimeRanOut(const EventTimeRanOut& event);
		void drawEscapeMenu();
		IEnumerator endGameScreenCoroutine(CoroutineArg arg);

		//Escape menu data
		bool bEscapeMenuVisible_ = false;
		bool bVerticalAxisReleased_[2] = { true, true };
		EEscapeButtonType currentEscapeButton_;

		bool bEnded_ = false;
	};

	struct EndGameSystem::EndGameScreenCoroutineData
	{
		bool oldPapaWon = false;
	};

	enum class EndGameSystem::EEscapeButtonType
	{
		RESUME = 0,
		RESTART = 1,
		QUIT = 2
	};
}