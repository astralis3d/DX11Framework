#ifndef __input_h__
#define __input_h__

#pragma once

#ifndef DIRECTINPUT_VERSION
# define DIRECTINPUT_VERSION	0x0800
#endif

#include <dinput.h>

class CInput
{
public:
	CInput() {}
	CInput(HWND hWnd, HINSTANCE hInstance, DWORD keybCoopFlags, DWORD mouseKeybState);
	~CInput();

	void Initialize(HWND hWnd, HINSTANCE hInstance, DWORD keybCoopFlags, DWORD mouseKeybState);

	void poll();

	bool keyDown(char key);
	bool mouseBtnDown(int button);

	float mouseX();
	float mouseY();
	float mouseZ();

private:
	IDirectInput8*			m_dInput;

	IDirectInputDevice8*	m_keyboard;
	char					m_kState[256];

	IDirectInputDevice8*	m_mouse;
	DIMOUSESTATE2			m_mouseState;

	HWND					m_hWnd;
};

#endif