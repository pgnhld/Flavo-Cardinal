#pragma once

#include "Global.h"
#include "EECS.h"
#include "Maths/Maths.h"
#include "CoroutineManager.h"

FLAVO_COMPONENT(ft_engine, CharacterController)
namespace ft_engine
{
	class CharacterController : public eecs::Component<CharacterController>, public eecs::IInvoker
	{
	public:
		CharacterController();

		bool bOnGround = true;
		bool bApplyGravity = true;
		float currentVerticalSpeed = 0.0f;

		float jumpCooldownInAir = 15.0f;
		float jumpSpeed = 1.0f;
		float forwardSpeed = 1.0f;
		float backwardSpeed = 0.5f;
		float sidesSpeed = 1.0f;
		float horizontalSpeed = 1.0f;
		float verticalSpeed = 1.0f;

		Vector3 lastVelocity;
		std::vector<Entity> lastHitGrounds;
		Coroutine drowningCoroutine;
		Coroutine leftPaintCoroutine;
		Coroutine rightPaintCoroutine;

		bool isBulletHitDeathActive = false;

		const float maximumJumpDelay = 0.3f;
		const float jumpCooldown = 0.3f;

		float timeSinceLastGroundTouch = 0.0f;
		float timeSinceLastJump = 0.0f;

		nlohmann::json serialize() override;
		void deserialize(const nlohmann::json& json) override;
	};
}