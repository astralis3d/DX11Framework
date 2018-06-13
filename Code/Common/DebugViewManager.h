#ifndef DebugViewManager_h__
#define DebugViewManager_h__

#include "PCH.h"

enum class EDebugSurface
{
	NormalRender = 0,
	DepthBuffer,
	StencilBuffer,
	UserDefined
};

class CDebugViewManager
{
public:
	CDebugViewManager();
	~CDebugViewManager();

	void AddDebugView(ID3D11ShaderResourceView* pSurface, const std::string& name, char accessKey);
	void ClearDebugViews();

	ID3D11ShaderResourceView* GetDebugSurface(char accessKey);
	const std::string GetDebugViewDesc(char accessKey) const;

private:
	struct SDebugSurface
	{
		char accessChar;
		std::string surfaceName;
		ID3D11ShaderResourceView* pSRV;
	};

	typedef std::vector<SDebugSurface>	TDebugSurfaceVec;

	TDebugSurfaceVec	m_debugViews;
};


#endif // DebugViewManager_h__
