#pragma once

#include "Global.h"
#include "EECS.h"
#include "EngineEvent.h"
#include "CoroutineManager.h"
#include "Rendering/StaticMeshRenderer.h"
#include "ImGuiExtension.h"
#include "Rendering/PointLight.h"
#include "Physics/CharacterController.h"

FLAVO_SYSTEM(ft_game, PlayerShootingSystem)
namespace ft_game
{
	class PaintableCube;

	class PlayerShootingSystem : public eecs::System<PlayerShootingSystem>, public eecs::IReceiver<PlayerShootingSystem>, public eecs::IInvoker
	{
	public:
		PlayerShootingSystem();
		~PlayerShootingSystem();

		void update(eecs::EntityManager& entities, double deltaTime) override;

	private:
		struct LineRendererTransformData;

		void drawCrosshair();
		void drawScreenSeparator() const;

		void onPostSceneLoaded(const EventPostSceneLoaded& event);
		void onPlayerInput(const EventPlayerInput& event);
		void onFreezeInput(const EventFreezeInput& event);
		void getPlayerCamera(bool bLocal, OUT ft_engine::Transform** cameraTransform) const;

		const float relativeCrosshairRadius = 0.03f;
		const float relativeScreenSeparatorWidth = 0.003f;
		bool bInputFrozen_[2] = { false, false };

		std::vector<LineRendererTransformData> activeLineRenderers_;
	};

	struct PlayerShootingSystem::LineRendererTransformData
	{
		LineRendererTransformData(Quaternion&& rotation, Vector3&& position, float diameter, float length, Entity&& entity) : rotation(rotation), position(position), diameter(diameter), length(length), entity(entity)
		{ }

		Quaternion rotation;
		Vector3 position;
		float diameter;
		float length;
		Entity entity;
	};
}