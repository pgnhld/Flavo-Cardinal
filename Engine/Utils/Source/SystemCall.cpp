#include "SystemCall.h"
#include <shlobj.h>

std::string utils::browseDirectory(const std::string& startPath) {
	TCHAR path[MAX_PATH];
	const char* pathParam = startPath.c_str();

	BROWSEINFO bi = { nullptr };
	bi.lpszTitle = ("Browse for folder...");
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lpfn = impl_utils::browseCallbackProc;
	bi.lParam = reinterpret_cast<LPARAM>(pathParam);

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	if (pidl != nullptr) {
		SHGetPathFromIDList(pidl, path);

		IMalloc * imalloc = 0;
		if (SUCCEEDED(SHGetMalloc(&imalloc))) {
			imalloc->Free(pidl);
			imalloc->Release();
		}

		return path;
	}

	return "";
}

int CALLBACK impl_utils::browseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) {
	if (uMsg == BFFM_INITIALIZED) {
		std::string tmp = reinterpret_cast<const char *>(lpData);
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	}

	return 0;
}

std::string utils::browseFile(const std::string& startPath, FileType type, bool bOpen) {
	char filename[MAX_PATH];

	OPENFILENAME ofn;
	ZeroMemory(&filename, sizeof(filename));
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr;
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = "Select file...";
	ofn.Flags = OFN_DONTADDTORECENT;
	if (bOpen) ofn.Flags |= OFN_FILEMUSTEXIST;
	ofn.lpstrInitialDir = startPath.c_str();

	switch (type) {
	case FileType::SCENE:
		ofn.lpstrFilter = "Flavo scenes\0*.fscene\0";
		break;
	case FileType::ALL:
		ofn.lpstrFilter = "All Files\0*.*\0";
		break;
	}

	if (bOpen) {
		if (GetOpenFileNameA(&ofn))
			return std::string(filename);

		return "";
	} else {
		if (GetSaveFileNameA(&ofn))
			return std::string(filename);

		return "";
	}
}

void utils::createNewProcess(const std::string& exePath, const std::string& commandLine) {
	// additional information
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	// set the size of the structures
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	char* cmdLineNonConst = nullptr;
	if (!commandLine.empty()) {
		cmdLineNonConst = new char[commandLine.length() + 1];
		strcpy_s(cmdLineNonConst, commandLine.length() + 1, commandLine.c_str());
	}

	// start the program up
	CreateProcess(
		exePath.c_str(),	// the path
		cmdLineNonConst,	// Command line
		nullptr,			// Process handle not inheritable
		nullptr,			// Thread handle not inheritable
		false,				// Set handle inheritance to FALSE
		0,					// No creation flags
		nullptr,			// Use parent's environment block
		nullptr,			// Use parent's starting directory 
		&si,				// Pointer to STARTUPINFO structure
		&pi					// Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
	);

	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	delete[] cmdLineNonConst;
}
