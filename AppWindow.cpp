#include <wrl.h>
#include "AppWindow.h"

extern TCHAR szWindowClass[];
extern TCHAR szTitle[];
extern HINSTANCE hInst;
extern int g_nCmdShow;

AppWindow::AppWindow(bool createWebView)
{
	m_window = CreateWindow(
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT,
		1200, 900,
		NULL,
		NULL,
		hInst,
		NULL
	);

	if (!m_window)
	{
		MessageBox(NULL,
			_T("Call to CreateWindow failed!"),
			_T("Windows Desktop Guided Tour"),
			NULL);

		return;
	}

	ShowWindow(m_window,
		g_nCmdShow);
	UpdateWindow(m_window);

	// This is to identify the window from a static context - the static WndProc
	SetWindowLongPtr(m_window, GWLP_USERDATA, (LONG_PTR)this);

	if (createWebView)
	{
		CreateWebView();
	}
}

void AppWindow::CreateWebView()
{
	if (m_webview) return;
	// Locate the browser and set up the environment for WebView
	CreateCoreWebView2EnvironmentWithOptions(nullptr, nullptr, nullptr,
		Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
			[this](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {

				// Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window m_window
				env->CreateCoreWebView2Controller(m_window, Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
					[this](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
						if (controller != nullptr) {
							m_webviewController = controller;
							m_webviewController->get_CoreWebView2(&m_webview);
						}

						// Resize WebView to fit the bounds of the parent window
						RECT bounds;
						GetClientRect(m_window, &bounds);
						m_webviewController->put_Bounds(bounds);

						// Schedule an async task to navigate to Bing
						m_webview->Navigate(L"https://www.bing.com/");

						return S_OK;
					}).Get());
				return S_OK;
			}).Get());
}


// The message handler does 2 things
// 1. Necessary webview resize handling
// 2. Processing keyboard accelerator events like Ctrl+N for new window
bool AppWindow::HandleWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_SIZE:
		if (m_webviewController != nullptr) {
			RECT bounds;
			GetClientRect(hWnd, &bounds);
			m_webviewController->put_Bounds(bounds);
		};
		return true;;
	case WM_KEYDOWN:
		// If bit 30 is set, it means the WM_KEYDOWN message is autorepeated.
		// We want to ignore it in that case.
		if (!(lParam & (1 << 30)))
		{
			if (GetKeyState(VK_CONTROL) < 0)
			{
				switch (UINT(wParam))
				{
				case 'N':
				{
					// Make a new window. If shift is down, make a new window with no webview
					Sleep(5000);
					new AppWindow(GetKeyState(VK_SHIFT) >= 0);
					break;
				};
				}
			}
		}
		return true;;
	case WM_DESTROY:
		PostQuitMessage(0);
		return true;
	}

	return false;
}

LRESULT CALLBACK AppWindow::WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (auto app = (AppWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA))
	{
		if (app->HandleWindowMessage(hWnd, message, wParam, lParam))
		{
			return 0;
		}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}