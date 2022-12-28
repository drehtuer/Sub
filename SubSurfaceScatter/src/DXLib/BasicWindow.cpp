#include "DXLib/BasicWindow.h"

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 1024

#define REDRAW_TIMER 1

using namespace SubSurfaceScatter;

BasicWindow::BasicWindow(HINSTANCE hInstance)
	: m_hInstance(hInstance),
	  m_hWnd(NULL),
	  m_windowWidth(WINDOW_WIDTH), m_windowHeight(WINDOW_HEIGHT),
	  m_windowMinimized(false), m_windowMaximized(false),
	  m_windowFullscreen(false),
	  m_windowInitialized(false),
	  m_windowTitle(L"SubSurfaceScatter")
{
	// we want to have a specific size for the client area, not the window area
	
	applyWindowSize();
	HRESULT hr = init();
	chkError(hr);
	if(FAILED(hr))
		PostQuitMessage(0);
}

BasicWindow::~BasicWindow() {
	release();
}

void BasicWindow::applyWindowSize() {
	RECT rectWindow, rectClient;
    GetWindowRect(m_hWnd, &rectWindow);
    GetClientRect(m_hWnd, &rectClient);
	POINT diff;
    diff.x = rectWindow.right - rectWindow.left - rectClient.right;
    diff.y = rectWindow.bottom - rectWindow.top - rectClient.bottom;
    MoveWindow(m_hWnd, rectWindow.left, rectWindow.top, m_windowWidth + diff.x, m_windowHeight + diff.y, FALSE);
    std::cout<<"width: "<<m_windowWidth<<", height: "<<m_windowHeight<<std::endl;
}

HRESULT BasicWindow::init() {
	MainWindow = this;
	HRESULT hr = initWindow();
	chkError(hr);
	if(SUCCEEDED(hr))
		m_windowInitialized = true;
	//SetTimer(m_hWnd, REDRAW_TIMER, 10, (TIMERPROC)NULL);
	return hr;
}

HRESULT BasicWindow::initWindow() {
	// register window
	WNDCLASSEX wcx;
	ZeroMemory(&wcx, sizeof(wcx));
	wcx.cbSize = sizeof(WNDCLASSEX);
	wcx.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcx.lpfnWndProc = BasicWindow::msgProxy;
	wcx.cbClsExtra = 0;
	wcx.cbWndExtra = 0;
	wcx.hInstance = m_hInstance;
	wcx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wcx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcx.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wcx.lpszMenuName = NULL; // no menu
	wcx.lpszClassName = L"BasicWindow";
	wcx.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if(!RegisterClassEx(&wcx)) {
		show_error(L"Could not register window class");
		return E_FAIL;
	}
	
	// create window
	m_hWnd = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,
		                    L"BasicWindow",
				            m_windowTitle.c_str(),
				            WS_OVERLAPPEDWINDOW,
				            CW_USEDEFAULT, // x
				            CW_USEDEFAULT, // y
				            m_windowWidth,
				            m_windowHeight,
				            NULL, // no parent
				            NULL, // no menu
				            m_hInstance,
							NULL);
	if(m_hWnd == NULL) {
		show_error(L"Could not create window!");
		return E_FAIL;
	}

	m_MessageHandler.setProperties(this, &m_hWnd);
    
	ShowWindow(m_hWnd, SW_SHOW);
	UpdateWindow(m_hWnd);
	return S_OK;
}

HRESULT BasicWindow::setTitle(std::wstring title) {
	return SetWindowText(m_hWnd, title.c_str());
}

HRESULT BasicWindow::appendTitle(std::wstring appTitle) {
	return setTitle(m_windowTitle + appTitle);
}

std::wstring BasicWindow::getTitle() const {
	LPTSTR windowText = L"";
	GetWindowText(m_hWnd, windowText, sizeof(windowText));
	return std::wstring(windowText);
}

int BasicWindow::run() {
	MSG msg = {0};

	while(msg.message != WM_QUIT) {
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			// wait for timer
            doWork();
		}
	}
	return static_cast<int>(msg.wParam);
}

void BasicWindow::release() {
	if(m_hWnd)
		DestroyWindow(m_hWnd);
	m_hWnd = NULL;
	UnregisterClass(L"BasicWindow", m_hInstance);
}

void BasicWindow::doWork() {
}

HRESULT BasicWindow::resize() {
	return S_OK;
}

int BasicWindow::getWidth() const {
	return m_windowWidth;
}

int BasicWindow::getHeight() const {
	return m_windowHeight;
}

int BasicWindow::getClientWidth() const {
	RECT rect;
	GetClientRect(m_hWnd, &rect);
	return rect.right - rect.left;
}

int BasicWindow::getClientHeight() const {
	RECT rect;
	GetClientRect(m_hWnd, &rect);
	return rect.bottom - rect.top;
}

void BasicWindow::setSize(int width, int height) {
	// resizing is expensive, so call it only if needed
	if(width != m_windowWidth || height != m_windowHeight) {
		m_windowWidth = width;
		m_windowHeight = height;
		applyWindowSize();
        resize();
	}
}

void BasicWindow::setClientSize(const int width, const int height) {
	RECT rect = {0, 0, width, height};
	AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW);
	setSize(rect.right - rect.left, rect.bottom - rect.top);
}

void BasicWindow::setWidth(int width) {
	m_windowWidth = width;
	resize();
}

void BasicWindow::setHeight(int height) {
	m_windowHeight = height;
	resize();
}

bool BasicWindow::isMaximized() const {
	return m_windowMaximized;
}

bool BasicWindow::isMinimized() const {
	return m_windowMinimized;
}

void BasicWindow::setMaximized(bool maximized) {
	m_windowMaximized = maximized;
	resize();
}

void BasicWindow::setMinimized(bool minimized) {
	m_windowMinimized = minimized;
}

bool BasicWindow::isFullscreen() const {
	return m_windowFullscreen;
}

void BasicWindow::setFullscreen(bool fullscreen) {
	m_windowFullscreen = fullscreen;
	resize();
}

MessageHandler &BasicWindow::getMessageHandler() {
	return m_MessageHandler;
}

bool BasicWindow::isInitialized() const {
	return m_windowInitialized;
}

LRESULT CALLBACK BasicWindow::msgProxy(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	if(MainWindow->isInitialized())
		return MainWindow->getMessageHandler().msgCallback(message, wParam, lParam);
	return DefWindowProc(hWnd, message, wParam, lParam);
}



