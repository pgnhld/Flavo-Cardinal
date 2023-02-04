#include "FlavoRootsGame/PlayerShootingSystem.h"
#include "SceneManager.h"
#include "Physics/Transform.h"
#include "FlavoRootsGame/Player.h"
#include "Rendering/StaticMeshRenderer.h"
#include "Rendering/Camera.h"
#include "Logger.h"
#include "imgui.h"
#include "FWindow.h"
#include "FResourceManager.h"
#include <random>
#include "CoroutineManager.h"
#include "DxtkString.h""
#include "ImGuiExtension.h"
#include "FAudio.h"
#include "FlavoRootsGame/LineRenderer.h"

ft_game::PlayerShootingSystem::PlayerShootingSystem()
{
	subscribe<EventPlayerInput>(this, &PlayerShootingSystem::onPlayerInput);
	subscribe<EventFreezeInput>(this, &PlayerShootingSystem::onFreezeInput);
	subscribe<EventPostSceneLoaded>(this, &PlayerShootingSystem::onPostSceneLoaded);
}

ft_game::PlayerShootingSystem::~PlayerShootingSystem() {
	unsubscribe<EventPlayerInput>();
	unsubscribe<EventFreezeInput>();
	unsubscribe<EventPostSceneLoaded>();
}

void ft_game::PlayerShootingSystem::update(eecs::EntityManager& entities, double deltaTime) {
	activeLineRenderers_.erase(std::remove_if(
			activeLineRenderers_.begin(), activeLineRenderers_.end(), [](LineRendererTransformData x) {return !x.entity.isValid(); }
		), activeLineRenderers_.end());

	drawCrosshair();
	drawScreenSeparator();
}

void ft_game::PlayerShootingSystem::drawCrosshair() {
	//init images
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowBorderSize = 0.0f;

	//gun count
	uint8 localGuns = 0, remoteGuns = 0;
	EntityManager& entities = ft_engine::SceneManager::getInstance().getScene().getEntityManager();
	//std::vector<Entity> guns = entities.getEntitiesWithComponents<ft_engine::Player, Gun>();
	//for (auto& it : guns) {
	//	ft_engine::Player* player = it.getComponent<ft_engine::Player>().get();
	//	if (player->bLocal) {
	//		++localGuns;
	//	} else {
	//		++remoteGuns;
	//	}
	//}

	framework::FResourceManager& resources = framework::FResourceManager::getInstance();
	const ImTextureID crosshairLocal = resources.getTexture(
		localGuns == 0 ? "../Data/Images/Crosshairs/Crosshair0.png" : localGuns == 1 ? "../Data/Images/Crosshairs/CrosshairMokebe1.png" : "../Data/Images/Crosshairs/CrosshairMokebe2.png"
	)->getSRV();
	const ImTextureID crosshairRemote = resources.getTexture(
		remoteGuns == 0 ? "../Data/Images/Crosshairs/Crosshair0.png" : remoteGuns == 1 ? "../Data/Images/Crosshairs/CrosshairTHC1.png" : "../Data/Images/Crosshairs/CrosshairTHC2.png"
	)->getSRV();

	const float radius = RELY(relativeCrosshairRadius);
	bool bWindowOpen = true;
	ImGui::SetNextWindowSize(REL(1.0f, 1.0f), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGui::Begin("Crosshair window", &bWindowOpen, INVISIBLE());

	ImGui::SetCursorPos(ImVec2(RELX(0.25f) - radius, RELY(0.5f) - radius));
	ImGui::Image(crosshairLocal, ImVec2(radius * 2.0f, radius * 2.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

	ImGui::SetCursorPos(ImVec2(RELX(0.75f) - radius, RELY(0.5f) - radius));
	ImGui::Image(crosshairRemote, ImVec2(radius * 2.0f, radius * 2.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	ImGui::End();
}

void ft_game::PlayerShootingSystem::drawScreenSeparator() const {
	//init images
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowBorderSize = 0.0f;

	framework::FResourceManager& resources = framework::FResourceManager::getInstance();
	const ImTextureID separator = resources.getTexture("../Data/Images/Separator.dds")->getSRV();

	bool bWindowOpen = true;
	ImGui::SetNextWindowSize(REL(1.0f, 1.0f), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGui::Begin("Separator window", &bWindowOpen, INVISIBLE());
	ImGui::SetCursorPos(REL(0.5f - relativeScreenSeparatorWidth, 0.0f));
	ImGui::Image(separator, REL(relativeScreenSeparatorWidth * 2.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	ImGui::End();
}

void ft_game::PlayerShootingSystem::onPostSceneLoaded(const EventPostSceneLoaded& event) {
	(void*)&event;
}

void ft_game::PlayerShootingSystem::onPlayerInput(const EventPlayerInput& event) {
	if (bInputFrozen_[event.bLocalPlayer ? 0 : 1])
		return;

	if (!event.bLeftAction && !event.bRightAction)
		return;

	ft_engine::Transform* cameraTransform = nullptr;
	getPlayerCamera(event.bLocalPlayer, &cameraTransform);

	framework::FAudio::getInstance().playOnce2D(framework::AudioClip2DType::SHOOT);

	ft_engine::Raycast ray;
	ray.maxLength = 1.0f; //gun->shootRange;
	ray.origin = cameraTransform->getWorldPosition();
	Vector3 forward = -cameraTransform->getWorldForward(); forward.Normalize();
	ray.direction = forward;

	uint8 layer = 
		1 << static_cast<uint8>(ft_engine::ELayer::Paintable)
		| 1 << static_cast<uint8>(ft_engine::ELayer::Pickable)
		| 1 << static_cast<uint8>(ft_engine::ELayer::Raft);
	layer |= 1 << static_cast<uint8>((event.bLocalPlayer) ? ft_engine::ELayer::PlayerAdditional_2 : ft_engine::ELayer::PlayerAdditional_1);

	EventPhysicsRaycast* eventPtr = new EventPhysicsRaycast(ray, layer);
	invokeNonConst<EventPhysicsRaycast>(eventPtr);
	if (eventPtr->bHit) {
		//Entity hit = eventPtr->hitEntity;
		//ft_engine::Collider* hitCollider = hit.getComponent<ft_engine::Collider>().get();
		//if (hitCollider->layer == ((event.bLocalPlayer) ? ft_engine::ELayer::PlayerAdditional_2 : ft_engine::ELayer::PlayerAdditional_1)) {
		//	handleAnotherPlayerHit(!event.bLocalPlayer, gun);
		//} else {
		//	handlePaintableHit(eventPtr->hitEntity, gun);
		//}
	}

	//Draw line renderer
	//const Vector3 endPosition = ray.origin + ((eventPtr->bHit) ? forward * eventPtr->distance : forward * gun->shootRange);
	//const Matrix worldPlayerMatrix = ft_engine::SceneManager::getInstance().getScene().getEntityManager().getComponent<ft_engine::Transform>(objectLifter->assignedTo_)->getWorldTransform();
	//drawLineRenderer(endPosition, gun, worldPlayerMatrix);

	delete eventPtr; //necessary because event is of non-const type
}

void ft_game::PlayerShootingSystem::onFreezeInput(const EventFreezeInput& event) {
	bInputFrozen_[event.bLocal ? 0 : 1] = event.bToFreeze;
}

void ft_game::PlayerShootingSystem::getPlayerCamera(bool bLocal, ft_engine::Transform** cameraTransform) const {
	EntityManager& entities = ft_engine::SceneManager::getInstance().getScene().getEntityManager();
	std::vector<Entity> cameras = entities.getEntitiesWithComponents<ft_engine::Transform, ft_render::Camera, ft_engine::Player>();
	for (auto& it : cameras) {
		ft_engine::Player* playerComponent = it.getComponent<ft_engine::Player>().get();
		if (playerComponent->bLocal == bLocal) {
			*cameraTransform = it.getComponent<ft_engine::Transform>().get();
			break;
		}
	}
}
