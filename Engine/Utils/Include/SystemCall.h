#pragma once
#include <string>
#include <windows.h>

namespace utils
{
	enum class FileType
	{
		SCENE,
		ALL
	};

	std::string browseDirectory(const std::string& startPath);
	std::string browseFile(const std::string& startPath, FileType type, bool bOpen);
	void createNewProcess(const std::string& exePath, const std::string& commandLine = std::string());
}

namespace impl_utils
{
	int CALLBACK browseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
}
