#pragma once

#include "Global.h"
#include "EECS.h"
#include "CoroutineManager.h"
#include "Physics/CharacterController.h"
#include "Water.h"

FLAVO_SYSTEM(ft_game, WaterSystem)
namespace ft_game
{
	class WaterSystem : public eecs::System<WaterSystem>, public eecs::IInvoker {
	public:
		WaterSystem();
		~WaterSystem();

		void update(eecs::EntityManager& entities, double deltaTime) override;
		void fixedUpdate(eecs::EntityManager& entities, double deltaTime) override;

		const float boxHalfSize = 0.5f;
		const float maxPlayerDepth = 0.3f;
		const float maxPlayerTimeInWater = 1.0f;

	private:
		struct DrawWaterEffectData;

		IEnumerator drawWaterEffect(CoroutineArg arg);
	};

	struct WaterSystem::DrawWaterEffectData
	{
		bool bLocalPlayer;
		Entity playerEntity;
		eecs::ComponentHandle<ft_engine::CharacterController> controller;
		Water* water;
	};
}