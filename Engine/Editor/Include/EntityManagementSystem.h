#pragma once

#include "Global.h"
#include "EECS.h"
#include "Physics/Transform.h"
#include "EngineEvent.h"

namespace ft_editor
{
	struct ClipboardComponent
	{
		nlohmann::json json;
		uint32 type;
	};

	class EntityManagementSystem : public eecs::System<EntityManagementSystem>, public eecs::IReceiver<EntityManagementSystem>, public eecs::IInvoker
	{
	public:
		EntityManagementSystem();
		EntityManagementSystem(const EntityManagementSystem& another) = delete;
		EntityManagementSystem(EntityManagementSystem&& another) = delete;
		~EntityManagementSystem();
		EntityManagementSystem& operator=(const EntityManagementSystem& another) = delete;
		EntityManagementSystem& operator=(EntityManagementSystem&& another) = delete;

		void update(EntityManager& entities, double deltaTime) override;

	private:
		void onPostSceneLoaded(const EventPostSceneLoaded& event);
		void checkCameraRaycast();

		void addTreeNode(EntityManager& entities, eecs::ComponentHandle<ft_engine::Transform> parentTransform);
		void updateEntityView(EntityManager& entities);
		void updateComponentView(EntityManager& entities);
		void addComponentView(EntityManager& entities);
		void removeComponentView(EntityManager& entites);
		void loadMesh(EntityManager& entities);
		void duplicateEntity(EntityManager& entities, eecs::ComponentHandle<ft_engine::Transform> entityTransform, eecs::ComponentHandle<ft_engine::Transform> parent);
		void duplicateComponent(EntityManager& entities, uint32 componentType, const nlohmann::json& json, Entity destinationEntity);

		void renderComponentEditView(EntityManager& entities, std::string componentType);
		bool renderComponentRecursively(nlohmann::json& j, std::string idString);

		static bool entityComponentGetter(void* data, int idx, const char** outText);
		static bool entityAddListGetter(void* data, int idx, const char** outText);

		eecs::ComponentHandle<ft_engine::Transform> currentEntityTransform_;
		std::vector<uint32> currentEntityComponentTypes_;
		int32 currentComponentListIndex_;
		bool bComponentChanged = true;
		ClipboardComponent clipboardComponent_;
		
		bool bAddComponentViewActivated_ = false;
		bool bRemoveComponentViewActivated_ = false;

		Entity cameraEntity_;

		std::unordered_map<std::string, float> componentFloatValues_;
		std::unordered_map<std::string, int> componentIntValues_;
		std::unordered_map<std::string, bool> componentBoolValues_;
	};
}
