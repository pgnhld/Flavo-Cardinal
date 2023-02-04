#include "MainViewSystem.h"
#include "FWindow.h"
#include "SystemCall.h"
#include <experimental/filesystem>
#include "SceneManager.h"
#include "imgui.h"
#include <fstream>
#include "FAudio.h"
#include "ImGuiExtension.h"
#include "Assertion.h"
#include "Logger.h"
#include "Metadata.h"
#include <Shlwapi.h>

using namespace ft_engine;
ft_editor::MainViewSystem::MainViewSystem() {

}

ft_editor::MainViewSystem::~MainViewSystem() {

}

void ft_editor::MainViewSystem::update(eecs::EntityManager& entities, double deltaTime) {
	const int32 mainWindowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	ImGui::PushFont(ImGui::Flavo::getInstance().fontMap[ImGui::Flavo::FontType::CALIBRI_REGULAR_20]);
	ImGui::Begin("Menu Bar", nullptr, mainWindowFlags);
	if (ImGui::BeginMenuBar()) {
		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_Button] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 0.2f, 0.0f, 1.0f);
		if (ImGui::Button("Play")) play();

		style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		if (ImGui::Button("Open")) open();
		if (ImGui::Button("Save")) save();
		if (ImGui::Button("Save As")) saveAs();
		if (ImGui::Button("Refresh")) refresh();

		style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
		if (ImGui::BeginMenu("FLAVO SOUND")) {
			if (ImGui::MenuItem("Dechost")) playMusic(framework::BackgroundMusicType::DECHOST);
			if (ImGui::MenuItem("Papamobile 2138")) playMusic(framework::BackgroundMusicType::PAPA_2138);
			ImGui::EndMenu();
		}

		style.Colors[ImGuiCol_Text] = ImVec4(1.0f, 0.2f, 0.0f, 1.0f);
		if (ImGui::Button("Exit")) exit();

		ImGui::SameLine(0.0f, 35.0f);
		ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.0f, 1.0f), ("Current: " + SceneManager::getInstance().getScene().getScenePath()).c_str());
	}
	ImGui::EndMenuBar();
	ImGui::End();
	ImGui::PopFont();
}

void ft_editor::MainViewSystem::open() const {
	const std::experimental::filesystem::path binPath = std::experimental::filesystem::current_path();
	std::string openPath = utils::browseFile(std::experimental::filesystem::current_path().string(), utils::FileType::SCENE, true);
	std::experimental::filesystem::current_path(binPath);
	if (openPath.empty())
		return;

	char relativePath[256];
	PathRelativePathTo(relativePath, binPath.string().c_str(),
		FILE_ATTRIBUTE_DIRECTORY, openPath.c_str(),
		FILE_ATTRIBUTE_NORMAL);

	ft_engine::SceneManager::getInstance().load(relativePath);
}

void ft_editor::MainViewSystem::save() {
	const std::string intro = std::string() + "* Automatically generated scene file from Flavo Editor\n" +
		"* Usage:\n" +
		"*	#XXX YYY\n" +
		"* where #XXX in :\n" +
		"*	#SYS = Systems only available in game\n" +
		"*	#COMP = Any type of Component\n" +
		"*	#ENT = entity\n" +
		"*\n";

	const std::string& currentScenePath = SceneManager::getInstance().getScene().getScenePath();
	std::ofstream fileStream;
	fileStream.open(currentScenePath, std::ios_base::out | std::ios_base::trunc);
	fileStream << intro;
	fileStream.close();

	EventSerializeSystem* eventPtr = new EventSerializeSystem();
	eventPtr->filePath = currentScenePath;
	invoke<EventSerializeSystem>(eventPtr);

	fileStream.open(currentScenePath, std::ios_base::out | std::ios_base::app);
	fileStream << "\n\n"; //some space after systems
	saveEntityRecursive(SceneManager::getInstance().getScene().getRootTransform().get(), -1, 0, fileStream);
	fileStream.close();
}

void ft_editor::MainViewSystem::saveAs() {
	const std::experimental::filesystem::path binPath = std::experimental::filesystem::current_path();
	std::string openPath = utils::browseFile(std::experimental::filesystem::current_path().string(), utils::FileType::SCENE, false);
	std::experimental::filesystem::current_path(binPath);
	if (openPath.empty())
		return;

	std::string& currentScenePath = ft_engine::SceneManager::getInstance().getScene().getScenePath();
	char relativePath[256];
	PathRelativePathTo(relativePath, binPath.string().c_str(),
		FILE_ATTRIBUTE_DIRECTORY, openPath.c_str(),
		FILE_ATTRIBUTE_NORMAL);

	currentScenePath = std::string(relativePath);
	save();
}

void ft_editor::MainViewSystem::play() const {
	utils::createNewProcess("FlavoBabilon.exe", "#" + SceneManager::getInstance().getScene().getScenePath());
}

void ft_editor::MainViewSystem::refresh() const {
	ft_engine::SceneManager::getInstance().load(ft_engine::SceneManager::getInstance().getScene().getScenePath());
}

void ft_editor::MainViewSystem::exit() const {
	framework::FWindow::getInstance().bQuitGame = true;
}

void ft_editor::MainViewSystem::playMusic(framework::BackgroundMusicType musicType) const {
	framework::FAudio::getInstance().playBackgroundMusic(musicType);
}

void ft_editor::MainViewSystem::saveEntityRecursive(Transform* transform, int level, uint64 parentId, std::ostream& stream) {
	string indent;
	for (int i = 0; i < level; ++i)
		indent += "  ";

	const uint64 entityId = transform->assignedTo_.index_;
	EntityManager& entities = SceneManager::getInstance().getScene().getEntityManager();
	eecs::ComponentHandle<Metadata> metadata = entities.getComponent<Metadata>(transform->assignedTo_);
	if (metadata.isValid() && metadata->name.substr(0, 3) == "###") {
		return;
	}

	if (level != -1) {
		stream << indent << "#ENT " << entityId << " " << parentId << "\n";

		reflection::Serializer& serializer = SceneManager::getInstance().getSerializer();
		for (uint32 i = 0; i < eecs::MAX_COMPONENTS; ++i) {
			eecs::ComponentBase* component = entities.getComponentBase(transform->assignedTo_, i);
			if (component == nullptr) continue;
			stream << indent << "#COMP " << serializer.getTypeComponentMap().at(i) << " " << component->serialize() << "\n";
		}
	}

	for (auto& it : transform->getChildren()) {
		if (ASSERT_FAIL(it.isValid(), format("Invalid transform for id: ", transform->assignedTo_.index_)))
			continue;

		saveEntityRecursive(it.get(), level + 1, entityId, stream);
	}

	stream << "\n";
}
