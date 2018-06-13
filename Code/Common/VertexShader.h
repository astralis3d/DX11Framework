#ifndef __vertexshader_h__
#define __vertexshader_h__

#ifdef _MSC_VER
# pragma once
#endif

#include "D3DShader.h"

class CVertexShader : public IShader
{
public:
	CVertexShader();
	CVertexShader(ID3D11Device* pDevice, const char* file, const char* function, const CD3D11ShaderMacro* pDefines = nullptr);
	virtual ~CVertexShader();

	virtual void Release();

	operator ID3D11VertexShader*() const { return (ID3D11VertexShader*)GetHandle(); }

private:
};

#endif