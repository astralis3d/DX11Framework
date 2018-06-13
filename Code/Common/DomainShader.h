#ifndef __domainshader_h__
#define __domainshader_h__

#ifdef _MSC_VER
# pragma once
#endif

#include "D3DShader.h"

class CDomainShader : public IShader
{
public:
	CDomainShader();
	CDomainShader(ID3D11Device* pDevice, const char* file, const char* function, const CD3D11ShaderMacro* pDefines = nullptr);
	virtual ~CDomainShader();

	virtual void Release();

	operator ID3D11DomainShader*() const { return (ID3D11DomainShader*)GetHandle(); }

private:
};

#endif