#pragma once

#include "Global.h"
#include "EECS.h"
#include "EngineEvent.h"

namespace ft_render 
{
	class Camera;
	class SkinnedMeshRenderer;
}

namespace ft_engine
{
	class Rigidbody;
	class CharacterController;
	class Transform;
}

FLAVO_SYSTEM(ft_game, PlayerMovementSystem)
namespace ft_game
{
	class WeaponGun;
	class WeaponKnife;

	struct AccumulatedPlayerMovementInput
	{
		AccumulatedPlayerMovementInput& operator+=(const EventPlayerInput& event);
		void clear();

		bool bDirty = false;
		bool bJump = false;
		float forward = 0.0f;
		float sides = 0.0f;
		float vertical = 0.0f;
		float horizontal = 0.0f;
	};

	class PlayerMovementSystem : public eecs::System<PlayerMovementSystem>, public eecs::IReceiver<PlayerMovementSystem>, public eecs::IInvoker
	{
	public:
		PlayerMovementSystem();
		~PlayerMovementSystem();

		void update(eecs::EntityManager& entities, double deltaTime) override;
		void fixedUpdate(eecs::EntityManager& entities, double deltaTime) override;

	private:
		void onPlayerInput(const EventPlayerInput& event);
		void onFreezeInput(const EventFreezeInput& event);
		void onFreezeMovement(const EventFreezeMovement& event);
		void onPostSceneLoaded(const EventPostSceneLoaded& event);

		bool movePlayer(const AccumulatedPlayerMovementInput& input, ft_engine::Transform* transform, ft_engine::CharacterController* controller, ft_render::SkinnedMeshRenderer* renderer);
		void rotatePlayer(const AccumulatedPlayerMovementInput& input, ft_engine::Transform* transform, ft_engine::CharacterController* controller);
		void rotateCamera(const AccumulatedPlayerMovementInput& input, ft_engine::Transform* transform, ft_render::Camera* camera, ft_engine::CharacterController* controller);

		AccumulatedPlayerMovementInput localInput_;
		AccumulatedPlayerMovementInput remoteInput_;
		bool bInputFrozen_[2] = { false, false };
		bool bMovementFrozen[2] = { false, false };
	};
}
