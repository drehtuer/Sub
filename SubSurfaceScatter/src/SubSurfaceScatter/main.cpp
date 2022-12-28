#include "DXLib/Utilities.h"

#include <Windows.h>

#include "DXLib/Context.h"


using namespace SubSurfaceScatter;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmnLine, int nCmdShow) {
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	freopen("CON", "w", stdout);
	freopen("CON", "w", stderr);
	Context window(hInstance);
	return window.run();
}
