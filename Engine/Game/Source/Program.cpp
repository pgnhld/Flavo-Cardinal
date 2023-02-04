#include <windows.h>
#include <Global.h>
#include "Core.h"
#include "FWindow.h"

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	ft_game::Core core(hInstance, nCmdShow);
	while (core.loop());

	// Free resources etc.
	core.cleanup();

	return 0;
}
