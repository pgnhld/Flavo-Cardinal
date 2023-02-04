#pragma once

#include "Global.h"
#include "Events/Event.h"
#include "Physics/Transform.h"
#include "Physics/Collider.h"
#include "Physics/TriggerCollider.h"

struct EventTransformUpdate : eecs::Event<EventTransformUpdate>
{

};

struct EventCertainTransformUpdate : eecs::Event<EventCertainTransformUpdate>
{
	EventCertainTransformUpdate(ft_engine::Transform* value, ft_engine::Transform* valueParent) {
		transformToUpdate = value;
		parent = valueParent;
	}

	ft_engine::Transform* transformToUpdate;
	ft_engine::Transform* parent;
};

struct EventPhysicsRaycast : eecs::Event<EventPhysicsRaycast>
{
	EventPhysicsRaycast(ft_engine::Raycast& raycast, uint8 collisionLayer)
		: raycast(raycast), collisionLayer(collisionLayer) {
	}

	ft_engine::Raycast& raycast;
	uint8 collisionLayer;
	Entity hitEntity;
	float distance = FLT_MAX;
	bool bHit = false;
};

struct EventPhysicsTriggerCollider : eecs::Event<EventPhysicsTriggerCollider>
{
	EventPhysicsTriggerCollider(ft_engine::TriggerCollider& triggerCollider, Matrix& triggerTransform, uint8 triggerLayer)
		: triggerCollider(triggerCollider), triggerWorldTransform(triggerTransform), triggerLayer(triggerLayer) {
	}

	ft_engine::TriggerCollider& triggerCollider;
	Matrix& triggerWorldTransform;
	uint8 triggerLayer;
	std::vector<eecs::Entity> foundEntities;
};

struct EventSystemLoaded : eecs::Event<EventSystemLoaded>
{
	std::string name;
	double period;
};

struct EventPostSceneCreated : eecs::Event<EventPostSceneCreated>
{
	std::string filePath;
};

struct EventPostSceneLoaded : eecs::Event<EventPostSceneLoaded>
{
	std::string filePath;
};

struct EventPlayerInput : eecs::Event<EventPlayerInput>
{
	bool bLocalPlayer = false; //player 1 or player 2
	bool bLeftAction = false;
	bool bRightAction = false;
	bool bJump = false;
	bool bPick = false;
	bool bEscapeButton = false;
	float forwardPlayer = 0.0f;
	float sidesPlayer = 0.0f;
	float horizontalScreen = 0.0f;
	float verticalScreen = 0.0f;
	double deltaTime = 0.0;
};

/* Used when UI element needs input */
struct EventFreezeInput : eecs::Event<EventFreezeInput>
{
	EventFreezeInput(bool bToFreeze, bool bLocal) : bToFreeze(bToFreeze), bLocal(bLocal) {}
	bool bToFreeze;
	bool bLocal;
};

struct EventFreezeMovement : eecs::Event<EventFreezeMovement>
{
	EventFreezeMovement(bool bToFreeze, bool bLocal) : bToFreeze(bToFreeze), bLocal(bLocal) {}
	bool bToFreeze = false;
	bool bLocal = false;
};

struct EventEntityRespawn : eecs::Event<EventEntityRespawn>
{
	Entity entityToRespawn;
};
