// compile with: /D_UNICODE /DUNICODE /DWIN32 /D_WINDOWS /c

#include <windows.h>
#include <stdlib.h>
#include <string>
#include <tchar.h>
#include <wrl.h>
#include <wil/com.h>
// include WebView2 header
#include <webview2.h>
#include <WebView2EnvironmentOptions.h>

using namespace Microsoft::WRL;

// Global variables

// The main window class name.
static TCHAR szWindowClass[] = _T("DesktopApp");

// The string that appears in the application's title bar.
static TCHAR szTitle[] = _T("WebView sample");

HINSTANCE hInst;

int g_nCmdShow;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


class AppWindow
{
	// Pointer to m_webviewController
	wil::com_ptr<ICoreWebView2Controller> m_webviewController;

	// Pointer to WebView window
	wil::com_ptr<ICoreWebView2> m_webview;

	HWND m_window;
	bool m_toggleUrl : 1;
	bool m_doSleep : 1;

	LRESULT CALLBACK HandleWindowMessage(HWND, UINT, WPARAM, LPARAM);

public:
	static LRESULT CALLBACK WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	AppWindow(bool createWebView = true);
	bool HandleWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT* result);
	void NavigateRandom();
	void Navigate(std::wstring url);
	void CreateWebView();
};

AppWindow::AppWindow(bool createWebView)
{
	// The parameters to CreateWindow explained:
	// szWindowClass: the name of the application
	// szTitle: the text that appears in the title bar
	// WS_OVERLAPPEDWINDOW: the type of window to create
	// CW_USEDEFAULT, CW_USEDEFAULT: initial position (x, y)
	// 500, 100: initial size (width, length)
	// NULL: the parent of this window
	// NULL: this application does not have a menu bar
	// hInstance: the first parameter from WinMain
	// NULL: not used in this application
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

	// The parameters to ShowWindow explained:
	// hWnd: the value returned from CreateWindow
	// nCmdShow: the fourth parameter from WinMain
	ShowWindow(m_window,
		g_nCmdShow);
	UpdateWindow(m_window);

	//SetWindowPos(m_window, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	//SetForegroundWindow(m_window);

	// this is arbitrary
	m_toggleUrl = false;
	m_doSleep = false;

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
		Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
			[this](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {

				// Create a CoreWebView2Controller and get the associated CoreWebView2 whose parent is the main window m_window
				env->CreateCoreWebView2Controller(m_window, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
					[this](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
						if (controller != nullptr) {
							m_webviewController = controller;
							m_webviewController->get_CoreWebView2(&m_webview);
						}

						// Add a few settings for the webview
						// The demo step is redundant since the values are the default settings
						ICoreWebView2Settings* Settings;
						m_webview->get_Settings(&Settings);
						Settings->put_IsScriptEnabled(TRUE);
						Settings->put_AreDefaultScriptDialogsEnabled(TRUE);
						Settings->put_IsWebMessageEnabled(TRUE);

						// Resize WebView to fit the bounds of the parent window
						RECT bounds;
						GetClientRect(m_window, &bounds);
						m_webviewController->put_Bounds(bounds);

						// Schedule an async task to navigate to Bing
						m_webview->Navigate(L"https://www.bing.com/");

						// Step 4 - Navigation events

						// Step 5 - Scripting

						// Step 6 - Communication between host and web content

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
					if (m_doSleep)
						Sleep(5000);
					new AppWindow(GetKeyState(VK_SHIFT) >= 0);
					break;
				};
				case 'M':
				{
					if (m_doSleep)
						Sleep(5000);
					NavigateRandom();
					break;
				};
				case 'D':
				{
					if (m_doSleep)
						Sleep(5000);
					CreateWebView();
				}
				case'E':
				{
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

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
)
{
	// Store instance handle in our global variable
	hInst = hInstance;

	g_nCmdShow = nCmdShow;

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInst;
	wcex.hIcon = LoadIcon(hInst, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL,
			_T("Call to RegisterClassEx failed!"),
			_T("Windows Desktop Guided Tour"),
			NULL);

		return 1;
	}

	new AppWindow();

	// Main message loop:
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_DESTROY  - post a quit message and return
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	TCHAR greeting[] = _T("Hello, Windows desktop!");
	return AppWindow::WndProcStatic(hWnd, message, wParam, lParam);
}
