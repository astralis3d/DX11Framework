#ifndef __pixelshader_h__
#define __pixelshader_h__

#ifdef _MSC_VER
# pragma once
#endif

#include "D3DShader.h"

class CPixelShader : public IShader
{
public:
	CPixelShader();
	CPixelShader(ID3D11Device* pDevice, const char* file, const char* function, const CD3D11ShaderMacro* pDefines = nullptr);
	virtual ~CPixelShader();

	virtual void Release();

	operator ID3D11PixelShader*() const { return (ID3D11PixelShader*)GetHandle(); }

private:
};

#endif