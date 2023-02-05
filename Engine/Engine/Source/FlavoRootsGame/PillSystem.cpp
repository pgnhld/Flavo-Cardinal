#include "FlavoRootsGame/PillSystem.h"
#include "Physics/Transform.h"
#include "FlavoRootsGame/Pill.h"
#include "FlavoRootsGame/Player.h"
#include "Physics/TriggerCollider.h"
#include "EngineEvent.h"
#include "Logger.h"
#include "imgui.h"
#include "FWindow.h"
#include "FResourceManager.h"
#include "FInput.h"
#include "SceneManager.h"
#include "ImGuiExtension.h"
#include "imgui_internal.h"
#include "FAudio.h"

ft_game::PillSystem::PillSystem() {
	subscribe<EventPostSceneLoaded>(this, &PillSystem::onSceneLoaded);
}

ft_game::PillSystem::~PillSystem() {

}

void ft_game::PillSystem::update(eecs::EntityManager& entities, double deltaTime) {
	//Rotate pill
	std::vector<Entity> pill_entities = entities.getEntitiesWithComponents<ft_engine::Transform, Pill, ft_engine::TriggerCollider>();
	for (Entity& pill_entity : pill_entities)
	{
		ft_engine::Transform* pill_transform = pill_entity.getComponent<ft_engine::Transform>().get();
		Pill* pill = pill_entity.getComponent<Pill>().get();
		pill_transform->rotate(DEG2RAD(pill->rotationSpeedDeg) * static_cast<float>(deltaTime), Vector3::UnitY);
	}

	// UI
	currentGameTime += deltaTime;

	if (currentGameTime > maxGameTime)
	{
		if (!bEnded)
		{
			EventTimeRanOut* event_time = new EventTimeRanOut();
			invoke<EventTimeRanOut>(event_time);
		}

		bEnded = true;
	}

	const float rel_offset_y = 0.05f;

	auto render_pill_image = [&](int num_pills, int num_collected_pills, float start_x, float end_x) {
		const float pill_width_x = 95.0f;
		const float pill_width_y = 52.0f;
		const float offset_x_rel = 0.3f;

		float total_available_size = (end_x - start_x) * ImGui::Flavo::getInstance().windowSize.x;
		float theoretical_x = pill_width_x * (float)num_pills + offset_x_rel * pill_width_x * (float)(num_pills - 1);
		float scale_x = total_available_size / theoretical_x;
		ImVec2 image_size(scale_x * pill_width_x, scale_x * pill_width_y);

		float start_x_pix = start_x * ImGui::Flavo::getInstance().windowSize.x;
		for (int i = 0; i < num_pills; ++i)
		{
			bool is_collected = (i + 1) <= num_collected_pills;
			ImGui::SetCursorPos(ImVec2(start_x_pix + (float)i * scale_x * (pill_width_x + offset_x_rel * pill_width_x), RELY(rel_offset_y)));
			ImGui::Image(IMAGE((is_collected ? "Pill.png" : "Pill_Empty.png")), image_size, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		}
	};

	bool bWindowOpen = true;
	ImGui::SetNextWindowSize(REL(1.0f, 1.0f), ImGuiCond_Always);
	ImGui::SetNextWindowPos(REL(0.0f, 0.0f));
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGui::Begin("PillSystem UI", &bWindowOpen, INVISIBLE());

	// OldPapa, consider pills still left
	render_pill_image(maxPillsToCollect, maxPillsToCollect - pillsCollectedSoFar, 0.15f, 0.35f);
	// YoungMagick, consider pills already collected
	render_pill_image(maxPillsToCollect, pillsCollectedSoFar, 0.65f, 0.85f);

	// Timer
	{
		const float digit_width_x = 95.0f;
		const float digit_width_y = 52.0f;
		const float offset_x_rel = 0.04f;
		const float start_x = 0.4f;
		const float end_x = 0.6f;
		const float num_digits = 4.0f;

		float total_available_size = (end_x - start_x) * ImGui::Flavo::getInstance().windowSize.x;
		float theoretical_x = digit_width_x * num_digits + offset_x_rel * digit_width_x * (num_digits - 1.0f);
		float scale_x = total_available_size / theoretical_x;
		ImVec2 image_size(scale_x * digit_width_x, scale_x * digit_width_y);

		float start_x_pix = start_x * ImGui::Flavo::getInstance().windowSize.x;

		float time_left = maxGameTime - currentGameTime;
		if (time_left < 0.0f)
			time_left = 0.0001f;
		int left_hundreds = (int)(time_left / 100.0f);
		int left_seconds = time_left - 100.0f * (float)left_hundreds;
		int sec_digit1 = (int)(left_seconds / 10.0f);
		int sec_digit2 = left_seconds - 10.0f * (float)sec_digit1;

		ImVec2 frame_size(414.0f, 75.0f);
		ImGui::SetCursorPos(ImVec2(start_x_pix - 15.0f, 36.0f));
		ImGui::Image(IMAGE("Frame.png"), frame_size, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));

		int digits[] = { 0, left_hundreds, sec_digit1, sec_digit2 };
		for (int i = 0; i < num_digits; ++i)
		{
			ImGui::SetCursorPos(ImVec2(start_x_pix + (float)i * scale_x * (digit_width_x + offset_x_rel * digit_width_x), 43.0f));
			ImGui::Image(IMAGE_DIGIT(digits[i]), image_size, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		}
	}

	ImGui::End();
}

void ft_game::PillSystem::fixedUpdate(EntityManager& entities, double fixedDeltaTime)
{
	std::vector<Entity> pill_entities = entities.getEntitiesWithComponents<ft_engine::Transform, Pill, ft_engine::TriggerCollider>();
	for (Entity& pill_entity : pill_entities)
	{
		ft_engine::TriggerCollider* pill_trigger = pill_entity.getComponent<ft_engine::TriggerCollider>().get();
		ft_engine::Transform* pill_transform = pill_entity.getComponent<ft_engine::Transform>().get();
		Pill* pill = pill_entity.getComponent<Pill>().get();

		Matrix worldMatrix = pill_transform->getWorldTransform();
		std::unique_ptr<EventPhysicsTriggerCollider> event = std::make_unique<EventPhysicsTriggerCollider>(*pill_trigger, worldMatrix, 1 << static_cast<uint8>(ft_engine::ELayer::Player));
		invokeNonConst(event.get());

		if (event->foundEntities.empty())
			continue;

		bool young_in_trigger = false;
		for (Entity& ent : event->foundEntities)
		{
			if (!ent.getComponent<ft_engine::Player>()->bLocal)
				young_in_trigger = true;
		}

		if (young_in_trigger)
		{
			++pillsCollectedSoFar;

			std::vector<eecs::ComponentHandle<ft_engine::Transform>> destroyable = pill_entity.getComponent<ft_engine::Transform>()->getChildren();
			for (auto& tr : destroyable)
			{
				entities.destroy(tr->assignedTo_);
			}
			entities.destroy(pill_entity);

			framework::FAudio::getInstance().playOnce2D(framework::AudioClip2DType::PILL);
			framework::FAudio::getInstance().playOnce2D(framework::AudioClip2DType::PILL_REACTION);

			const bool should_game_end = pillsCollectedSoFar >= maxPillsToCollect;
			if (should_game_end)
			{
				EventAllPillsCollected* event_all = new EventAllPillsCollected();
				invoke<EventAllPillsCollected>(event_all);
			}
		}
	};
}

void ft_game::PillSystem::onSceneLoaded(const EventPostSceneLoaded& event)
{
	//EntityManager& entities = ft_engine::SceneManager::getInstance().getScene().getEntityManager();
	//std::vector<Entity> pill_entities = entities.getEntitiesWithComponents<ft_engine::Transform, Pill, ft_engine::TriggerCollider>();
	//totalNumPills = pill_entities.size();
}
