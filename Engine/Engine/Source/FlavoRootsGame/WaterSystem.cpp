#include "FlavoRootsGame/WaterSystem.h"
#include "Physics\Transform.h"
#include "Physics\TriggerCollider.h"
#include "Physics\Rigidbody.h"
#include "FlavoRootsGame/Water.h"
#include "FlavoRootsGame/Player.h"
#include "EngineEvent.h"
#include "Logger.h"
#include "Physics/CharacterController.h"
#include "ImGuiExtension.h"
#include "FResourceManager.h"

namespace ft_game
{
	using ft_engine::Transform;
	using ft_engine::TriggerCollider;
	using ft_engine::Rigidbody;
	using ft_engine::Player;
}

ft_game::WaterSystem::WaterSystem() {
}

ft_game::WaterSystem::~WaterSystem() {
}

void ft_game::WaterSystem::update(eecs::EntityManager & entities, double deltaTime) {
}

void ft_game::WaterSystem::fixedUpdate(eecs::EntityManager & entities, double deltaTime) {
	std::vector<Entity> waters = entities.getEntitiesWithComponents<Transform, TriggerCollider, Water>();
	const int waterSize = waters.size();
	for (int i = 0; i < waterSize; i++) {
		Entity currEntity = waters[i];
		Transform* currTransform = currEntity.getComponent<Transform>().get();
		TriggerCollider* currTrigger = currEntity.getComponent<TriggerCollider>().get();
		Water* currWater = currEntity.getComponent<Water>().get();

		Matrix worldMatrix = currTransform->getWorldTransform();
		EventPhysicsTriggerCollider* eventData = new EventPhysicsTriggerCollider(
			*currTrigger, 
			worldMatrix, 
			static_cast<uint8_t>(
				1 << static_cast<uint8>(ft_engine::ELayer::Pickable) 
				| 1 << static_cast<uint8>(ft_engine::ELayer::Player)
				| 1 << static_cast<uint8>(ft_engine::ELayer::Raft)
			)
		);
		invokeNonConst<EventPhysicsTriggerCollider>(eventData);

		std::vector<Entity> playerNoLongerInWater = currWater->playersInWater;
		for (auto insideWaterEntity : eventData->foundEntities) {
			Transform* rigidbodyTransform = insideWaterEntity.getComponent<Transform>().get();
			const float depth = MathsHelper::Clamp(std::abs(currWater->waterSurfaceWorldPosition.y - (rigidbodyTransform->getWorldPosition().y - boxHalfSize)), 0.0f, 1.0f);

			if (insideWaterEntity.hasComponent<Rigidbody>()) {
				Rigidbody* rigidbody = insideWaterEntity.getComponent<Rigidbody>().get();
				rigidbody->addForce(Vector3::Up * (depth * currWater->maxForce - currWater->damper * rigidbody->velocity.y));
			} else if (insideWaterEntity.hasComponent<ft_engine::CharacterController>()) {
				if (depth < maxPlayerDepth)
					continue;

				eecs::ComponentHandle<ft_engine::CharacterController> controller = insideWaterEntity.getComponent<ft_engine::CharacterController>();
				if (controller->is_respawning)
					continue;

				const auto it = std::find(playerNoLongerInWater.begin(), playerNoLongerInWater.end(), insideWaterEntity);
				if (it != playerNoLongerInWater.end()) playerNoLongerInWater.erase(it);
				const auto fieldIt = std::find(currWater->playersInWater.begin(), currWater->playersInWater.end(), insideWaterEntity);


				Player* player = insideWaterEntity.getComponent<Player>().get();			
				if (fieldIt == currWater->playersInWater.end()) {
					currWater->playersInWater.push_back(insideWaterEntity);

					DrawWaterEffectData* data = new DrawWaterEffectData();
					data->bLocalPlayer = player->bLocal;
					data->controller = controller;
					data->playerEntity = insideWaterEntity;
					data->water = currWater;
					controller->drowningCoroutine = START_COROUTINE(
						&WaterSystem::drawWaterEffect,
						DrawWaterEffectData*,
						data
					);
				}
			}
		}

		for (Entity& e : playerNoLongerInWater) {
			auto it = std::find(currWater->playersInWater.begin(), currWater->playersInWater.end(), e);
			if (it != currWater->playersInWater.end()) {
				Coroutine coro = it->getComponent<ft_engine::CharacterController>()->drowningCoroutine;
				STOP_COROUTINE(coro);
				currWater->playersInWater.erase(it);
			}
		}

		delete eventData; //has to be manually deleted
	}
}

IEnumerator ft_game::WaterSystem::drawWaterEffect(CoroutineArg arg) {
	DrawWaterEffectData* data = static_cast<DrawWaterEffectData*>(arg);

	float timer = 0.0f;
	while (true) {
		bool bEnd = timer > maxPlayerTimeInWater;	
		const float t = timer / maxPlayerTimeInWater;

		bool bWindowOpen = true;
		ImGui::SetNextWindowSize(REL(1.0f, 1.0f), ImGuiCond_Always);
		ImGui::SetNextWindowPos(REL(0.0f, 0.0f));
		ImGui::SetNextWindowBgAlpha(0.0f);
		ImGui::Begin("Water effect window", &bWindowOpen, INVISIBLE());

		ImGui::SetCursorPos(REL((data->bLocalPlayer ? 0.0f : 0.5f), 0.0f));
		ImGui::Image(IMAGE("ScreenEffects/Rain.png"), REL(0.5f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.3f + t * 0.7f));

		ImGui::End();

		if (bEnd)
			break;

		timer += framework::FTime::deltaTime;
		YIELD_RETURN_NULL();
	}

	EventEntityRespawn* playerResetData = new EventEntityRespawn();
	playerResetData->entityToRespawn = data->playerEntity;
	invoke<EventEntityRespawn>(playerResetData);

	const auto fieldIt = std::find(data->water->playersInWater.begin(), data->water->playersInWater.end(), data->playerEntity);
	if (fieldIt != data->water->playersInWater.end()) data->water->playersInWater.erase(fieldIt);
}
