#ifndef __utility_h__
#define __utility_h__

#ifdef _MSC_VER
# pragma once
#endif

#include "PCH.h"
#include <cstdio>

//----------------------------------------------------------------------
// For quick debugging in "Output" window in Visual Studio
//----------------------------------------------------------------------
int VDebugPrintf(const char* format, va_list args);
int DebugPrintf(const char* format, ...);

//----------------------------------------------------------------------
// For DirectX
//----------------------------------------------------------------------
HRESULT CreateInputLayoutDescFromVertexShaderSignature(ID3DBlob* pShaderBlob, ID3D11Device* pD3DDevice, ID3D11InputLayout** pInputLayout);

void SetSamplerStates(ID3D11DeviceContext* pDevCon);


void SaveBackbufferToFile(IDXGISwapChain* pSwapchain, ID3D11DeviceContext* pDevContext);
void SaveTextureToFile(ID3D11ShaderResourceView* pSRV, ID3D11DeviceContext* pDevCon);

//----------------------------------------------------------------------
// String Operations
//----------------------------------------------------------------------
inline std::wstring AnsiToWString(const char* ansiStr)
{
	WCHAR buffer[2048];
	MultiByteToWideChar(CP_ACP, 0, ansiStr, -1, buffer, 2048);

	return std::wstring(buffer);
}

//------------------------------------------------------------------------
// Error Handling
//------------------------------------------------------------------------
inline std::string GetWin32ErrorStringAnsi(DWORD errorCode)
{
	char errorString[MAX_PATH];

	::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,
					 nullptr,
					 errorCode,
					 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					 errorString,
					 MAX_PATH,
					 nullptr  );

	std::string message = "Win32 Error: ";
	message += errorString;

	return errorString;
}

inline std::wstring GetWin32ErrorString(DWORD errorCode)
{
	wchar_t errorString[MAX_PATH];

	::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
					nullptr,
					errorCode,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					errorString,
					MAX_PATH,
					nullptr  );

	std::wstring message = L"Win32 Error: ";
	message += errorString;

	return errorString;
}

inline std::string GetFullPath(const char* path)
{
	
}

inline std::string GetFilenameFromPath(const char* pathToFile)
{
	std::string str = std::string( pathToFile );

	std::string::size_type pos = str.find_last_of( "\\/" ) + 1;
	std::string path = str.substr( pos, str.length() - pos );

	return path;
}

//------------------------------------------------------------------------
uint32 CalculateShaderTimestamp(unsigned short seconds, unsigned short miliseconds);


#endif