#include "EntityManagementSystem.h"
#include "imgui.h"
#include "ImGuiExtension.h"
#include "SceneManager.h"
#include "Logger.h"
#include "Metadata.h"
#include "SystemCall.h"
#include <filesystem>
#include <Shlwapi.h>
#include "FResourceManager.h"
#include "FLoader.h"
#include "Rendering/SkinnedMeshRenderer.h"
#include "Rendering/StaticMeshRenderer.h"
#include "imgui_internal.h"
#include "EngineEvent.h"
#include "Rendering/Camera.h"
#include "FInput.h"

ft_editor::EntityManagementSystem::EntityManagementSystem() {

}

ft_editor::EntityManagementSystem::~EntityManagementSystem() {

}

void ft_editor::EntityManagementSystem::update(eecs::EntityManager& entities, double deltaTime) {
	//Editor click
	if (framework::FInput::getMouse().getState().rightButton)
		checkCameraRaycast();

	updateEntityView(entities);
	updateComponentView(entities);
	addComponentView(entities);
	removeComponentView(entities);
}

void ft_editor::EntityManagementSystem::onPostSceneLoaded(const EventPostSceneLoaded& event) {
	EntityManager& entities = ft_engine::SceneManager::getInstance().getScene().getEntityManager();
	std::vector<Entity> cameraEntities = entities.getEntitiesWithComponents<ft_render::Camera, ft_engine::Metadata>();
	for (Entity e : cameraEntities) {
		ft_engine::Metadata* metadata = e.getComponent<ft_engine::Metadata>().get();
		if (metadata->name.substr(0, 3) == "###") {
			cameraEntity_ = e;
			break;
		}
	}
}

void ft_editor::EntityManagementSystem::checkCameraRaycast() {
	if (!cameraEntity_.isValid())
		return;

	if (!cameraEntity_.hasComponent<ft_engine::Transform>())
		return;

	ft_engine::Transform* transform = cameraEntity_.getComponent<ft_engine::Transform>().get();
	ft_engine::Raycast ray;
	ray.origin = transform->getWorldPosition();
	ray.direction = -transform->getWorldForward();
	ray.maxLength = 100.0f;

	EventPhysicsRaycast* raycast = new EventPhysicsRaycast(ray, UINT8_MAX);
	invokeNonConst<EventPhysicsRaycast>(raycast);
	if (raycast->bHit) {
		if (raycast->hitEntity.hasComponent<ft_engine::Transform>()) {
			currentEntityTransform_ = raycast->hitEntity.getComponent<ft_engine::Transform>();
			bComponentChanged = true;
			componentFloatValues_.clear();
			componentBoolValues_.clear();
			componentIntValues_.clear();
		}
	}

	delete raycast;
}

void ft_editor::EntityManagementSystem::addTreeNode(EntityManager& entities, eecs::ComponentHandle<ft_engine::Transform> parentTransform) {
	const std::vector<eecs::ComponentHandle<ft_engine::Transform>>& children = parentTransform->getChildren();
	for (const auto& it : children) {
		if (!it.isValid()) {
			LOG_W("Wrong");
			continue;
		}
		eecs::ComponentHandle<ft_engine::Metadata> metadata = entities.getComponent<ft_engine::Metadata>(it.get()->assignedTo_);
		std::string entityName = (metadata.isValid()) ? metadata->name : "--UNKNOWN--";
		//if (entityName.substr(0, 3) == "---")
		//	continue;

		const int treeFlags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ((it.get()->getChildren().empty()) ? ImGuiTreeNodeFlags_Leaf : 0);
		const bool opened = ImGui::TreeNodeEx(&it.get()->assignedTo_.index_, treeFlags, entityName.c_str());
		if (it == currentEntityTransform_) {
			ImGuiWindow* window = ImGui::GetCurrentWindow();
			const ImRect frame = ImRect(ImVec2(window->DC.CursorPos.x, window->DC.CursorPos.y - 19.0f), ImVec2(window->Pos.x + ImGui::GetContentRegionMax().x, window->DC.CursorPos.y - 3.0f));
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(1.0f, 1.0f, 0.0f, 0.5f));
			const ImU32 col = ImGui::GetColorU32(ImGuiCol_HeaderActive);
			ImGui::RenderFrame(frame.Min, frame.Max, col, false, 0.0f);
			ImGui::PopStyleColor();
		}

		if (ImGui::IsItemClicked()) {
			currentEntityTransform_ = it;
			bComponentChanged = true;
			componentFloatValues_.clear();
			componentBoolValues_.clear();
			componentIntValues_.clear();
		}
		if (opened) {
			addTreeNode(entities, it);
			ImGui::TreePop();
		}
	}
}

void ft_editor::EntityManagementSystem::updateEntityView(EntityManager& entities) {
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.87f, 0.87f, 0.6f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_Text] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	ImGui::PushFont(ImGui::Flavo::getInstance().fontMap[ImGui::Flavo::FontType::CALIBRI_BOLD_26]);
	ImGui::Begin("Hierarchy", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_ResizeFromAnySide);
		ImGui::PushFont(ImGui::Flavo::getInstance().fontMap[ImGui::Flavo::FontType::CALIBRI_REGULAR_16]);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
		if (ImGui::Button("E_Add")) {
			ft_engine::Scene& scene = ft_engine::SceneManager::getInstance().getScene();
			Entity ent = entities.create();
			const eecs::ComponentHandle<ft_engine::Transform> trHandle = ent.addComponent<ft_engine::Transform>();
			scene.assignTransformParent(trHandle, scene.getRootTransform());
			eecs::ComponentHandle<ft_engine::Metadata> metadata = ent.addComponent<ft_engine::Metadata>();
			metadata->name = "-UNKNOWN-";
		}
		ImGui::SameLine();
		if (ImGui::Button("E_Remove")) {
			try {
				ft_engine::Scene& scene = ft_engine::SceneManager::getInstance().getScene();
				scene.destroyRecursively(currentEntityTransform_->assignedTo_);
				currentEntityTransform_ = eecs::ComponentHandle<ft_engine::Transform>();
			} catch (std::exception& ex) {
				LOG_W(format("Couldn't remove entity: ", currentEntityTransform_->assignedTo_.index_));
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("E_Duplicate")) {
			try {
				duplicateEntity(entities, currentEntityTransform_, currentEntityTransform_->getParent());
			} catch (std::exception& ex) {
				LOG_W(format("Couldn't duplicate entity: ", currentEntityTransform_->assignedTo_.index_));
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("E_Load")) {
			try {
				loadMesh(entities);
			} catch (std::exception& ex) {
				LOG_W("Couldn't load mesh");
			}
		}
		ImGui::PopStyleColor(2);
		ImGui::PopFont();
		ImGui::Separator();

		ImGui::PushItemWidth(-1);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
		ImGui::Text("Parent");
		ImGui::PopStyleColor();
		if (currentEntityTransform_.isValid() && currentEntityTransform_->assignedTo_ != 0) {
			int32 parentIndex = currentEntityTransform_->getParent()->assignedTo_.index_;
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_TextStep, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
			if (ImGui::InputInt("Parent", &parentIndex, 1, 0, ImGuiInputTextFlags_EnterReturnsTrue)) {
				try {
					ft_engine::Scene& scene = ft_engine::SceneManager::getInstance().getScene();
					scene.assignTransformParent(
						currentEntityTransform_,
						entities.getComponent<ft_engine::Transform>(entities.get(parentIndex))
					);
				} catch (std::exception& ex) {
					//TODO: Some error message
				}
			}
			ImGui::PopStyleColor(3);
		}
		ImGui::PopItemWidth();
		ImGui::Separator();

		style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		ImGui::PushFont(ImGui::Flavo::getInstance().fontMap[ImGui::Flavo::FontType::CALIBRI_REGULAR_16]);
		eecs::ComponentHandle<ft_engine::Transform> rootTransform = ft_engine::SceneManager::getInstance().getScene().getRootTransform();
		const bool opened = ImGui::TreeNodeEx(&rootTransform->assignedTo_.index_,
			ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen, "--ROOT--");
		if (ImGui::IsItemClicked()) {
			currentEntityTransform_ = eecs::ComponentHandle<ft_engine::Transform>();
			bComponentChanged = true;
			componentFloatValues_.clear();
			componentBoolValues_.clear();
			componentIntValues_.clear();
		} if (opened) {
			addTreeNode(entities, rootTransform);
			ImGui::TreePop();
		}
		ImGui::PopFont();
	ImGui::End();
	ImGui::PopFont();
}

void ft_editor::EntityManagementSystem::updateComponentView(EntityManager& entities) {
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.87f, 0.87f, 0.6f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_Text] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	ImGui::PushFont(ImGui::Flavo::getInstance().fontMap[ImGui::Flavo::FontType::CALIBRI_BOLD_26]);
	ImGui::Begin("Entity", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_ResizeFromAnySide | ImGuiWindowFlags_AlwaysHorizontalScrollbar);
		if (currentEntityTransform_.isValid()) {
			ImGui::PushFont(ImGui::Flavo::getInstance().fontMap[ImGui::Flavo::FontType::CALIBRI_REGULAR_20]);
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Entity ID:");

			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), std::to_string(currentEntityTransform_->assignedTo_.index_).c_str());

			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Components:");
			ImGui::PopFont();

			ImGui::PushFont(ImGui::Flavo::getInstance().fontMap[ImGui::Flavo::FontType::CALIBRI_REGULAR_16]);
			ImGui::PushItemWidth(-1);
			style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			currentEntityComponentTypes_ = entities.getEntityActiveComponentsType(currentEntityTransform_->assignedTo_);
			if (ImGui::ListBox("Components", &currentComponentListIndex_, &entityComponentGetter, static_cast<void*>(this), currentEntityComponentTypes_.size(), 9)) {
				bComponentChanged = true;
				componentFloatValues_.clear();
				componentBoolValues_.clear();
				componentIntValues_.clear();
			}

			style.Colors[ImGuiCol_Button] = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
			ImGui::PushItemWidth(-1);
			if (ImGui::Button("Add"))
				bAddComponentViewActivated_ = true;
			ImGui::SameLine();
			if (ImGui::Button("Remove")) {
				bRemoveComponentViewActivated_ = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("Copy")) {
				clipboardComponent_.type = currentEntityComponentTypes_[currentComponentListIndex_];
				clipboardComponent_.json = entities.getComponentBase(currentEntityTransform_->assignedTo_, clipboardComponent_.type)->serialize();
			}
			ImGui::SameLine();
			if (ImGui::Button("Paste")) {
				try {
					duplicateComponent(entities, clipboardComponent_.type, clipboardComponent_.json, entities.get(currentEntityTransform_->assignedTo_));
				} catch (std::exception& ex) {
					LOG_W("Couldn't paste component");
				}
			}
			ImGui::PopItemWidth();
			ImGui::PopStyleColor();

			ImGui::Separator();
			ImGui::PopItemWidth();
			ImGui::PopFont();

			ImGui::PushFont(ImGui::Flavo::getInstance().fontMap[ImGui::Flavo::FontType::CALIBRI_REGULAR_20]);
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Selected component:");

			if (currentComponentListIndex_ >= 0 && currentComponentListIndex_ < currentEntityComponentTypes_.size()) {
				std::string selectedName = ft_engine::SceneManager::getInstance().getSerializer().getTypeComponentMap().at(currentEntityComponentTypes_[currentComponentListIndex_]);
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), selectedName.c_str());
				renderComponentEditView(entities, selectedName);
			} else {
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "--NOTHING--");
			}
			ImGui::PopFont();
		}
	ImGui::End();
	ImGui::PopFont();
}

void ft_editor::EntityManagementSystem::addComponentView(EntityManager& entities) {
	static int currentSelectedComponent = 0;

	if (!bAddComponentViewActivated_)
		return;

	ImGui::PushFont(ImGui::Flavo::getInstance().fontMap[ImGui::Flavo::FontType::CALIBRI_BOLD_26]);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
	ImGui::Begin("Add Component", &bAddComponentViewActivated_, ImGuiWindowFlags_NoCollapse);

	reflection::Serializer& serialzier = ft_engine::SceneManager::getInstance().getSerializer();
	ImGui::PushFont(ImGui::Flavo::getInstance().fontMap[ImGui::Flavo::FontType::CALIBRI_REGULAR_20]);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	ImGui::PushItemWidth(-1);
	ImGui::ListBox("Available Components", &currentSelectedComponent, &entityAddListGetter, static_cast<void*>(this), serialzier.getComponentTypeMap().size() + 1, 15); //+1 because of "Empty Component" at very beggining
	ImGui::PopItemWidth();
	ImGui::PopFont();
	ImGui::PopStyleColor();

	if (ImGui::Button("Accept")) {
		const char* selectedItemName;
		entityAddListGetter(static_cast<void*>(this), currentSelectedComponent, &selectedItemName);
		if (selectedItemName[0] != '-') {
			const std::string s = selectedItemName;
			const uint32 type = serialzier.getComponentTypeMap().at(s);

			eecs::ComponentBase* newComponent = serialzier.getComponentCreationMap().at(s)();
			entities.addComponent(currentEntityTransform_->assignedTo_.index_, newComponent, type);
			bAddComponentViewActivated_ = false;
		}
	}

	ImGui::SameLine();
	if (ImGui::Button("Back")) {
		bAddComponentViewActivated_ = false;
	}

	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopFont();
}

void ft_editor::EntityManagementSystem::removeComponentView(EntityManager& entites) {
	if (!bRemoveComponentViewActivated_)
		return;

	bRemoveComponentViewActivated_ = false;
	if (currentEntityComponentTypes_.size() <= currentComponentListIndex_)
		return;

	const uint32 componentType = currentEntityComponentTypes_[currentComponentListIndex_];
	if (componentType == static_cast<uint32>(reflection::ComponentEnum::Transform)
		|| componentType == static_cast<uint32>(reflection::ComponentEnum::Metadata))
		return;

	entites.removeComponent(currentEntityTransform_->assignedTo_.index_, componentType);
	currentComponentListIndex_ = currentEntityComponentTypes_.size();
}

void ft_editor::EntityManagementSystem::loadMesh(EntityManager& entities) {
	using namespace eecs;

	const std::experimental::filesystem::path binPath = std::experimental::filesystem::current_path();
	std::string openPath = utils::browseFile(std::experimental::filesystem::current_path().string(), utils::FileType::ALL, true);
	std::experimental::filesystem::current_path(binPath);
	if (openPath.empty())
		return;

	char relativePath[256];
	PathRelativePathTo(relativePath, binPath.string().c_str(),
		FILE_ATTRIBUTE_DIRECTORY, openPath.c_str(),
		FILE_ATTRIBUTE_NORMAL);

	framework::FResourceManager& resourceManager = framework::FResourceManager::getInstance();
	ft_engine::Scene& scene = ft_engine::SceneManager::getInstance().getScene();
	framework::MeshTypeLoadInfo meshInfo;
	ASSERT_CRITICAL(resourceManager.loadMesh(relativePath, OUT meshInfo), "Mesh couldn't be loaded");

	Entity parentEntity = entities.create();
	ComponentHandle<ft_engine::Transform> transform = parentEntity.addComponent<ft_engine::Transform>();
	ComponentHandle<ft_engine::Metadata> metadata = parentEntity.addComponent<ft_engine::Metadata>();
	metadata->name = meshInfo.names[0];
	scene.assignEntityParent(parentEntity, scene.getEntity(scene.getRootTransform()->assignedTo_));

	for (size_t i = 1; i <= meshInfo.childrenCount; i++) {
		Entity childEntity = entities.create();
		ComponentHandle<ft_engine::Transform> transformChild = childEntity.addComponent<ft_engine::Transform>();
		transformChild->setBoundingBox(meshInfo.boundingBoxes[i]);
		ComponentHandle<ft_engine::Metadata> metadataChild = childEntity.addComponent<ft_engine::Metadata>();
		metadataChild->name = meshInfo.names[i];
		scene.assignEntityParent(childEntity, parentEntity);

		if (meshInfo.skinnedVector[i] == framework::MeshSkinnedType::SKINNED) {
			ComponentHandle<ft_render::SkinnedMeshRenderer> skinned = childEntity.addComponent<ft_render::SkinnedMeshRenderer>();
			skinned->reloadMesh(framework::FMeshIdentifier(relativePath, i));
		} else if (meshInfo.skinnedVector[i] == framework::MeshSkinnedType::STATIC) {
			ComponentHandle<ft_render::StaticMeshRenderer> staticMesh = childEntity.addComponent<ft_render::StaticMeshRenderer>();
			staticMesh->reloadMesh(framework::FMeshIdentifier(relativePath, i));
		}
	}
}

void ft_editor::EntityManagementSystem::duplicateEntity(EntityManager& entities, eecs::ComponentHandle<ft_engine::Transform> entityTransform, eecs::ComponentHandle<ft_engine::Transform> parent) {
	if (!entityTransform.isValid())
		return;

	Entity oldEntity = entities.get(entityTransform->assignedTo_);
	Entity newEntity = entities.create();
	std::vector<uint32> componentTypes = entities.getEntityActiveComponentsType(entityTransform->assignedTo_);
	for (uint32 componentType : componentTypes) {
		const nlohmann::json oldJson = entities.getComponentBase(oldEntity.getId(), componentType)->serialize();
		duplicateComponent(entities, componentType, oldJson, newEntity);
	}

	auto newEntityTransform = newEntity.getComponent<ft_engine::Transform>();
	ft_engine::SceneManager::getInstance().getScene().assignTransformParent(newEntityTransform, parent);

	for (auto& it : entityTransform->getChildren()) {
		duplicateEntity(entities, it, newEntityTransform);
	}
}

void ft_editor::EntityManagementSystem::duplicateComponent(EntityManager& entities, uint32 componentType, const nlohmann::json& json, Entity destinationEntity) {
	reflection::Serializer& serializer = ft_engine::SceneManager::getInstance().getSerializer();
	eecs::ComponentBase* newComponent = serializer.getComponentCreationMap().at(serializer.getTypeComponentMap().at(componentType))();
	newComponent->deserialize(json);
	ASSERT_FAIL(entities.addComponent(destinationEntity.getId().index_, newComponent, componentType), "Coludn't duplicate component");
}

void ft_editor::EntityManagementSystem::renderComponentEditView(EntityManager& entities, std::string componentType) {
	static bool bJsonException = false;
	static std::string errorMessageWhat;

	Entity currentEntity = entities.get(currentEntityTransform_->assignedTo_);
	eecs::ComponentBase* selectedComponent = entities.getComponentBase(currentEntityTransform_->assignedTo_, currentEntityComponentTypes_[currentComponentListIndex_]);
	nlohmann::json json = selectedComponent->serialize();
	nlohmann::json savedJson = json;
	ImGui::PushFont(ImGui::Flavo::getInstance().fontMap[ImGui::Flavo::FontType::CALIBRI_REGULAR_16]);
	if (renderComponentRecursively(json, "")) {
		try {
			currentEntityTransform_->translate(Vector3::Zero); //setting bDirty to true
			selectedComponent->deserialize(json);
			bJsonException = false;
			errorMessageWhat = "";
			bComponentChanged = false;
		} catch(std::exception& ex) {
			try {
				selectedComponent->deserialize(savedJson);
				bComponentChanged = true;
				componentFloatValues_.clear();
				componentBoolValues_.clear();
				componentIntValues_.clear();
				errorMessageWhat = ex.what();
			} catch (...) {
				errorMessageWhat = ex.what();
			}
			bJsonException = true;
		}
	} else {
		bComponentChanged = false;
	}
	ImGui::PopFont();

	//error message
	if (bJsonException) {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
		ImGui::PushFont(ImGui::Flavo::getInstance().fontMap[ImGui::Flavo::FontType::CALIBRI_BOLD_26]);
		ImGui::Text("Wrong value; Reverting... ");
		ImGui::Text(errorMessageWhat.c_str());
		ImGui::PopFont();
		ImGui::PopStyleColor();
	}
}

bool ft_editor::EntityManagementSystem::renderComponentRecursively(nlohmann::json& j, std::string idString) {
	bool retFlag = false;
	for (auto it = j.begin(); it != j.end(); ++it) {
		string idKey = idString + it.key();
		if (it->is_structured()) {
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), it.key().c_str());
			ImGui::Indent(16.0f);
			if (renderComponentRecursively(*it, idString + it.key())) retFlag = true;
			ImGui::Unindent(16.0f);
		} else {
			ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), it.key().c_str());
			ImGui::SameLine();
			ImGui::PushItemWidth(-1);
			ImGui::PushStyleColor(ImGuiCol_TextStep, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

			if (it->is_number_float()) {
				if (bComponentChanged)
					componentFloatValues_.insert({ idKey, *it });
				if (ImGui::InputFloat(idKey.c_str(), &componentFloatValues_[idKey], 0.1, 0, 5, ImGuiInputTextFlags_EnterReturnsTrue)) {
					j.at(it.key()) = componentFloatValues_[idKey];
					retFlag = true;
				}
			} else if (it->is_number_integer()) {
				if (bComponentChanged)
					componentIntValues_.insert({ idKey, *it });
				if (ImGui::InputInt(idKey.c_str(), &componentIntValues_[idKey], 1, 0, ImGuiInputTextFlags_EnterReturnsTrue)) {
					j.at(it.key()) = componentIntValues_[idKey];
					retFlag = true;
				}
			} else if (it->is_boolean()) {
				if (bComponentChanged)
					componentBoolValues_.insert({ idKey, *it });
				if (ImGui::CheckboxNoLabel(idKey.c_str(), &componentBoolValues_[idKey])) {
					j.at(it.key()) = componentBoolValues_[idKey];
					retFlag = true;
				}
			} else if (it->is_string()) {
				std::array<char, 256> array{};
				std::string str = *it;
				std::copy(str.begin(), str.end(), array.begin());

				if (it.key().substr(0, 1) == "#") {
					if (ImGui::Button(idKey.c_str(), ImVec2(20, 20))) {
						const std::experimental::filesystem::path binPath = std::experimental::filesystem::current_path();
						std::string openPath = utils::browseFile(std::experimental::filesystem::current_path().string(), utils::FileType::ALL, true);
						std::experimental::filesystem::current_path(binPath);
						
						char relativePath[256];
						PathRelativePathTo(relativePath, binPath.string().c_str(),
							FILE_ATTRIBUTE_DIRECTORY, openPath.c_str(),
							FILE_ATTRIBUTE_NORMAL);

						if (!openPath.empty())
							j.at(it.key()) = relativePath;
						retFlag = true;
					}
					ImGui::SameLine();
					ImGui::Text(str.c_str());
				} else {
					/* Input text has its own label, it's not changable though */
					if (ImGui::InputText(idKey.c_str(), array.data(), 256, ImGuiInputTextFlags_EnterReturnsTrue)) {
						j.at(it.key()) = array.data();
						retFlag = true;
					}
				}
			} else {
				LOG_R("Couldn't get json object type: ", it->type_name());
			}

			ImGui::PopStyleColor();
			ImGui::PopItemWidth();
		}
	}

	return retFlag;
}

bool ft_editor::EntityManagementSystem::entityComponentGetter(void* data, int idx, const char** outText) {
	EntityManagementSystem* ptr = static_cast<EntityManagementSystem*>(data);
	const std::unordered_map<uint32, std::string>& typeComponentMap = ft_engine::SceneManager::getInstance().getSerializer().getTypeComponentMap();
	if (idx >= ptr->currentEntityComponentTypes_.size())
		return false;

	*outText = typeComponentMap.at(ptr->currentEntityComponentTypes_[idx]).c_str();
	return true;
}

bool ft_editor::EntityManagementSystem::entityAddListGetter(void* data, int idx, const char** outText) {
	EntityManagementSystem* ptr = static_cast<EntityManagementSystem*>(data);
	const auto& map = ft_engine::SceneManager::getInstance().getSerializer().getTypeComponentMap();
	if (idx > map.size()) {
		*outText = "---";
		return false;
	}

	if (map.find(idx) == map.end()) {
		*outText = "---Empty Component Slot---";
		return true;
	}

	if (std::find(ptr->currentEntityComponentTypes_.begin(), ptr->currentEntityComponentTypes_.end(), idx) != ptr->currentEntityComponentTypes_.end()) {
		*outText = "---Already added---";
		return true;
	}

	*outText = map.at(idx).c_str();
	return true;
}
