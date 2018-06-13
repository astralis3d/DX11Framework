#ifndef __geometryshader_h__
#define __geometryshader_h__

#ifdef _MSC_VER
# pragma once
#endif

#include "D3DShader.h"

class CGeometryShader : public IShader
{
public:
	CGeometryShader();
	CGeometryShader(ID3D11Device* pDevice, const char* file, const char* function, const CD3D11ShaderMacro* pDefines = nullptr);
	virtual ~CGeometryShader();

	virtual void Release();

	operator ID3D11GeometryShader*() const { return (ID3D11GeometryShader*)GetHandle(); }

private:
};

#endif