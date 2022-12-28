#include "DXLib/BasicWindow.h"
#include "DXLib/MessageHandler.h"
#include "AntTweakBar.h"
#include "DXLib/ShaderManager.h"
#include "DXLib/Menu.h"
#include "DXLib/Context.h"

using namespace SubSurfaceScatter;

MessageHandler::MessageHandler() :
	m_BasicWindow(NULL),
	m_hWnd(NULL) {

}
	
void MessageHandler::setProperties(BasicWindow *Parent, HWND *hWnd) {
	m_BasicWindow = Parent;
	m_hWnd = hWnd;
}

LRESULT MessageHandler::msgCallback(UINT message, WPARAM wParam, LPARAM lParam) {
	if(TwEventWin(m_hWnd, message, wParam, lParam)) {
		return 0;
	}

	switch(message) {
	case WM_SIZE:
		// resize finished
		
		//if(LOWORD(wParam) == SIZE_MINIMIZED) {
		//	// minimized
		//	m_BasicWindow->setMinimized(true);
		//} else
		//if(LOWORD(wParam) == SIZE_MAXIMIZED) {
		//	// maximized
		//	m_BasicWindow->setMaximized(true);
		//} else
		//if(LOWORD(wParam) == SIZE_RESTORED) {
		//	// restored
		//	if(m_BasicWindow->isMinimized()) {
		//		// from minimized
		//		m_BasicWindow->setMinimized(false);

		//	} else
		//	if(m_BasicWindow->isMaximized()) {
		//		// from maximized
		//		m_BasicWindow->setMaximized(false);

		//	} else
			if(m_windowResizing) {
				// from resizing
				// wait for user to finish resizing

			} else {
				// just call resize
				m_BasicWindow->setSize(LOWORD(lParam), HIWORD(lParam));
			}
		//}
		break;

	case WM_ENTERSIZEMOVE:
		// user start resizing
		m_windowResizing = true;
		//m_BasicWindow->setSize(m_BasicWindow->getWidth(), m_BasicWindow->getHeight());
		break;

	case WM_EXITSIZEMOVE:
		// user end resizing
		m_windowResizing = false;
		m_BasicWindow->setSize(m_BasicWindow->getWidth(), m_BasicWindow->getHeight());
		break;

	case WM_TIMER:
		m_BasicWindow->doWork();
		break;

	case WM_QUIT:
	case WM_DESTROY:
		// user quit
		PostQuitMessage(0);
		break;

	case WM_MENUCHAR:
		// disable beep when entering fullscreen mode with alt-enter
		return MAKELRESULT(0, MNC_CLOSE);

	case WM_GETMINMAXINFO:
		// set minimum window size
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		break;

	case WM_KEYDOWN:
		// key pressed
		keyDown((int)wParam);
		break;

	default:
		return DefWindowProc(*m_hWnd, message, wParam, lParam);
	}

	return 0;
}

void MessageHandler::keyDown(int vkey) {
	Menu *M = NULL;
	Context *CTX = NULL;
	MSG msg;
	std::string textureName;
	switch(vkey) {
	case VK_F5:
		LOGG << "Reloading shaders ...";
		ShaderManager::getInstance()->reloadShaders();
		LOGG << "Reloading complete";
		break;

	case 0x51: // Q
		msg.hwnd = *m_hWnd;
		msg.message = WM_QUIT;
		DispatchMessage(&msg);
		break;

	case VK_F3:
		CTX = (Context*)m_BasicWindow;
		CTX->saveCurrentImage("../../data/screenshots/screenshot");
		break;

	case VK_F2:
		M = Menu::getInstance();
		M->getParams()->SceneSettings.showMenu = !M->getParams()->SceneSettings.showMenu;
		break;

	default:
		break;
	}
}
