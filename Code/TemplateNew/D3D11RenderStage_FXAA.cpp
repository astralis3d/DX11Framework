#include "PCH.h"
#include "MyApp.h"

void CMyApp::RenderStage_FXAA(ID3D11DeviceContext* pDevCon, ID3D11ShaderResourceView* pSRVInput, ID3D11RenderTargetView* pRTVOutput)
{
	// For debuugging purposes
	PIXEvent event(L"FXAA");
	
	m_GPUProfiler.StartTimer(pDevCon, TS_FXAA);
	m_PostProcessFXAA.Render(pDevCon, pSRVInput, pRTVOutput);
	m_GPUProfiler.EndTimer(pDevCon, TS_FXAA);
}
