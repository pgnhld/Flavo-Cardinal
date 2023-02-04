#pragma once

#include "Global.h"
#include <memory>
#include "Json.h"
#include <windows.h>

namespace framework
{
	class FWindow
	{
	public:
		class Config;

		static FWindow& getInstance();
		FWindow(HINSTANCE hInstance, int nCmdShow, bool bEditor);
		void processMessage();
		HWND getWindow() const;

		bool isEditorWindow() const;
		bool isFullscreen() const;
		bool isVsyncEnabled() const;

		int32 getWindowHeight() const;
		int32 getWindowWidth() const;
		
		int32 getRenderingWidth() const;
		int32 getRenderingHeight() const;

		void showWindow(bool show);

		bool bQuitGame = false;

	private:
		static LRESULT CALLBACK windowEventProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		void initWindow(HINSTANCE hInstance, int nCmdShow);

	private:
		static FWindow* instance_;
		std::unique_ptr<Config> config_;
		HWND window_;
		bool bEditor_;

		int32 renderingWidth_;
		int32 renderingHeight_;
	};

	class FWindow::Config
	{
	public:
		Config() = default;
		Config(const Config& another) = delete;
		Config(Config&& another) = delete;
		~Config() = default;

		Config& operator=(const Config& another) = delete;
		Config& operator=(Config&& another) = delete;

		void load();
		void save() const;

	public:
		int32 width;
		int32 height;
		bool bFullscreen;
		bool bAllowResize; //only when bFullscreen = false
		bool bTitleBar;
		bool bVsync;
	};

	void to_json(OUT nlohmann::json& json, const FWindow::Config& config);
	void from_json(const nlohmann::json& json, OUT FWindow::Config& config);
}
