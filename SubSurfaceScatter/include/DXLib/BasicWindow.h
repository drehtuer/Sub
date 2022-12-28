#ifndef SUBSURFACESCATTER_BASICWINDOW_H
#define SUBSURFACESCATTER_BASICWINDOW_H

#include "DXLib/Utilities.h"

#include <Windows.h>

#include <string>

#include "DXLib/MessageHandler.h"

namespace SubSurfaceScatter {

	class DLLE BasicWindow {
	public:
		BasicWindow(HINSTANCE hInstance);
		virtual ~BasicWindow();

		virtual HRESULT init();

		virtual int run();

		int getWidth() const;
		int getHeight() const;
		int getClientWidth() const;
		int getClientHeight() const;
		void virtual setSize(int width, int height);
		void setClientSize(const int width, const int height);
		void applyWindowSize();
		void virtual setWidth(int width);
		void virtual setHeight(int height);
		bool isMaximized() const;
		bool isMinimized() const;
		bool isFullscreen() const;
		void setMaximized(bool maximized);
		void setMinimized(bool minimized);
		void setFullscreen(bool fullscreen);
		virtual void doWork();
		
		HRESULT setTitle(std::wstring title);
		HRESULT appendTitle(std::wstring title);
		std::wstring getTitle() const;
		virtual bool isInitialized() const;

	protected:
		virtual MessageHandler &getMessageHandler();
		
		virtual HRESULT resize();
		HRESULT initWindow();
		virtual void release();


		HINSTANCE m_hInstance;
		HWND m_hWnd;
		
		MessageHandler m_MessageHandler;
		
	private:
		static LRESULT CALLBACK msgProxy(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		int m_windowWidth, m_windowHeight;
		bool m_windowMinimized, m_windowMaximized, m_windowFullscreen, m_windowInitialized;
		std::wstring m_windowTitle;
	};

	static BasicWindow *MainWindow;
};

#endif
