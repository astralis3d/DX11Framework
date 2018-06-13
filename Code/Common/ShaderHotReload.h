#ifndef __shaderhotreload_h__
#define __shaderhotreload_h__

// Prototype, singleton-based shaders hot reloading.

#ifdef _MSC_VER
# pragma once
#endif

#include "PCH.h"

class IShader;

class CShaderHotReload
{
public:
	static CShaderHotReload* Get();

	void AddShader(IShader* pShader);
	void ReloadShaders(bool bReloadAll = false);

	void Flush();

private:
	typedef std::vector<IShader*>	TShadersVec;
	TShadersVec		m_shaders;

	// private ctor & dtor
	CShaderHotReload();
	~CShaderHotReload();
};

#endif