#pragma once

#include "renderdoc_app.h"

class CRenderDocPluginLoader
{
public:
	void Initialize();
	void Release();

	typedef RENDERDOC_API_1_1_1 RENDERDOC_API_CONTEXT;

	RENDERDOC_API_CONTEXT* GetRenderDocAPI() const;

private:
	void* m_pRenderDocDLL;
	RENDERDOC_API_CONTEXT*	m_pRenderDocAPI;
};