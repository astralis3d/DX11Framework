#include "PCH.h"
#include "DebugViewManager.h"

CDebugViewManager::CDebugViewManager()
{

}

//------------------------------------------------------------------------
CDebugViewManager::~CDebugViewManager()
{
	ClearDebugViews();
}

//------------------------------------------------------------------------
void CDebugViewManager::AddDebugView(ID3D11ShaderResourceView* pSurface, const std::string& name, char accessKey)
{
	for (auto& i : m_debugViews)
	{
		if (i.accessChar == accessKey)
			return;
	}

	SDebugSurface surface;
	surface.accessChar = accessKey;
	surface.pSRV = pSurface;
	surface.surfaceName = name;

	m_debugViews.push_back(surface);
}

//------------------------------------------------------------------------
void CDebugViewManager::ClearDebugViews()
{
	m_debugViews.clear();
}

//------------------------------------------------------------------------
ID3D11ShaderResourceView* CDebugViewManager::GetDebugSurface(char accessKey)
{
	for (auto& i : m_debugViews)
	{
		if (i.accessChar == accessKey)
			return i.pSRV;
	}

	return nullptr;
}

//------------------------------------------------------------------------
const std::string CDebugViewManager::GetDebugViewDesc(char accessKey) const
{
	for (auto i : m_debugViews)
	{
		if (i.accessChar == accessKey)
			return i.surfaceName;
	}

	return std::string();
}
