#include "PCH.h"
#include "RenderDocPluginLoader.h"

void CRenderDocPluginLoader::Initialize()
{
	// Init RenderDoc API
	m_pRenderDocAPI = nullptr;

	m_pRenderDocDLL = LoadLibraryA( "..//..//Lib//renderdoc.dll" );
	if (!m_pRenderDocDLL)
	{
		m_pRenderDocDLL = LoadLibraryA( "renderdoc.dll" );
	}


	if (m_pRenderDocDLL)
	{
		const pRENDERDOC_GetAPI RENDERDOC_GETAPI = (pRENDERDOC_GetAPI)GetProcAddress( (HMODULE) m_pRenderDocDLL, "RENDERDOC_GetAPI" );
		const int returnValue = RENDERDOC_GETAPI( eRENDERDOC_API_Version_1_1_1, (void**)&m_pRenderDocAPI );

		if (returnValue != 1)
		{
			DebugPrintf( "[WARNING] RenderDOC_GetAPI did not return 1" );
		}

		//m_pRenderDocAPI->SetCaptureKeys(nullptr, 0);
		m_pRenderDocAPI->SetFocusToggleKeys( nullptr, 0 );

	}
	else
	{
		DebugPrintf( GetWin32ErrorStringAnsi( ::GetLastError() ).c_str() );
		DebugPrintf( "[WARNING] Loading renderdoc.dll failed." );
	}

}

void CRenderDocPluginLoader::Release()
{
	FreeLibrary( (HMODULE) m_pRenderDocDLL );
}

CRenderDocPluginLoader::RENDERDOC_API_CONTEXT* CRenderDocPluginLoader::GetRenderDocAPI() const
{
	return m_pRenderDocAPI;
}
