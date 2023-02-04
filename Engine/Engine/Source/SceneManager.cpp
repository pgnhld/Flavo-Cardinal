#include "SceneManager.h"
#include "FWindow.h"
#include "StringExtension.h"
#include <fstream>
#include "Assertion.h"
#include "Logger.h"
#include "EngineEvent.h"
#include "CoroutineManager.h"
#include "Rendering/RenderSystem.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include <chrono>

ft_engine::SceneManager::SceneManager()
	: serializer_(std::make_unique<reflection::Serializer>()), config_(std::make_unique<Config>()), renderSystem_(std::make_unique<ft_render::RenderSystem>()) {
	scene_ = std::make_unique<Scene>();

	config_->load();
}

ft_engine::SceneManager::~SceneManager() {

}

ft_engine::SceneManager& ft_engine::SceneManager::getInstance() {
	static SceneManager manager;
	return manager;
}

reflection::Serializer& ft_engine::SceneManager::getSerializer() {
	return *serializer_;
}

ft_engine::Scene& ft_engine::SceneManager::getScene() {
	return *scene_;
}

bool ft_engine::SceneManager::load(uint32 sceneIndex) {
	if (ASSERT_FAIL(sceneIndex < config_->sceneIndexToPath.size(), "Such scene index cannot exist in in BuildSettings")) 
		return false;

	const auto it = config_->sceneIndexToPath.find(sceneIndex);
	if (ASSERT_FAIL(it != config_->sceneIndexToPath.end(), "There is no such scene index in BuildSettings"))
		return false;

	bNeedAcceptOnNextSceneLoad_ = true; // sceneIndex != 0; //main menu loading does not need user input to end
	nextSceneToLoad_ = it->second;
	currentlyLoadedIndex_ = sceneIndex;
	return true;
}

void ft_engine::SceneManager::load(const std::string& scenePath) {
	nextSceneToLoad_ = scenePath;
	for (auto& it : config_->sceneIndexToPath) {
		if (it.second == scenePath) {
			currentlyLoadedIndex_ = it.first;
			bNeedAcceptOnNextSceneLoad_ = true; // currentlyLoadedIndex_ != 0;
			break;
		}
	}
}

bool ft_engine::SceneManager::update() {
	renderSystem_->update(
		scene_->getEntityManager(),
		framework::FTime::deltaTime
	);

	if (nextSceneToLoad_.empty())
		return false;

	framework::FTime::bSceneStarted = false;
	scene_.reset();
	CoroutineManager::stopAllCoroutines();

	scene_ = std::make_unique<Scene>();
	scene_->scenePath_ = nextSceneToLoad_;
	for (auto& it : config_->sceneIndexToPath) {
		if (it.second == nextSceneToLoad_) {
			scene_->sceneIndex_ = it.first;
			break;
		}
	}

	serializer_->initRootComponent(scene_->rootEntity_);

	loadingScreen_.startLoading(currentlyLoadedIndex_, scene_->sceneIndex_ == 0);
	EventPostSceneCreated* preEventPtr = new EventPostSceneCreated();
	preEventPtr->filePath = nextSceneToLoad_;
	invoke<EventPostSceneCreated>(preEventPtr);

	std::vector<std::pair<uint64, uint64>> parentChildAssignments;

	int64 lastEntityId = -1;
	std::ifstream sceneFile;
	sceneFile.open(nextSceneToLoad_);

	//get char count
	sceneFile.seekg(0, std::ifstream::end);
	const uint32 charsInFile = sceneFile.tellg();
	sceneFile.seekg(std::ifstream::beg);

	std::string currentLine;
	auto lastTime = std::chrono::high_resolution_clock::now();
	while (getline(sceneFile, currentLine)) {
		const uint32 currentCharIndex = sceneFile.tellg();

		ImGui_ImplDX11_NewFrame();
		if (!framework::FWindow::getInstance().isEditorWindow()) {
			loadingScreen_.loadingThreadLoop(static_cast<float>(currentCharIndex) / static_cast<float>(charsInFile));
		}
		EntityManager temporaryEmptyManager;
		renderSystem_->update(
			temporaryEmptyManager,
			framework::FTime::deltaTime
		);

		const std::chrono::time_point<std::chrono::high_resolution_clock> newTime = std::chrono::high_resolution_clock::now();
		const std::chrono::duration<double> delta = newTime - lastTime;
		framework::FTime::deltaTime = delta.count();
		lastTime = newTime;

		if (isLoadedSceneLineSkippable(REF currentLine))
			continue;

		const size_t firstSpace = currentLine.find_first_of(" \t");
		const std::string lineType = currentLine.substr(0, firstSpace);

		if (lineType == "#SYS") {
			const size_t secondSpace = currentLine.find_first_of(" \t", firstSpace + 1);
			const std::string classType = currentLine.substr(firstSpace + 1, secondSpace - firstSpace - 1);
			const std::string paramString = currentLine.substr(secondSpace + 1);
			const double period = std::stod(paramString);

			EventSystemLoaded* eventPtr = new EventSystemLoaded();
			eventPtr->name = classType;
			eventPtr->period = period;
			invoke<EventSystemLoaded>(eventPtr);

			if (framework::FWindow::getInstance().isEditorWindow())
				continue;
			createSerializedSystem(classType, period);
		} else if (lineType == "#ENT") {
			const size_t secondSpace = currentLine.find_first_of(" \t", firstSpace + 1);
			const std::string entityId = currentLine.substr(firstSpace + 1, secondSpace - firstSpace - 1);
			const std::string entityParent = currentLine.substr(secondSpace + 1);
			lastEntityId = std::stoll(entityId);
			int64 entityParentId = std::stoll(entityParent);
			scene_->entityManager_->create(lastEntityId);
			parentChildAssignments.emplace_back(lastEntityId, entityParentId);
		} else if (lineType == "#COMP") {
			if (ASSERT_FAIL(lastEntityId != -1, "Component serialized without Entity"))
				continue;

			const size_t secondSpace = currentLine.find_first_of(" \t", firstSpace + 1);
			const std::string classType = currentLine.substr(firstSpace + 1, secondSpace - firstSpace - 1);
			const std::string paramString = currentLine.substr(secondSpace + 1);
			createSerializedComponent(classType, paramString, lastEntityId);
		} else {
			LOG_W(format("Incorrect line in scene file; ", currentLine));
			break;
		}
	}

	sceneFile.close();

	for (const auto& it : parentChildAssignments) {
		scene_->assignEntityParent(it.first, it.second);
	}

	EventPostSceneLoaded* eventPtr = new EventPostSceneLoaded();
	eventPtr->filePath = nextSceneToLoad_;
	invoke<EventPostSceneLoaded>(eventPtr);
	
	bool bStartGame = framework::FWindow::getInstance().isEditorWindow() || !bNeedAcceptOnNextSceneLoad_;
	while (!bStartGame) {
		framework::FWindow::getInstance().processMessage();
		if (framework::FWindow::getInstance().bQuitGame)
			break;

		const std::chrono::time_point<std::chrono::high_resolution_clock> newTime = std::chrono::high_resolution_clock::now();
		const std::chrono::duration<double> delta = newTime - lastTime;
		framework::FTime::deltaTime = delta.count();
		lastTime = newTime;

		ImGui_ImplDX11_NewFrame();
		bStartGame = loadingScreen_.finishLoading();
		renderSystem_->update(
			scene_->getEntityManager(),
			framework::FTime::deltaTime
		);
	}

	framework::FTime::bSceneStarted = true;
	nextSceneToLoad_ = std::string();
	return true;
}

bool ft_engine::SceneManager::isLoadedSceneLineSkippable(std::string& currentLine) const {
	if (currentLine.empty())
		return true;

	const auto strBegin = currentLine.find_first_not_of(" \t");
	if (strBegin == std::string::npos)
		return true;

	const auto strEnd = currentLine.find_last_not_of(" \t");
	const auto strRange = strEnd - strBegin + 1;

	currentLine = currentLine.substr(strBegin, strRange);
	if (currentLine.empty())
		return true;

	if (currentLine[0] != '#')
		return true;

	return false;
}

void ft_engine::SceneManager::createSerializedSystem(const std::string& systemString, double period) const {
	const std::unordered_map<std::string, CreateObjectOfTypeFunc>& systemMap = serializer_->getSystemCreationMap();
	eecs::SystemBase* system = systemMap.at(systemString)();
	scene_->systemManager_->add(system, period);
}

void ft_engine::SceneManager::createSerializedComponent(const std::string& componentString, const std::string& paramString, uint64 lastEntityId) const {
	const std::unordered_map<std::string, CreateComponentOfTypeFunc>& componentMap = serializer_->getComponentCreationMap();
	eecs::ComponentBase* component = componentMap.at(componentString)();
	const nlohmann::json json = nlohmann::json::parse(paramString);
	try {
		component->deserialize(json);
	} catch (std::exception& ex) {
		LOG_R("Component [", componentString, "] cannot be deserialized properly\n", ex.what());
	}

	const uint64 typeIndex = serializer_->getComponentTypeMap().at(componentString);
	ASSERT_FAIL(scene_->entityManager_->addComponent(lastEntityId, component, typeIndex),
		format("Couldn't add component... entity=", lastEntityId, ", type=", typeIndex));
}

void ft_engine::SceneManager::Config::load() {
	std::ifstream inFile("../Config/BuildSettings.json");
	nlohmann::json inJson;
	inFile >> inJson;
	from_json(inJson, *this);
}

void ft_engine::SceneManager::Config::save() const {
	std::ofstream outFile("../Config/BuildSettings.json");
	nlohmann::json outJson;
	to_json(outJson, *this);
	outFile << std::setw(4) << outJson << std::endl;
}

void ft_engine::to_json(nlohmann::json& json, const SceneManager::Config& config) {
	//not working too well with unordered_map

	//json = {
	//	{ "sceneIndexToPath", config.sceneIndexToPath }
	//};
}

void ft_engine::from_json(const nlohmann::json& json, SceneManager::Config& config) {
	std::unordered_map<std::string, std::string> map = json.at("sceneIndexToPath").get<std::unordered_map<std::string, std::string>>();
	for (auto& it : map) {
		config.sceneIndexToPath.emplace(std::stoi(it.first), it.second);
	}
}
