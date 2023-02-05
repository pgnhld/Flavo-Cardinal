#include "FlavoRootsGame/PlayerShootingSystem.h"
#include "SceneManager.h"
#include "Physics/Transform.h"
#include "FlavoRootsGame/Player.h"
#include "Rendering/StaticMeshRenderer.h"
#include "Rendering/SkinnedMeshRenderer.h"
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
#include "FlavoRootsGame/WeaponGun.h"

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
	// Update weapon cooldowns
	std::vector<Entity> gunEntities = entities.getEntitiesWithComponents<WeaponGun>();
	for (Entity gunEnt : gunEntities) {
		gunEnt.getComponent<WeaponGun>()->timeSinceLastGunShot += deltaTime;
	}

	//Check line renderers lifetime
	for (size_t i = 0; i < activeLineRenderers_.size(); ++i) {
		if (!activeLineRenderers_[i].entity.isValid()) {
			continue;
		}

		LineRenderer* lineRenderer = activeLineRenderers_[i].entity.getComponent<LineRenderer>().get();
		lineRenderer->timer += framework::FTime::deltaTime;
		if (lineRenderer->timer > lineRenderer->maxTime) {
			entities.destroy(activeLineRenderers_[i].entity);
		}
		else {
			float currDiamater = activeLineRenderers_[i].diameter * ((lineRenderer->maxTime - lineRenderer->timer) / lineRenderer->maxTime);
			activeLineRenderers_[i].entity.getComponent<ft_engine::Transform>()->setLocalTransform(Matrix::Compose(activeLineRenderers_[i].position, activeLineRenderers_[i].rotation, Vector3(currDiamater, currDiamater, activeLineRenderers_[i].length)));
		}
	}

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
	EntityManager& entities = ft_engine::SceneManager::getInstance().getScene().getEntityManager();
	std::vector<Entity> guns = entities.getEntitiesWithComponents<ft_engine::Player, WeaponGun, ft_engine::Transform>();

	framework::FResourceManager& resources = framework::FResourceManager::getInstance();
	const ImTextureID crosshair = resources.getTexture("../Data/Images/Crosshairs/Crosshair.png")->getSRV();

	const float radius = RELY(relativeCrosshairRadius);
	bool bWindowOpen = true;
	ImGui::SetNextWindowSize(REL(1.0f, 1.0f), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGui::Begin("Crosshair window", &bWindowOpen, INVISIBLE());

	ImGui::SetCursorPos(ImVec2(RELX(0.25f) - radius, RELY(0.5f) - radius));
	ImGui::Image(crosshair, ImVec2(radius * 2.0f, radius * 2.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

	ImGui::SetCursorPos(ImVec2(RELX(0.75f) - radius, RELY(0.5f) - radius));
	ImGui::Image(crosshair, ImVec2(radius * 2.0f, radius * 2.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	ImGui::End();
}

void ft_game::PlayerShootingSystem::drawScreenSeparator() const {
	//init images
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowBorderSize = 0.0f;

	framework::FResourceManager& resources = framework::FResourceManager::getInstance();
	const ImTextureID separator = resources.getTexture("../Data/Images/Separator.png")->getSRV();

	bool bWindowOpen = true;
	ImGui::SetNextWindowSize(REL(1.0f, 1.0f), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGui::Begin("Separator window", &bWindowOpen, INVISIBLE());
	ImGui::SetCursorPos(ImVec2(RELX(0.5f) - 2.5f, 0.0f));
	ImGui::Image(separator, ImVec2(5.0f, 1080.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
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

	EntityManager& entities = ft_engine::SceneManager::getInstance().getScene().getEntityManager();
	std::vector<Entity> gun_entities = entities.getEntitiesWithComponents<WeaponGun, ft_engine::Transform, ft_engine::Player>();
	auto get_player_gun = [&](bool bLocal) -> Entity* {
		for (Entity& ent : gun_entities)
		{
			if (bLocal == ent.getComponent<ft_engine::Player>()->bLocal)
				return &ent;
		}
		return nullptr;
	};

	Entity* playerEntity = get_player_gun(event.bLocalPlayer);
	if (!playerEntity) {
		return;
	}
	if (!playerEntity->hasComponent<WeaponGun>()) {
		return;
	}

	WeaponGun* gun = playerEntity->getComponent<WeaponGun>().get();
	if (!gun) {
		return;
	}

	if (gun->timeSinceLastGunShot < WeaponGun::cooldown) {
		return;
	}

	std::vector<Entity> player_entities = entities.getEntitiesWithComponents<ft_engine::CharacterController, ft_engine::Transform, ft_engine::Player>();
	auto get_player = [&](bool bLocal) -> Entity* {
		for (Entity& ent : player_entities)
		{
			if (bLocal == ent.getComponent<ft_engine::Player>()->bLocal)
				return &ent;
		}
		return nullptr;
	};

	framework::FAudio::getInstance().playOnce2D(framework::AudioClip2DType::SHOOT);

	ft_engine::Raycast ray;
	ray.maxLength = gun->attackRange;
	ray.origin = cameraTransform->getWorldPosition();
	Vector3 forward = -cameraTransform->getWorldForward(); forward.Normalize();
	ray.direction = forward;

	uint8 layer = 
		1 << static_cast<uint8>(ft_engine::ELayer::Paintable)
		| 1 << static_cast<uint8>(ft_engine::ELayer::Pickable)
		| 1 << static_cast<uint8>(ft_engine::ELayer::Raft);
	layer |= 1 << static_cast<uint8>(ft_engine::ELayer::Player);

	EventPhysicsRaycast* eventPtr = new EventPhysicsRaycast(ray, layer);
	invokeNonConst<EventPhysicsRaycast>(eventPtr);
	if (eventPtr->bHit) {
		Entity hit = eventPtr->hitEntity;
		ft_engine::Collider* hitCollider = hit.getComponent<ft_engine::Collider>().get();
		if (hitCollider->layer == ft_engine::ELayer::Player) {
			handleAnotherPlayerHit(!event.bLocalPlayer, gun);
		} else {
			//handlePaintableHit(eventPtr->hitEntity, gun);
		}
	}

	gun->timeSinceLastGunShot = 0.0f;

	//Draw line renderer
	const Vector3 endPosition = ray.origin + ((eventPtr->bHit) ? forward * eventPtr->distance : forward * gun->attackRange);
	const Matrix worldPlayerMatrix = ft_engine::SceneManager::getInstance().getScene().getEntityManager().getComponent<ft_engine::Transform>(*get_player(event.bLocalPlayer))->getWorldTransform();
	drawLineRenderer(endPosition, gun, worldPlayerMatrix);

	delete eventPtr; //necessary because event is of non-const type
}

void ft_game::PlayerShootingSystem::handleAnotherPlayerHit(bool bOtherLocal, WeaponGun* ourGun) {
	EntityManager& entities = ft_engine::SceneManager::getInstance().getScene().getEntityManager();
	std::vector<Entity> playerEntities = entities.getEntitiesWithComponents<ft_engine::CharacterController, ft_engine::Player>();
	Entity* entity_controller = nullptr;
	ft_engine::CharacterController* controller = nullptr;
	for (Entity e : playerEntities) {
		ft_engine::Player* p = e.getComponent<ft_engine::Player>().get();
		if (p->bLocal != bOtherLocal)
			continue;

		entity_controller = &e;
		controller = e.getComponent<ft_engine::CharacterController>().get();
	}

	if (ASSERT_FAIL(controller != nullptr, "Controller is null"))
		return;

	if (controller->isBulletHitDeathActive)
		return;

	controller->isBulletHitDeathActive = true;

	framework::FAudio::getInstance().playOnce2D(framework::AudioClip2DType::BULLET_HIT_REACTION);

	PaintEffectData* data = new PaintEffectData();
	data->bLocalHit = bOtherLocal;
	data->entity_to_respawn = *entity_controller;

	const Coroutine tmpCoroutinehandle = START_COROUTINE(
		&PlayerShootingSystem::drawPaintEffect,
		PaintEffectData*,
		data
	);

	STOP_COROUTINE(controller->rightPaintCoroutine);
	controller->rightPaintCoroutine = tmpCoroutinehandle;
}

IEnumerator ft_game::PlayerShootingSystem::drawPaintEffect(CoroutineArg arg) {
	PaintEffectData* data = static_cast<PaintEffectData*>(arg);

	EventEntityRespawn* event = new EventEntityRespawn();
	event->entityToRespawn = data->entity_to_respawn;
	invoke<EventEntityRespawn>(event);

	//ImTextureID effectTexture = IMAGE("ScreenEffects/Hit.png");

	float timer = 0.0f;
	while (timer < data->duration) {
		float t = timer / data->duration;
		t = std::min(t, 1.0f);

		data->entity_to_respawn.getComponent<ft_render::SkinnedMeshRenderer>()->bEnabledOther = (int)(t * 10.0f) % 2 == 0;

		timer += framework::FTime::deltaTime;
		YIELD_RETURN_NULL();
	}

	data->entity_to_respawn.getComponent<ft_render::SkinnedMeshRenderer>()->bEnabledOther = true;
}

void ft_game::PlayerShootingSystem::drawLineRenderer(Vector3 endPosition, WeaponGun* gun, Matrix playerWorldMatrix) {
	EntityManager& entities = ft_engine::SceneManager::getInstance().getScene().getEntityManager();

	//Our renderer
	Entity lineOwn = entities.create();
	eecs::ComponentHandle<LineRenderer> lineRendererOwn = lineOwn.addComponent<LineRenderer>();
	eecs::ComponentHandle<ft_engine::Player> playerOwn = lineOwn.addComponent<ft_engine::Player>();
	playerOwn->bLocal = entities.getComponent<ft_engine::Player>(gun->assignedTo_)->bLocal;

	eecs::ComponentHandle<ft_engine::Transform> transformOwn = lineOwn.addComponent<ft_engine::Transform>();
	ft_engine::SceneManager::getInstance().getScene().assignEntityParent(lineOwn.getId().index_, 0);
	transformOwn->setBoundingBox(DirectX::BoundingBox(Vector3(0.0f, 0.0f, -0.5f), Vector3(0.5f, 0.5f, 0.5f)));
	Vector3 ourStartPosition = gun->ownGunMatrix.Translation();
	const float ownTrueDiameter = lineRendererOwn->diameter;
	const Vector3 ownScale = Vector3(ownTrueDiameter, ownTrueDiameter, (endPosition - ourStartPosition).Length());
	const Matrix ownRotationMatrix = Matrix::CreateLookAt(ourStartPosition, endPosition, Vector3::Up).Invert();
	Quaternion ownRotation = Quaternion::CreateFromRotationMatrix(ownRotationMatrix);
	const Matrix ownMatrix = Matrix::Compose(ourStartPosition, ownRotation, ownScale);
	transformOwn->setLocalTransform(ownMatrix);
	invoke<EventCertainTransformUpdate>(new EventCertainTransformUpdate(transformOwn.get(), ft_engine::SceneManager::getInstance().getScene().getRootTransform().get()));

	eecs::ComponentHandle<ft_render::StaticMeshRenderer> rendererOwn = lineOwn.addComponent<ft_render::StaticMeshRenderer>();
	rendererOwn->reloadMesh(framework::FMeshIdentifier(lineRendererOwn->cylinderMeshPath, 1));
	framework::FMaterial& ownMaterial = rendererOwn->getMaterial();
	ownMaterial.diffuse = framework::FResourceManager::getInstance().getTexture(lineRendererOwn->diffusePath);
	ownMaterial.colorTint = lineRendererOwn->lineColor;
	ownMaterial.specialEffect = 100.0f;
	rendererOwn->bEnabledOther = false;
	activeLineRenderers_.emplace_back(LineRendererTransformData(std::move(ownRotation), std::move(ourStartPosition), ownTrueDiameter, ownScale.z, std::move(lineOwn)));

	//Other renderer
	Entity lineOther = entities.create();
	eecs::ComponentHandle<LineRenderer> lineRendererOther = lineOther.addComponent<LineRenderer>();
	eecs::ComponentHandle<ft_engine::Player> playerOther = lineOther.addComponent<ft_engine::Player>();
	playerOther->bLocal = entities.getComponent<ft_engine::Player>(gun->assignedTo_)->bLocal;

	eecs::ComponentHandle<ft_engine::Transform> transformOther = lineOther.addComponent<ft_engine::Transform>();
	ft_engine::SceneManager::getInstance().getScene().assignEntityParent(lineOther.getId().index_, 0);
	transformOther->setBoundingBox(DirectX::BoundingBox(Vector3(0.0f, 0.0f, -0.5f), Vector3(0.5f, 0.5f, 0.5f)));
	Vector3 otherStartPosition = (gun->otherGunMatrix * playerWorldMatrix).Translation();
	const float otherTrueDiameter = lineRendererOther->diameter;
	const Vector3 otherScale = Vector3(otherTrueDiameter, otherTrueDiameter, (endPosition - otherStartPosition).Length());
	const Matrix otherRotationMatrix = Matrix::CreateLookAt(otherStartPosition, endPosition, Vector3::Up).Invert();
	Quaternion otherRotation = Quaternion::CreateFromRotationMatrix(otherRotationMatrix);
	const Matrix otherMatrix = Matrix::Compose(otherStartPosition, otherRotation, otherScale);
	transformOther->setLocalTransform(otherMatrix);
	invoke<EventCertainTransformUpdate>(new EventCertainTransformUpdate(transformOther.get(), ft_engine::SceneManager::getInstance().getScene().getRootTransform().get()));

	eecs::ComponentHandle<ft_render::StaticMeshRenderer> rendererOther = lineOther.addComponent<ft_render::StaticMeshRenderer>();
	rendererOther->reloadMesh(framework::FMeshIdentifier(lineRendererOther->cylinderMeshPath, 1));
	framework::FMaterial& otherMaterial = rendererOther->getMaterial();
	otherMaterial.diffuse = framework::FResourceManager::getInstance().getTexture(lineRendererOther->diffusePath);
	otherMaterial.colorTint = lineRendererOther->lineColor;
	otherMaterial.specialEffect = 100.0f;
	rendererOther->bEnabledOwn = false;
	rendererOther->bEnabledOther = true;
	activeLineRenderers_.emplace_back(LineRendererTransformData(std::move(otherRotation), std::move(otherStartPosition), otherTrueDiameter, otherScale.z, std::move(lineOther)));
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
