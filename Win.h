#pragma once
#include <Windows.h>

class Win
{
public:
	// ウィンドウサイズ
	static const int window_width = 1280;  // 横幅
	static const int window_height = 720;  // 縦幅

	WNDCLASSEX windowClass{}; // ウィンドウクラスの設定

	HWND hwnd = nullptr;	// ウィンドウハンドル
	
	MSG msg{};  // メッセージ


	//関数
	static LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	HWND GetHwnd() { return hwnd; }

	HINSTANCE GetWindowHandle() { return windowClass.hInstance; }// ウィンドウハンドル

	LPCWSTR GetWindowProc() { return windowClass.lpszClassName; }// ウィンドウプロシージャを設定

	MSG GetMsg() { return msg; } // メッセージ

	LPMSG GetLPMsg() { return &msg; } // メッセージ

	void WinCreate();//WindowAPI初期化

	void WinFinalize();

	bool ProcessMessage();

};

