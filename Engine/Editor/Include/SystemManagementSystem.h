#pragma once

#include "Global.h"
#include "EECS.h"
#include "EngineEvent.h"
#include "EditorEvent.h"

namespace ft_editor
{
	class SystemManagementSystem : public eecs::System<SystemManagementSystem>, public eecs::IReceiver<SystemManagementSystem>
	{
	public:
		SystemManagementSystem();
		SystemManagementSystem(const SystemManagementSystem& another) = delete;
		SystemManagementSystem(SystemManagementSystem&& another) = delete;
		~SystemManagementSystem();
		SystemManagementSystem& operator=(const SystemManagementSystem& another) = delete;
		SystemManagementSystem& operator=(SystemManagementSystem&& another) = delete;

		void update(eecs::EntityManager& entities, double deltaTime) override;

	private:
		struct SystemData;

		void onSystemLoaded(const EventSystemLoaded& event);
		void onSerializeSystem(const EventSerializeSystem& event);
		void onPostSceneLoaded(const EventPostSceneLoaded& event);

		void changeSystemOrder(int order);

		std::vector<SystemData> inactiveSystems_;
		std::vector<SystemData> activeSystems_;
		std::pair<int, bool> currentSystemItemIndexActive_;

		static bool systemListboxGetActive(void* data, int idx, const char** outText);
		static bool systemListboxGetInactive(void* data, int idx, const char** outText);
	};

	struct SystemManagementSystem::SystemData
	{
		std::string name;
		float period;
	};
}
