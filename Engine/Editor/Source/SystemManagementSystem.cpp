#include "SystemManagementSystem.h"
#include <utility>
#include "SceneManager.h"
#include "EditorEvent.h"
#include "FWindow.h"
#include "imgui.h"
#include "ImGuiExtension.h"
#include <fstream>

ft_editor::SystemManagementSystem::SystemManagementSystem() {
	subscribe<EventSystemLoaded>(this, &SystemManagementSystem::onSystemLoaded);
	subscribe<EventSerializeSystem>(this, &SystemManagementSystem::onSerializeSystem);
	subscribe<EventPostSceneLoaded>(this, &SystemManagementSystem::onPostSceneLoaded);
	currentSystemItemIndexActive_ = { -1, false };
}

ft_editor::SystemManagementSystem::~SystemManagementSystem() {
	unsubscribe<EventSystemLoaded>();
	unsubscribe<EventSerializeSystem>();
	unsubscribe<EventPostSceneLoaded>();
}

void ft_editor::SystemManagementSystem::update(eecs::EntityManager& entities, double deltaTime) {
	int currentActiveItem = -1;
	int currentInactiveItem = -1;

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.87f, 0.87f, 0.6f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_Text] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	ImGui::PushFont(ImGui::Flavo::getInstance().fontMap[ImGui::Flavo::FontType::CALIBRI_BOLD_26]);
	ImGui::Begin("Systems", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_ResizeFromAnySide);
		style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
		ImGui::PushFont(ImGui::Flavo::getInstance().fontMap[ImGui::Flavo::FontType::CALIBRI_REGULAR_20]);
		ImGui::Text("Current item:");
		ImGui::PopFont();

		ImGui::PushFont(ImGui::Flavo::getInstance().fontMap[ImGui::Flavo::FontType::CALIBRI_REGULAR_16]);
		style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		if (currentSystemItemIndexActive_.first != -1) {
			SystemData& selected = (currentSystemItemIndexActive_.second)
				? (activeSystems_[currentSystemItemIndexActive_.first])
				: (inactiveSystems_[currentSystemItemIndexActive_.first]);
			ImGui::Text(("Name " + selected.name).c_str());
			int order = (currentSystemItemIndexActive_.second) ? currentSystemItemIndexActive_.first : -1;
			ImGui::PushStyleColor(ImGuiCol_TextStep, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
			if (ImGui::InputInt("Order ", &order, 1, 0))
				changeSystemOrder(order);
			ImGui::PopStyleColor();
			double period = selected.period;
			if (ImGui::InputDouble("Frequency", &period)) {
				period = std::max(0.0, period);
				selected.period = period;
			}
		} else {
			ImGui::Text("Name: --Not selected--");
		}
		ImGui::PopFont();

		style.Colors[ImGuiCol_Separator] = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
		ImGui::Separator();

		ImGui::PushFont(ImGui::Flavo::getInstance().fontMap[ImGui::Flavo::FontType::CALIBRI_REGULAR_20]);
		ImGui::BeginChild("Selection lists");
			style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
			ImGui::Text("Active");

			style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			ImGui::PushFont(ImGui::Flavo::getInstance().fontMap[ImGui::Flavo::FontType::CALIBRI_REGULAR_16]);
			ImGui::PushItemWidth(-1);
			ImGui::ListBox("Active", 
				(currentSystemItemIndexActive_.second) ? &currentSystemItemIndexActive_.first : &currentActiveItem,
				&SystemManagementSystem::systemListboxGetActive,
				static_cast<void*>(this), activeSystems_.size());
			ImGui::PopItemWidth();
			ImGui::PopFont();

			style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
			ImGui::Text("Inactive");

			style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			ImGui::PushFont(ImGui::Flavo::getInstance().fontMap[ImGui::Flavo::FontType::CALIBRI_REGULAR_16]);
			ImGui::PushItemWidth(-1);
			ImGui::ListBox("Inactive", 
				(!currentSystemItemIndexActive_.second) ? &currentSystemItemIndexActive_.first : &currentInactiveItem,
				&SystemManagementSystem::systemListboxGetInactive, 
				static_cast<void*>(this), inactiveSystems_.size());
			ImGui::PopItemWidth();
			ImGui::PopFont();
		ImGui::EndChild();
		ImGui::PopFont();
	ImGui::End();
	ImGui::PopFont();
	
	if (currentActiveItem != -1) {
		currentSystemItemIndexActive_ = { currentActiveItem, true };
	} else if (currentInactiveItem != -1) {
		currentSystemItemIndexActive_ = { currentInactiveItem, false };
	}
}

void ft_editor::SystemManagementSystem::onSystemLoaded(const EventSystemLoaded& event) {
	SystemData data;
	data.name = event.name;
	data.period = event.period;
	activeSystems_.push_back(data);
}

void ft_editor::SystemManagementSystem::onSerializeSystem(const EventSerializeSystem& event) {
	std::ofstream fileStream;
	fileStream.open(event.filePath, std::ios_base::out | std::ios_base::app);
	for (const auto& it : activeSystems_)
		fileStream << "#SYS " << it.name << " " << it.period << "\n";
	fileStream.close();
}

void ft_editor::SystemManagementSystem::onPostSceneLoaded(const EventPostSceneLoaded& event) {
	auto systemMap = ft_engine::SceneManager::getInstance().getSerializer().getSystemCreationMap();
	for (const auto& it : systemMap) {
		if (it.first == "ft_render::RenderSystem")
			continue; //we should not be able to add or remove RenderSystem from scene systems

		const std::string systemName = it.first;
		bool bActive = false;
		for (const auto& jt : activeSystems_) {
			if (systemName == jt.name) {
				bActive = true;
				break;
			}
		}

		if (!bActive) {
			SystemData data; data.name = systemName; data.period = 0.0;
			inactiveSystems_.push_back(data);
		}
	}
}

void ft_editor::SystemManagementSystem::changeSystemOrder(int order) {
	if (currentSystemItemIndexActive_.second) {
		if (order < 0) {
			std::vector<SystemData> movedSystems(activeSystems_.begin() + currentSystemItemIndexActive_.first + 1, activeSystems_.end());
			activeSystems_.erase(activeSystems_.begin() + currentSystemItemIndexActive_.first + 1, activeSystems_.end());

			if (activeSystems_.size() > 1)
				std::swap(activeSystems_[currentSystemItemIndexActive_.first], activeSystems_[activeSystems_.size() - 1]);
			inactiveSystems_.push_back(activeSystems_.back());
			activeSystems_.erase(activeSystems_.end() - 1);
			activeSystems_.insert(activeSystems_.end(), movedSystems.begin(), movedSystems.end());

			currentSystemItemIndexActive_.first = -1;
			currentSystemItemIndexActive_.second = false;

		} else if (order < static_cast<int>(activeSystems_.size())) {
			std::swap(activeSystems_[order], activeSystems_[currentSystemItemIndexActive_.first]);
			currentSystemItemIndexActive_.first = order;
		}
	} else {
		if (order > -1 && order <= activeSystems_.size()) {
			if (inactiveSystems_.size() > 1)
				std::swap(inactiveSystems_[currentSystemItemIndexActive_.first], inactiveSystems_[inactiveSystems_.size() - 1]);
			std::vector<SystemData> movedSystems(activeSystems_.begin() + order, activeSystems_.end());
			activeSystems_.erase(activeSystems_.begin() + order, activeSystems_.end());
			activeSystems_.push_back(inactiveSystems_[inactiveSystems_.size() - 1]);
			activeSystems_.insert(activeSystems_.end(), movedSystems.begin(), movedSystems.end());
			inactiveSystems_.erase(inactiveSystems_.end() - 1);
			currentSystemItemIndexActive_.first = order;
			currentSystemItemIndexActive_.second = true;
		}
	}
}

bool ft_editor::SystemManagementSystem::systemListboxGetActive(void* data, int idx, const char** outText) {
	const std::vector<SystemData>& sys = static_cast<SystemManagementSystem*>(data)->activeSystems_;
	if (idx >= sys.size())
		return false;

	*outText = sys[idx].name.c_str();
	return true;
}

bool ft_editor::SystemManagementSystem::systemListboxGetInactive(void* data, int idx, const char** outText) {
	const std::vector<SystemData>& sys = static_cast<SystemManagementSystem*>(data)->inactiveSystems_;
	if (idx >= sys.size())
		return false;

	*outText = sys[idx].name.c_str();
	return true;
}
