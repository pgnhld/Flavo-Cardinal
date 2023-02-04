#include <windows.h>
#include <Global.h>
#include "Core.h"
#include "FWindow.h"

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	ft_editor::Core core(hInstance, nCmdShow);

	while (core.loop());

	return 0;
}
