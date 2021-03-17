#pragma once
#include <windows.h>
#include <stdlib.h>
#include <string>
#include <tchar.h>
#include <wil/com.h>
// include WebView2 header
#include <webview2.h>

class AppWindow
{
	// Pointer to m_webviewController
	wil::com_ptr<ICoreWebView2Controller> m_webviewController;

	// Pointer to WebView window
	wil::com_ptr<ICoreWebView2> m_webview;

	HWND m_window;

public:
	static LRESULT CALLBACK WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	AppWindow(bool createWebView = true);
	bool HandleWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	void CreateWebView();
};
