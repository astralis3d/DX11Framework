#ifndef __hullshader_h__
#define __hullshader_h__

#ifdef _MSC_VER
# pragma once
#endif

#include "D3DShader.h"

class CHullShader : public IShader
{
public:
	CHullShader();
	CHullShader(ID3D11Device* pDevice, const char* file, const char* function, const CD3D11ShaderMacro* pDefines = nullptr);
	virtual ~CHullShader();

	virtual void Release();

	operator ID3D11HullShader*() const { return (ID3D11HullShader*)GetHandle(); }

private:
};

#endif