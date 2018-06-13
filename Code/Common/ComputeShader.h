#ifndef __computeshader_h__
#define __computeshader_h__

#ifdef _MSC_VER
# pragma once
#endif

#include "D3DShader.h"

class CComputeShader : public IShader
{
public:
	CComputeShader();
	CComputeShader(ID3D11Device* pDevice, const char* file, const char* function, const CD3D11ShaderMacro* pDefines = nullptr);
	virtual ~CComputeShader();

	virtual void Release();

	operator ID3D11ComputeShader*() const { return (ID3D11ComputeShader*)GetHandle(); }

private:
};

#endif