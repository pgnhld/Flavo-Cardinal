#include "FlavoRootsGame/PlayerMovementSystem.h"
#include "Physics/Rigidbody.h"
#include "Physics/CharacterController.h"
#include "Physics/Transform.h"
#include "FlavoRootsGame/Player.h"
#include "FlavoRootsGame/WeaponGun.h"
#include "FlavoRootsGame/WeaponKnife.h"
#include "Rendering/Camera.h"
#include "FTime.h"
#include "Maths/Maths.h"
#include "DxtkString.h"
#include "Rendering/SkinnedMeshRenderer.h"
#include "FAudio.h"
#include "SceneManager.h"
#include "Logger.h"

using namespace ft_engine;
ft_game::AccumulatedPlayerMovementInput& ft_game::AccumulatedPlayerMovementInput::operator+=(const EventPlayerInput& event) {
	this->bDirty = true;
	this->forward += event.forwardPlayer * event.deltaTime;
	this->sides += event.sidesPlayer * event.deltaTime;
	this->vertical += event.verticalScreen * event.deltaTime;
	this->horizontal += event.horizontalScreen * event.deltaTime;
	this->bJump |= event.bJump;
	return *this;
}

void ft_game::AccumulatedPlayerMovementInput::clear() {
	this->bDirty = false;
	this->forward = 0.0f;
	this->sides = 0.0f;
	this->vertical = 0.0f;
	this->horizontal = 0.0f;
	this->bJump = false;
}

ft_game::PlayerMovementSystem::PlayerMovementSystem() {
	subscribe<EventPlayerInput>(this, &PlayerMovementSystem::onPlayerInput);
	subscribe<EventFreezeInput>(this, &PlayerMovementSystem::onFreezeInput);
	subscribe<EventPostSceneLoaded>(this, &PlayerMovementSystem::onPostSceneLoaded);
}

ft_game::PlayerMovementSystem::~PlayerMovementSystem() {
	unsubscribe<EventPlayerInput>();
	unsubscribe<EventFreezeInput>();
	unsubscribe<EventPostSceneLoaded>();
}

void ft_game::PlayerMovementSystem::update(EntityManager& entities, double deltaTime) {
	//get cameras
	std::vector<Entity> cameraEntities = entities.getEntitiesWithComponents<ft_engine::Player, ft_render::Camera, ft_engine::Transform>();
	ft_engine::Transform* cameraTransforms[2] = { nullptr, nullptr };
	for (auto& it : cameraEntities) {
		ft_engine::Player* playerComponent = it.getComponent<ft_engine::Player>().get();
		cameraTransforms[playerComponent->bLocal ? 0 : 1] = it.getComponent<ft_engine::Transform>().get();
	}

	std::vector<Entity> gun_entities = entities.getEntitiesWithComponents<WeaponGun, Transform, Player>();
	auto get_player_gun = [&](bool bLocal) -> Entity* {
		for (Entity& ent : gun_entities)
		{
			if (bLocal == ent.getComponent<Player>()->bLocal)
				return &ent;
		}
		return nullptr;
	};

	std::vector<Entity> players = entities.getEntitiesWithComponents<CharacterController, ft_render::SkinnedMeshRenderer, Player>();
	for (auto& it : players) {
		Player* player = it.getComponent<Player>().get();
		ft_render::SkinnedMeshRenderer* renderer = it.getComponent<ft_render::SkinnedMeshRenderer>().get();
		const uint32 index = player->bLocal ? 0 : 1;

		//Clamp
		float rotationY = cameraTransforms[index]->getLocalRotation().Euler().x;
		if (rotationY > DEG2RAD(180.0f)) {
			rotationY = MathsHelper::Clamp(rotationY, DEG2RAD(310.0f), DEG2RAD(359.0f));
		} else {
			rotationY = MathsHelper::Clamp(rotationY, DEG2RAD(0.0f), DEG2RAD(50.0f));
		}

		if (!renderer->additionalBoneOffsets.empty()) {
			//head bone
			renderer->additionalBoneOffsets[22] = Matrix::CreateFromYawPitchRoll(0.0f, -rotationY, 0.0f);
			// right lowerarm
			renderer->additionalBoneOffsets[4] = Matrix::CreateFromYawPitchRoll(0.0f, 0.0f, rotationY);

			Entity* gun_entity = get_player_gun(player->bLocal);
			gun_entity->getComponent<WeaponGun>()->otherGunMatrix = renderer->thisMeshBoneInfo_[5].boneOffset_.Invert() * renderer->thisMeshBoneInfo_[5].finalTransform_;
		}
	}

	// Set gun positions from our view
	for (Entity& ent : gun_entities)
	{
		const Matrix additionalTransform = Matrix::CreateTranslation(0.0f, 0.0f, 0.0f);
		ent.getComponent<WeaponGun>()->ownGunMatrix = additionalTransform * ent.getComponent<Transform>()->getWorldTransform();
	}
}

void ft_game::PlayerMovementSystem::fixedUpdate(EntityManager& entities, double deltaTime) {
	using ft_render::Camera;
	if (!localInput_.bDirty && !remoteInput_.bDirty)
		return; //nothing to do here

	std::vector<Entity> players = entities.getEntitiesWithComponents<Transform, CharacterController, Player>();
	CharacterController* localController = nullptr;
	CharacterController* remoteController = nullptr;
	bool bWalkedOnGround = false;
	for (auto& i : players) {
		Player* player = i.getComponent<Player>().get();
		CharacterController* controller = i.getComponent<CharacterController>().get();
		ft_render::SkinnedMeshRenderer* renderer = i.getComponent<ft_render::SkinnedMeshRenderer>().get();
		AccumulatedPlayerMovementInput& input = (player->bLocal) ? localInput_ : remoteInput_;
		if (player->bLocal)
			localController = controller;
		else
			remoteController = controller;

		if (!input.bDirty) {
			continue;
		}

		Transform* transform = i.getComponent<Transform>().get();
		if (!bMovementFrozen[player->bLocal ? 0 : 1])
			bWalkedOnGround |= movePlayer(input, transform, controller, renderer);

		rotatePlayer(input, transform, controller);
	}
	framework::FAudio::getInstance().setWalkSound(bWalkedOnGround);

	std::vector<Entity> cameras = entities.getEntitiesWithComponents<Transform, Camera, Player>();
	for (auto& it : cameras) {
		Player* player = it.getComponent<Player>().get();
		AccumulatedPlayerMovementInput& input = (player->bLocal) ? localInput_ : remoteInput_;
		if (!input.bDirty)
			continue;

		Camera* camera = it.getComponent<Camera>().get();
		Transform* transform = it.getComponent<Transform>().get();
		rotateCamera(input, transform, camera, (player->bLocal) ? localController : remoteController);
	}

	localInput_.clear();
	remoteInput_.clear();
	invoke<EventTransformUpdate>();
}

void ft_game::PlayerMovementSystem::onPlayerInput(const EventPlayerInput& event) {
	if (bInputFrozen_[event.bLocalPlayer ? 0 : 1])
		return;

	if (event.bLocalPlayer) {
		localInput_ += event;
	} else {
		remoteInput_ += event;
	}
}

void ft_game::PlayerMovementSystem::onFreezeInput(const EventFreezeInput& event) {
	bInputFrozen_[event.bLocal ? 0 : 1] = event.bToFreeze;
}

void ft_game::PlayerMovementSystem::onFreezeMovement(const EventFreezeMovement& event) {
	bMovementFrozen[event.bLocal ? 0 : 1] = event.bToFreeze;
}

void ft_game::PlayerMovementSystem::onPostSceneLoaded(const EventPostSceneLoaded& event) {
	EntityManager& entities = SceneManager::getInstance().getScene().getEntityManager();
	std::vector<Entity> players = entities.getEntitiesWithComponents<CharacterController, Player, ft_render::SkinnedMeshRenderer>();

	for (auto& it : players) {
		//init animation data
		ft_render::SkinnedMeshRenderer* renderer = it.getComponent<ft_render::SkinnedMeshRenderer>().get();
		renderer->blendingDurationMatrix = {
			{ 0.0f, 0.2f, 0.2f, 0.2f, 0.2f, 0.1f }, //idle
			{ 0.2f, 0.0f, 0.4f, 0.4f, 0.4f, 0.1f }, //forward
			{ 0.2f, 0.4f, 0.0f, 0.4f, 0.4f, 0.1f }, //backward
			{ 0.2f, 0.4f, 0.4f, 0.0f, 0.4f, 0.1f }, //left
			{ 0.2f, 0.4f, 0.4f, 0.4f, 0.0f, 0.1f },	//right
			{ 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.0f }	//jump
		};

		renderer->blendingNeutralPoseMatrix = {
			{ -1, -1, -1, -1, -1, -1 },
			{ -1, -1, 0, 0, 0, -1 },
			{ -1, 0, -1, 0, 0, -1 },
			{ -1, 0, 0, -1, 6, -1 },
			{ -1, 0, 0, 6, -1, -1 },
			{ -1, -1, -1, -1, -1, -1 }
		};

		renderer->crossFadeType = ft_render::SkinnedMeshRenderer::ECrossFadeType::SMOOTH_STOP_2;
	}
}

bool ft_game::PlayerMovementSystem::movePlayer(const AccumulatedPlayerMovementInput& input, Transform* transform,
	CharacterController* controller, ft_render::SkinnedMeshRenderer* renderer) {
	const Vector3 movementNoRotationVector(
		input.sides * controller->sidesSpeed,
		0.0f,
		input.forward * ((input.forward >= 0.0f) ? controller->forwardSpeed : controller->backwardSpeed)
	);

	Vector3 movementVector = Vector3::Transform(movementNoRotationVector, transform->getLocalRotation());

	//Play animations
	if (controller->bOnGround) {
		if (movementNoRotationVector == Vector3::Zero) {
			renderer->changeAnimation(0); //idle
		} else {
			if (movementNoRotationVector.z > 0.0f) {
				renderer->changeAnimation(1, 2.0f); //forward
			} else if (movementNoRotationVector.z < 0.0f) {
				renderer->changeAnimation(2, 1.5f); //backward
			} else { // == 0.0f
				if (movementNoRotationVector.x > 0.0f) {
					renderer->changeAnimation(4, 2.0f); //right
				} else {
					renderer->changeAnimation(3, 2.0f); //left
				}
			}
		}
	}

	if (controller->timeSinceLastJump >= controller->jumpCooldown) {
		const bool bJumpPerform = (input.bJump && controller->timeSinceLastGroundTouch < controller->maximumJumpDelay);
		if (bJumpPerform) {
			controller->currentVerticalSpeed = controller->jumpSpeed;
			renderer->changeAnimation(5);
			controller->timeSinceLastJump = 0.0f;
		} else {
			if (controller->bOnGround) {
				controller->currentVerticalSpeed = -0.1f;
			} else {
				if (controller->bApplyGravity)
					controller->currentVerticalSpeed += Physics::gravity.y * framework::FTime::defaultFixedDeltaTime;
			}
		}
	} else {
		if (controller->bOnGround) {
			controller->currentVerticalSpeed = -0.1f;
		} else {
			if (controller->bApplyGravity)
				controller->currentVerticalSpeed += Physics::gravity.y * framework::FTime::defaultFixedDeltaTime;
		}
	}

	controller->timeSinceLastJump += framework::FTime::fixedDeltaTime;


	movementVector.y = controller->currentVerticalSpeed * framework::FTime::defaultFixedDeltaTime;
	transform->translate(movementVector);

	controller->lastVelocity = movementVector / framework::FTime::defaultFixedDeltaTime;

	return (controller->bOnGround && !(movementVector.x == 0.0f && movementNoRotationVector.z == 0.0f));
}

void ft_game::PlayerMovementSystem::rotatePlayer(const AccumulatedPlayerMovementInput& input, Transform* transform,
	CharacterController* controller) {

	Vector3 deltaRotationVector(0.0f, input.horizontal, 0.0f);
	deltaRotationVector *= controller->horizontalSpeed;
	transform->rotate(Quaternion::CreateFromEuler(deltaRotationVector));
}

void ft_game::PlayerMovementSystem::rotateCamera(const AccumulatedPlayerMovementInput& input, Transform* transform,
	ft_render::Camera* camera, CharacterController* controller) {
	const float prevEulerX = transform->getLocalRotation().Euler().x;

	Vector3 deltaRotationVector(input.vertical, 0.0f, 0.0f);
	deltaRotationVector *= controller->verticalSpeed;

	//theoreticalEulerX = (theoreticalEulerX < DirectX::SimpleMath::PI) ? 
	//max(0.0f, min(theoreticalEulerX, DEG2RAD(89))) : max(DEG2RAD(271.0f), min(theoreticalEulerX, DEG2RAD(359.0f)));

	float theoreticalEulerX = prevEulerX + deltaRotationVector.x;

	// Clamp pitch angle.
	if (theoreticalEulerX < DEG2RAD(180.0f)) {
		theoreticalEulerX = clamp(theoreticalEulerX, DEG2RAD(-90.f), DEG2RAD(89.99f));
	} else {
		theoreticalEulerX = clamp(theoreticalEulerX, DEG2RAD(270.01f), DEG2RAD(359.99f));

		// tofix: probably we shouldn't comapare floats this way...
		if (theoreticalEulerX == DEG2RAD(359.99f)) {
			theoreticalEulerX = DEG2RAD(360.01f);
		}
	}

	deltaRotationVector.x = theoreticalEulerX - prevEulerX;
	transform->rotate(Quaternion::CreateFromEuler(deltaRotationVector));
}
