#pragma once

#include "Global.h"
#include "EECS.h"
#include "imgui.h"
#include "FResourceManager.h"

FLAVO_SYSTEM(ft_game, MainMenuSystem)
namespace ft_game
{
	class MainMenuSystem : public eecs::System<MainMenuSystem>, public eecs::IInvoker, public eecs::IReceiver<MainMenuSystem> {
	public:
		MainMenuSystem();
		~MainMenuSystem();

		void update(eecs::EntityManager& entities, double deltaTime) override;

	private:
		enum class MainMenuState;
		void drawMenuButtons();
		void drawCredits();

		bool bButtonReleased_[2] = { true, true };
		bool bDrawMenuButtons_ = true;
		bool bDrawCredits_ = false;
		MainMenuState currentState_;
		framework::FResourceManager& resources_;
	};

	enum class MainMenuSystem::MainMenuState
	{
		NEW_GAME,
		CREDITS,
		QUIT,
		MAX_STATES
	};
}