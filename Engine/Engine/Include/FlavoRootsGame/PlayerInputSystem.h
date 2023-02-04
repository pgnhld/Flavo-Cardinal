#pragma once

#include "Global.h"
#include "EECS.h"

FLAVO_SYSTEM(ft_engine, PlayerInputSystem)
namespace ft_engine
{
	class PlayerInputSystem : public eecs::System<PlayerInputSystem>, public eecs::IInvoker, public eecs::IReceiver<PlayerInputSystem>
	{
	public:
		PlayerInputSystem();
		~PlayerInputSystem();

		void update(eecs::EntityManager& entities, double deltaTime) override;
		void fixedUpdate(eecs::EntityManager& entities, double deltaTime) override;

	private:
		void updateLocal(double deltaTime);
		void updateRemote(double deltaTime);
		void updateMouse(bool bLocalPlayer, double deltaTime);
		void updateGamepad(bool bLocalPlayer, int32 padNumber, double deltaTime);

		bool bPickButtonReleased_[2];
		bool bLeftActionReleased_[2];
		bool bRightActionReleased_[2];
		bool bEscapeButtonReleased_[2];
	};
}
