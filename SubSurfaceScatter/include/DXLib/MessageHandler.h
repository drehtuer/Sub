#ifndef SUBSURFACESCATTER_MESSAGEHANDLER_H
#define SUBSURFACESCATTER_MESSAGEHANDLER_H

#include "DXLib/Utilities.h"

namespace SubSurfaceScatter {
	// forward declaration
	class BasicWindow;

	class DLLE MessageHandler {
	public:
		MessageHandler();
		virtual void setProperties(BasicWindow *Parent, HWND *hWnd);
		
		virtual LRESULT msgCallback(UINT message, WPARAM wParam, LPARAM lParam);

		virtual void keyDown(int vkey);

	private:
		int m_windowResizing;
		HWND *m_hWnd;
		BasicWindow *m_BasicWindow;
	};
}

#endif
