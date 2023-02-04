#include "Core.h"

#include "SceneManager.h"
#include "FWindow.h"
#include "Network/NetworkManager.h"
#include "FResourceManager.h"
#include "FTime.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "Logger.h"
#include "ImGuiExtension.h"
#include "CoroutineManager.h"

ft_game::Core::Core(HINSTANCE hInstance, int nCmdShow) :
	bToExit_(false), lastLoopTime_(std::chrono::high_resolution_clock::now()), fixedUpdateTimer_(0.0) {
	LOG_I("Started engine...");

	//Init project singletons
	window_ = new framework::FWindow(hInstance, nCmdShow, false);
	networkManager_ = new ft_engine::NetworkManager();

	rendererD3D11_ = new framework::FRendererD3D11(window_->getWindow(),
		window_->getRenderingWidth(),
		window_->getRenderingHeight(),
		window_->isFullscreen());

	// Show window after initializing D3D11
	window_->showWindow(true);

	//Init UI
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize = ImVec2(rendererD3D11_->getRendererWidth(), rendererD3D11_->getRendererHeight());
	io.IniFilename = "../Config/Imgui.ini";
	ImGui_ImplDX11_Init(framework::FWindow::getInstance().getWindow(), framework::FRendererD3D11::getInstance().getD3D11Device(), framework::FRendererD3D11::getInstance().getD3D11DeviceContext());
	ImGui::Flavo::getInstance().init(Vector2(window_->getRenderingWidth(), window_->getRenderingHeight()));
	ImGui::StyleColorsDark();

	//When playing game via Editor, commandLine is set to scene path, otherwise it is current .exe location
	std::string commandLine = GetCommandLine();
	if (commandLine.substr(0, 1) == "#") {
		ft_engine::SceneManager::getInstance().load(commandLine.substr(1));
	} else {
		ft_engine::SceneManager::getInstance().load("../Data/Scene/MainMenu.fscene");
	}

	ImGui_ImplDX11_NewFrame();
	ft_engine::SceneManager::getInstance().update();
}

bool ft_game::Core::loop() {
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

	//start UI render
	ImGui_ImplDX11_NewFrame();

	//invoke fixedUpdate()
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

	//calculate FPS
	static double fpsTimer = 0.0;
	static uint32 fpsCounter = 0;
	if (fpsTimer >= 1.0) {
		LOG_C(fpsCounter);
		fpsCounter = 0;
		fpsTimer = 0.0;
	}
	fpsTimer += framework::FTime::deltaTime;
	fpsCounter++;

	//update coroutines
	ft_engine::CoroutineManager::getInstance().update();

	//invoke update()
	scene.update(deltaTime);

	//check if scene should be reloaded + RenderSystem
	if (ft_engine::SceneManager::getInstance().update()) {
		//things that have to be cleaned after scene reload
	}

	return true;
}

void ft_game::Core::cleanup() {
	ImGui_ImplDX11_Shutdown();
	ImGui::DestroyContext();

	framework::FResourceManager::getInstance().releaseResources();

	rendererD3D11_->cleanup();
	delete rendererD3D11_;
	rendererD3D11_ = nullptr;
}
