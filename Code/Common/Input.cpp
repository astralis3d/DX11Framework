#include "PCH.h"
#include "Input.h"

CInput::CInput(HWND hWnd, HINSTANCE hInstance, DWORD keybCoopFlags, DWORD mouseKeybState)
{
	Initialize(hWnd, hInstance, keybCoopFlags, mouseKeybState);
}

//----------------------------------------------------------------------
CInput::~CInput()
{
	if (m_keyboard)
	{
		m_keyboard->Unacquire();
		m_keyboard->Release();
		m_keyboard = 0;
	}

	if (m_mouse)
	{
		m_mouse->Unacquire();
		m_mouse->Release();
		m_mouse = 0;
	}

	if (m_dInput)
		m_dInput->Release();
}

//------------------------------------------------------------------------
void CInput::Initialize(HWND hWnd, HINSTANCE hInstance, DWORD keybCoopFlags, DWORD mouseKeybState)
{
	DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_dInput, 0);

	m_dInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, 0);
	m_dInput->CreateDevice(GUID_SysMouse, &m_mouse, 0);

	m_keyboard->SetDataFormat(&c_dfDIKeyboard);
	m_mouse->SetDataFormat(&c_dfDIMouse2);

	m_keyboard->SetCooperativeLevel(hWnd, keybCoopFlags);
	m_mouse->SetCooperativeLevel(hWnd, mouseKeybState);

	m_keyboard->Acquire();
	m_mouse->Acquire();

	m_hWnd = hWnd;
}

//----------------------------------------------------------------------
void CInput::poll()
{
	/*
	if (GetFocus() != m_hWnd)
		return;*/

	if (FAILED(m_keyboard->GetDeviceState(sizeof(m_kState), (void*)m_kState)))
	{
		ZeroMemory(m_kState, sizeof(m_kState));
		m_keyboard->Acquire();
	}

	if (FAILED(m_mouse->GetDeviceState(sizeof(DIMOUSESTATE2), (void*)&m_mouseState)))
	{
		ZeroMemory(&m_mouseState, sizeof(m_mouseState));
		m_mouse->Acquire();
	}
}

//----------------------------------------------------------------------
bool CInput::keyDown(char key)
{
	return (m_kState[key] & 0x80) != 0;
}

//----------------------------------------------------------------------
bool CInput::mouseBtnDown(int button)
{
	return (m_mouseState.rgbButtons[button] & 0x80) != 0;
}

//----------------------------------------------------------------------
float CInput::mouseX()
{
	return (float) m_mouseState.lX;
}

//----------------------------------------------------------------------
float CInput::mouseY()
{
	return (float) m_mouseState.lY;
}

//----------------------------------------------------------------------
float CInput::mouseZ()
{
	return (float) m_mouseState.lZ;
}
