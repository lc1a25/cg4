#include "Input.h"
#define DIRECTINPUT_VERSION 0x0800	//DirectInputのバージョン指定
#include <dinput.h>
#include "Win.h"
#include <wrl.h>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

BYTE Input::keys[ALL_KEYS] = { 0 };
BYTE Input::oldkeys[ALL_KEYS] = { 0 };

void Input::Initialize(Win*win)
{
	this->win = win;
	HRESULT result;
	//DirectInputオブジェクトの生成

	result = DirectInput8Create(
		win->GetWindowHandle(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dinput, nullptr);

	//キーボードデバイスの生成

	result = dinput->CreateDevice(GUID_SysKeyboard, &devkeyboard, NULL);

	//入力データの形式セット
	result = devkeyboard->SetDataFormat(&c_dfDIKeyboard);

	//排他制御レベルのセット
	result = devkeyboard->SetCooperativeLevel(
		win->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	/*//画面が手前にある場合のみ入力を受け付ける　//デバイスをこのアプリだけで占有しない//Windowsキーを無効にする*/
}

void Input::Update()
{
	HRESULT result;
	for (int i = 0; i < ALL_KEYS; ++i) {
		oldkeys[i] = keys[i];
	}
	result = devkeyboard->Acquire();
	result = devkeyboard->GetDeviceState(sizeof(keys), keys);
}




