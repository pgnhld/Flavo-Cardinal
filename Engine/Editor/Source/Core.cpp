#include "Core.h"

#include "SceneManager.h"
#include "FWindow.h"
#include "FRendererD3D11.h"
#include "Network/NetworkManager.h"
#include "SystemManagementSystem.h"
#include "MainViewSystem.h"
#include "FTime.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "ImGuiExtension.h"
#include "FAudio.h"
#include "EntityManagementSystem.h"
#include "Logger.h"
#include "Rendering/RenderSystem.h"
#include "Metadata.h"
#include "Rendering/Camera.h"
#include "Physics/TransformSystem.h"
#include "CoroutineManager.h"
#include "FResourceManager.h"
#include "FlavoRootsGame/Player.h"

ft_editor::Core::Core(HINSTANCE hInstance, int nCmdShow) :
bToExit_(false), lastLoopTime_(std::chrono::high_resolution_clock::now()), fixedUpdateTimer_(0.0) {
	LOG_I("Started editor...");
	subscribe<EventPostSceneCreated>(this, &Core::onPostSceneCreated);
	subscribe<EventPostSceneLoaded>(this, &Core::onPostSceneLoaded);

	framework::FWindow* window = new framework::FWindow(hInstance, nCmdShow, true);
	ft_engine::NetworkManager& networkManager = ft_engine::NetworkManager::getInstance();
	framework::FRendererD3D11* rendererD3D11 = new framework::FRendererD3D11(window->getWindow(), window->getWindowWidth(), window->getWindowHeight(), window->isFullscreen());
	window->showWindow(true);

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(rendererD3D11->getRendererWidth(), rendererD3D11->getRendererHeight());
	io.IniFilename = "../Config/Editor/ImguiEditor.ini";
	ImGui_ImplDX11_Init(framework::FWindow::getInstance().getWindow(), framework::FRendererD3D11::getInstance().getD3D11Device(), framework::FRendererD3D11::getInstance().getD3D11DeviceContext());
	ImGui::Flavo::getInstance().init(Vector2(window->getWindowWidth(), window->getWindowHeight()));
	ImGui::StyleColorsDark();

	ft_engine::SceneManager::getInstance().load("../Data/Scene/DemoScene.fscene");
	ImGui_ImplDX11_NewFrame();
	ft_engine::SceneManager::getInstance().update();
}

ft_editor::Core::~Core() {
	unsubscribe<EventPostSceneCreated>();
	unsubscribe<EventPostSceneLoaded>();

	ImGui_ImplDX11_Shutdown();
	ImGui::DestroyContext();

	framework::FResourceManager::getInstance().releaseResources();
	framework::FRendererD3D11::getInstance().cleanup();
}

bool ft_editor::Core::loop() {
	framework::FWindow::getInstance().processMessage();
	if (framework::FWindow::getInstance().bQuitGame) {
		return false;
	}

	//calculate time deltas
	const std::chrono::time_point<std::chrono::high_resolution_clock> newTime = std::chrono::high_resolution_clock::now();
	const std::chrono::duration<float> delta = newTime - lastLoopTime_;
	const float deltaTime = delta.count();
	framework::FTime::deltaTime = deltaTime;
	framework::FTime::timeSinceStart += deltaTime;
	lastLoopTime_ = newTime;

	//UI and audio
	ImGui_ImplDX11_NewFrame();
	framework::FAudio::getInstance().update();

	//fixed updates
	ft_engine::Scene& scene = ft_engine::SceneManager::getInstance().getScene();
	fixedUpdateTimer_ += deltaTime;
	uint32 fixedThisFrame = 0;
	while (fixedUpdateTimer_ >= framework::FTime::fixedDeltaTime) {
		if (fixedThisFrame > FIXED_UPDATE_PER_FRAME_CAP) {
			LOG_W(format("fixedUpdate() invoked too many times this frame [Current cap = ", FIXED_UPDATE_PER_FRAME_CAP, "]"));
			break;
		}
		scene.fixedUpdate(framework::FTime::fixedDeltaTime);
		fixedUpdateTimer_ -= framework::FTime::fixedDeltaTime;
		fixedThisFrame += 1;
	}

	//updates
	framework::FTime::deltaTime = deltaTime;
	scene.update(deltaTime);
	lastLoopTime_ = newTime;

	//update coroutines
	ft_engine::CoroutineManager::getInstance().update();

	//check for scene change requests
	ft_engine::SceneManager::getInstance().update();
	return true;
}

void ft_editor::Core::onPostSceneCreated(const EventPostSceneCreated& event) {
	ft_engine::SceneManager& manager = ft_engine::SceneManager::getInstance();
	ft_engine::Scene& scene = manager.getScene();
	scene.addSystem(new ft_engine::TransformSystem(), 0.0);
	scene.addSystem(new SystemManagementSystem(), 0.0);
	scene.addSystem(new MainViewSystem(), 0.0);
	scene.addSystem(new EntityManagementSystem(), 0.0);
}

void ft_editor::Core::onPostSceneLoaded(const EventPostSceneLoaded& event) {
	ft_engine::SceneManager& manager = ft_engine::SceneManager::getInstance();
	ft_engine::Scene& scene = manager.getScene();
	std::vector<Entity> cameraEntities = scene.getEntityManager().getEntitiesWithComponents<ft_render::Camera>();
	for (auto& e : cameraEntities)
		e.getComponent<ft_render::Camera>()->bEnabled = false;

	Entity cameraEntity = scene.instantiate();
	eecs::ComponentHandle<ft_engine::Transform> transform = cameraEntity.addComponent<ft_engine::Transform>();
	scene.assignTransformParent(transform, scene.getRootTransform());
	transform->translate(Vector3(1.0f, 20.0f, 3.0f));
	transform->rotate(Quaternion::CreateFromEuler(Vector3(DEG2RAD(85.0f), 0.0f, 0.0f)));
	eecs::ComponentHandle<ft_engine::Metadata> metadata = cameraEntity.addComponent<ft_engine::Metadata>();
	metadata->name = "### EditorCamera";
	cameraEntity.addComponent<ft_render::Camera>();
	cameraEntity.addComponent<ft_engine::Player>();
}

