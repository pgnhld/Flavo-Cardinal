#include "FlavoRootsGame/RespawnSystem.h"
#include "FlavoRootsGame/Player.h"
#include "Serializer.h"
#include "SceneManager.h"
#include "FResourceManager.h"
#include "ImGuiExtension.h"
#include "Physics/CharacterController.h"

ft_game::RespawnSystem::RespawnSystem() {
	subscribe<EventLevelRestart>(this, &RespawnSystem::onLevelRestart);
	subscribe<EventComponentAdded>(this, &RespawnSystem::onComponentAdded);
	subscribe<EventPostSceneLoaded>(this, &RespawnSystem::onPostSceneLoaded);
	subscribe<EventEntityRespawn>(this, &RespawnSystem::onEntityRespawn);
}

ft_game::RespawnSystem::~RespawnSystem() {
	unsubscribe<EventLevelRestart>();
	unsubscribe<EventComponentAdded>();
	unsubscribe<EventPostSceneLoaded>();
	unsubscribe<EventEntityRespawn>();
}

void ft_game::RespawnSystem::update(eecs::EntityManager& entities, double deltaTime) {

}

void ft_game::RespawnSystem::onLevelRestart(const EventLevelRestart& event) {
	FadeScreenData* data = new FadeScreenData();
	data->bRespawnAll = true;

	START_COROUTINE(
		&RespawnSystem::fadeScreen,
		FadeScreenData*,
		data
	);
}

void ft_game::RespawnSystem::onComponentAdded(const EventComponentAdded& event) {
	if (event.componentType != static_cast<uint32>(reflection::ComponentEnum::CharacterController)
		&& event.componentType != static_cast<uint32>(reflection::ComponentEnum::Rigidbody))
		return;

	Entity entity = event.entity;

	//Add empty matrices because Transform is not valid yet
	respawnableStateMap_.insert({ event.entity, RespawnState() });
}

void ft_game::RespawnSystem::onPostSceneLoaded(const EventPostSceneLoaded& event) {
	for (auto& pair : respawnableStateMap_) {
		Entity entity = pair.first;
		eecs::ComponentHandle<ft_engine::Transform> transformHandle = entity.getComponent<ft_engine::Transform>();
		if (ASSERT_FAIL(transformHandle.isValid(), "Transform is not valid"))
			continue;

		RespawnState state;
		state.startTransformMatrix = transformHandle->getLocalTransform();
		pair.second = state;
	}
}

void ft_game::RespawnSystem::onEntityRespawn(const EventEntityRespawn& event) {
	Entity entityToRespawn = event.entityToRespawn;
	if (entityToRespawn.hasComponent<ft_engine::CharacterController>()) {
		entityToRespawn.getComponent<ft_engine::CharacterController>()->is_respawning = true;

		FadeScreenData* data = new FadeScreenData();
		data->bRespawnAll = false;
		data->toRespawn = entityToRespawn;

		START_COROUTINE(
			&RespawnSystem::fadeScreen,
			FadeScreenData*,
			data
		);
	} else {
		respawnEntity(event.entityToRespawn, true);
	}
}

void ft_game::RespawnSystem::respawnEntity(Entity entity, bool bToCheckpoint) {
	if (!entity.isValid())
		return;

	RespawnState& state = respawnableStateMap_[entity];
	Matrix& transformMatrix = state.startTransformMatrix;

	ft_engine::Transform* transform = entity.getComponent<ft_engine::Transform>().get();
	transform->setLocalTransform(transformMatrix);
}

IEnumerator ft_game::RespawnSystem::fadeScreen(CoroutineArg arg) {
	EntityManager& entities = ft_engine::SceneManager::getInstance().getScene().getEntityManager();

	FadeScreenData* data = static_cast<FadeScreenData*>(arg);
	std::vector<eecs::ComponentHandle<ft_engine::Player>> playerComponents;
	if (data->bRespawnAll) {
		std::vector<Entity> players = entities.getEntitiesWithComponents<ft_engine::CharacterController>();
		std::for_each(players.begin(), players.end(), [&](Entity& e) {
			playerComponents.push_back(e.getComponent<ft_engine::Player>());
		});
	} else {
		playerComponents.push_back(data->toRespawn.getComponent<ft_engine::Player>());
	}

	for (auto& p : playerComponents) {
		invoke<EventFreezeInput>(new EventFreezeInput(true, p->bLocal));
	}

	framework::FResourceManager& resources = framework::FResourceManager::getInstance();
	const ImTextureID blacknessTexture = resources.getTexture("../Data/Images/Pure_Black_Small.png")->getSRV();
	bool bWindowOpen = true;

	const float halfDuration = 0.5f * fadeScreenDuration;
	float timer = 0.0f;
	while (timer < halfDuration) {
		const float t = timer / halfDuration;

		ImGui::SetNextWindowSize(REL(1.0f, 1.0f), ImGuiCond_Always);
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetNextWindowBgAlpha(0.0f);
		ImGui::Begin("Crosshair window", &bWindowOpen, INVISIBLE());

		for (auto& p : playerComponents) {
			ImGui::SetCursorPos(REL((p->bLocal ? 0.0f : 0.5f), 0.0f));
			ImGui::Image(blacknessTexture, REL(0.5f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, t));
		}

		ImGui::End();

		timer += framework::FTime::deltaTime;
		YIELD_RETURN_NULL();
	}

	//Respawn chosen entities
	if (data->bRespawnAll) {
		for (auto& pair : respawnableStateMap_)
			respawnEntity(pair.first, false);
	} else {
		respawnEntity(data->toRespawn, true);
	}

	timer = halfDuration;
	while (timer > 0.0f) {
		const float t = timer / halfDuration;

		ImGui::SetNextWindowSize(REL(1.0f, 1.0f), ImGuiCond_Always);
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetNextWindowBgAlpha(0.0f);
		ImGui::Begin("Crosshair window", &bWindowOpen, INVISIBLE());

		for (auto& p : playerComponents) {
			ImGui::SetCursorPos(REL((p->bLocal ? 0.0f : 0.5f), 0.0f));
			ImGui::Image(blacknessTexture, REL(0.5f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, t));
		}

		ImGui::End();

		timer -= framework::FTime::deltaTime;
		YIELD_RETURN_NULL();
	}

	for (auto& p : playerComponents) {
		invoke<EventFreezeInput>(new EventFreezeInput(false, p->bLocal));
		entities.getComponent<ft_engine::CharacterController>(p->assignedTo_)->is_respawning = false;
	}
}
