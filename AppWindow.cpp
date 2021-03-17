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

	// this is arbitrary
	m_toggleUrl = false;
	m_doSleep = false;

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

void AppWindow::Navigate(std::wstring url)
{
	if (m_webview)
		m_webview->Navigate(url.c_str());
}

void AppWindow::NavigateRandom()
{
	m_toggleUrl = !m_toggleUrl;
	std::wstring url = m_toggleUrl ? L"https://google.com" : L"https://startpage.com";
	Navigate(url.c_str());
}

// The message handler does 2 things
// 1. Necessary webview resize handling
// 2. Processing keyboard accelerator events like Ctrl+N for new window
bool AppWindow::HandleWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT* result)
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
					if (m_doSleep)
						Sleep(5000);
					new AppWindow(GetKeyState(VK_SHIFT) >= 0);
					break;
				};
				case 'M':
				{
					// Navigate the current window to startpage.com or google.com randomly.
					if (m_doSleep)
						Sleep(5000);
					NavigateRandom();
					break;
				};
				case 'D':
				{
					// Create a webview in the current window.
					if (m_doSleep)
						Sleep(5000);
					CreateWebView();
				}
				case'E':
				{
					// Set whether or not to wait for 5 seconds.
					m_doSleep = !m_doSleep;
				}
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
		LRESULT result = 0;
		if (app->HandleWindowMessage(hWnd, message, wParam, lParam, &result))
		{
			return result;
		}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}