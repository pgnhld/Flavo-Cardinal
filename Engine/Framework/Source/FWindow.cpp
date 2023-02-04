#include "FWindow.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "FInput.h"
#include <filesystem>
#include "Assertion.h"
#include <fstream>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
framework::FWindow* framework::FWindow::instance_ = nullptr;

framework::FWindow& framework::FWindow::getInstance() {
	return *instance_;
}

framework::FWindow::FWindow(HINSTANCE hInstance, int nCmdShow, bool bEditor)
	: config_(std::make_unique<Config>())
	, window_(nullptr)
	, bEditor_(bEditor) {
	ASSERT_CRITICAL(instance_ == nullptr, "More than one instance of FWindow!");
	instance_ = this;

	config_->load();
	initWindow(hInstance, nCmdShow);
	FInput::getMouse().setWindow(window_);
}

void framework::FWindow::processMessage() {
	MSG msg;
	while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
		// Special case for WM_QUIT
		if (msg.message == WM_QUIT)
			bQuitGame = true;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (msg.message == WM_QUIT) {
		bQuitGame = true;
	}
}

HWND framework::FWindow::getWindow() const {
	return window_;
}

bool framework::FWindow::isEditorWindow() const {
	return bEditor_;
}

bool framework::FWindow::isFullscreen() const {
	return config_->bFullscreen;
}

bool framework::FWindow::isVsyncEnabled() const {
	return config_->bVsync;
}

int32 framework::FWindow::getWindowHeight() const {
	return config_->height;
}

int32 framework::FWindow::getWindowWidth() const {
	return config_->width;
}

int32 framework::FWindow::getRenderingWidth() const {
	return renderingWidth_;
}

int32 framework::FWindow::getRenderingHeight() const {
	return renderingHeight_;
}

void framework::FWindow::showWindow(bool show) {
	int nCmdShow = show ? SW_SHOW : SW_HIDE;

	ShowWindow(window_, nCmdShow);
	UpdateWindow(window_);
}

LRESULT framework::FWindow::windowEventProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wParam, lParam))
		return true;

	switch (msg) {
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_ACTIVATEAPP:
		Mouse::processMessage(msg, wParam, lParam);
		Keyboard::processMessage(msg, wParam, lParam);
		break;
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
	case WM_MOUSEHOVER:
		Mouse::processMessage(msg, wParam, lParam);
		break;
	/*case WM_SYSKEYDOWN:*/
	/*case WM_SYSKEYUP:*/
	case WM_KEYDOWN:
	case WM_KEYUP:
		Keyboard::processMessage(msg, wParam, lParam);
		break;

	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	} //switch

	return 0;
}

void framework::FWindow::initWindow(HINSTANCE hInstance, int nCmdShow) {
	WNDCLASSEX wc;
	const char* g_szClassName = "FlavoWindowClass";

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_DBLCLKS;	// detect double clicks
	wc.lpfnWndProc = windowEventProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = g_szClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	ASSERT_CRITICAL(RegisterClassEx(&wc), "Couldn't register window");

	DWORD dwStyle = 0;
	DWORD dwStyleEx = 0;	// exStyle can be useful later

	// Requested size for rendering
	const int sizeScreenX = ::GetSystemMetrics(SM_CXSCREEN);
	const int sizeScreenY = ::GetSystemMetrics(SM_CYSCREEN);
	int requestedSizeX = config_->width;
	int requestedSizeY = config_->height;

	// For fullscreen we want resolution of primary monitor.
	// TODO: Support for many monitors? (MSDN: GetMonitorInfo)
	if (config_->bFullscreen) {
		requestedSizeX = sizeScreenX;
		requestedSizeY = sizeScreenY;
	}

	// Assign proper styles
	if (config_->bFullscreen) {
		dwStyle = WS_POPUP;
		dwStyleEx = WS_EX_APPWINDOW;
	} else {
		dwStyle = WS_OVERLAPPEDWINDOW;
		//dwStyleEx = WS_EX_TOPMOST; //Extremely annoying when debugging

		if (!config_->bAllowResize) {
			dwStyle &= ~WS_THICKFRAME;
			dwStyle &= ~WS_MAXIMIZEBOX;
		}

		if (!config_->bTitleBar) {
			dwStyle = WS_POPUP;
		}
	}

	// When e.g. 1280x720 is requested rendering size, size of whole window
	// will be a bit larger - we need to handle that.
	RECT windowRect;
	SetRect(&windowRect, 0, 0, requestedSizeX, requestedSizeY);
	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwStyleEx);

	// Centered position
	const int sizeWindowX = windowRect.right - windowRect.left;
	const int sizeWindowY = windowRect.bottom - windowRect.top;

	const int positionX = (bEditor_) ? 0 : /*sizeScreenX / 2 - sizeWindowX / 2*/ 0;
	const int positionY = (bEditor_) ? 0 : /*sizeScreenY / 2 - sizeWindowY / 2*/ 0;

	window_ = CreateWindowEx(
		dwStyleEx,
		g_szClassName,
		"Flavo Babilon",
		dwStyle,
		positionX, positionY,
		sizeWindowX, sizeWindowY,
		HWND_DESKTOP,
		NULL,
		hInstance,
		NULL
	);

	// Set proper rendering resolution.
	// (It doesn't have to be the same as window size)
	if (config_->bFullscreen) {
		renderingWidth_ = sizeWindowX;
		renderingHeight_ = sizeWindowY;
	} else {
		renderingWidth_ = config_->width;
		renderingHeight_ = config_->height;
	}


	ASSERT_CRITICAL(window_ != nullptr, "Window creation fail");
}

void framework::FWindow::Config::load() {
	const char* configPath = (getInstance().isEditorWindow()) ? "../Config/Editor/WindowSettings.json" : "../Config/WindowSettings.json";
	std::ifstream inFile(configPath);
	nlohmann::json inJson;
	inFile >> inJson;
	from_json(inJson, *this);
}

void framework::FWindow::Config::save() const {
	const char* configPath = (getInstance().isEditorWindow()) ? "../Config/Editor/WindowSettings.json" : "../Config/WindowSettings.json";
	std::ofstream outFile(configPath);
	nlohmann::json outJson;
	to_json(outJson, *this);
	outFile << std::setw(4) << outJson << std::endl;
}

void framework::to_json(nlohmann::json& json, const FWindow::Config& config) {
	json = nlohmann::json{
		{ "width", config.width },
		{ "height", config.height },
		{ "bFullscreen", config.bFullscreen },
		{ "bAllowResize", config.bAllowResize },
		{ "bVsync", config.bVsync },
		{ "bTitleBar", config.bTitleBar }
	};
}

void framework::from_json(const nlohmann::json& json, FWindow::Config& config) {
	config.width = json.at("width").get<int32>();
	config.height = json.at("height").get<int32>();
	config.bFullscreen = json.at("bFullscreen").get<bool>();
	config.bAllowResize = json.at("bAllowResize").get<bool>();
	config.bVsync = json.at("bVsync").get<bool>();
	config.bTitleBar = json.at("bTitleBar").get<bool>();
}
