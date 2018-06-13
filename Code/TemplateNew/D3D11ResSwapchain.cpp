//------------------------------------------------------------------------
// File: D3D11ResSwapchain.cpp
//
// Creating & destroying resources which depend on the back buffer
// (textures, resource views...)
//------------------------------------------------------------------------

#include "PCH.h"
#include "MyApp.h"

//------------------------------------------------------------------------
HRESULT CMyApp::OnD3D11SwapChainResized(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	// For some reason, OnD3D11CreateDevice and OnD3D11SwapchainResized are called twice.
	// We do not want to waste processing power to load all resources twice, so perform some magic here.
	static bool bBlockFirstCall = true;
	if (bBlockFirstCall)
	{
		bBlockFirstCall = false;
		return S_OK;
	}


	CBaseApp::OnD3D11SwapChainResized(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc);


	const UINT Width = pBackBufferSurfaceDesc->Width;
	const UINT Height = pBackBufferSurfaceDesc->Height;

	// GBuffer
	m_GBuffer.InitializeGBuffer(pd3dDevice, Width, Height);

	// Other render targets
	m_RenderTargetColor.Initialize(pd3dDevice, Width, Height, DXGI_FORMAT_R16G16B16A16_FLOAT, true);
	m_RenderTargetFXAAProxy.Initialize(pd3dDevice, Width, Height, DXGI_FORMAT_R16G16B16A16_FLOAT);

	// Shadow maps
	const UINT ShadowMapSize = 2048;
	m_ShadowMap.Initialize(pd3dDevice, ShadowMapSize, ShadowMapSize, DXGI_FORMAT_D32_FLOAT, true);

	/* BLOOM */
	m_bloomTexHalfRes.Initialize(pd3dDevice, Width / 2, Height / 2, DXGI_FORMAT_R16G16B16A16_FLOAT);

	for (uint32 i = 0; i < 2; ++i) 
	{
		m_bloomTex[i].Initialize(pd3dDevice, Width / 4, Height / 4, DXGI_FORMAT_R11G11B10_FLOAT );
	}
	
	/* MOTION BLUR */
	m_motionBlurTex.Initialize(pd3dDevice, Width, Height, DXGI_FORMAT_R16G16B16A16_FLOAT);


	// Debug views
	m_debugViewMgr.AddDebugView( m_GBuffer.m_depthStencilBuffer.m_SRV, "Linear Depth", '1' );		// Linear depth
	m_debugViewMgr.AddDebugView( m_GBuffer.m_depthStencilBuffer.m_SRVStencil, "Stencil Buffer", '2' );
	m_debugViewMgr.AddDebugView( m_GBuffer.m_RenderTarget0.m_SRV, "GBuffer RT0", '3' );
	m_debugViewMgr.AddDebugView( m_GBuffer.m_RenderTarget1.m_SRV, "GBuffer RT1", '4' );
	m_debugViewMgr.AddDebugView( m_bloomTex[0].m_SRV, "Bloom", '5' );
	m_debugViewMgr.AddDebugView( m_HBAOPlus.GetHBAOPlusSRV(), "HBAO+", '6' );
	m_debugViewMgr.AddDebugView( m_motionBlurTex.m_SRV, "After motionblur", '7' );
	m_debugViewMgr.AddDebugView( m_lensFlares.m_renderTargetHalfRes.m_SRV, "LensFlaresThreshold", '8' );
	m_debugViewMgr.AddDebugView( m_lensFlares.m_renderTargetHalfRes2.m_SRV, "LensFlaresGhosts", '9' );

	// External
	m_textureLUT.LoadTexture(pd3dDevice, "..//..//media//LUT.dds");
	m_textureNoise.LoadTexture(pd3dDevice, "..//..//media//noise.dds");
	m_textureNormalsFit.LoadTexture( pd3dDevice, "..//..//Media//w3_normals_encode.dds" );
	m_textureVignetteMask.LoadTexture( pd3dDevice, "..//..//media//vignette.dds" );
	m_texLensDirt.LoadTexture( pd3dDevice, "..//..//media//lensdirt.png");
	m_texLensStar.LoadTexture( pd3dDevice, "..//..//media//lensstar.png" );
	
	return S_OK;
}

//------------------------------------------------------------------------
void CMyApp::OnD3D11SwapChainReleasing()
{
	// For some reason, OnD3D11CreateDevice and OnD3D11SwapchainResized are called twice.
	// We do not want to waste processing power to load all resources twice, so perform some magic here.
	static bool bBlockFirstCall = true;
	if (bBlockFirstCall)
	{
		bBlockFirstCall = false;
		return;
	}



	CBaseApp::OnD3D11SwapChainReleasing();

	// GBuffer
	m_GBuffer.Cleanup();

	// Render targets
	m_RenderTargetColor.Cleanup();
	m_RenderTargetFXAAProxy.Cleanup();

	// Shadows
	m_ShadowMap.Cleanup();

	// Bloom
	for (uint32 i = 0; i < 2; ++i)
	{
		m_bloomTex[i].Cleanup();
	}
	m_bloomTexHalfRes.Cleanup();

	/* MOTION BLUR */
	m_motionBlurTex.Cleanup();


	// External
	m_textureLUT.ReleaseTexture();
	m_textureNoise.ReleaseTexture();
	m_textureNormalsFit.ReleaseTexture();
	m_textureVignetteMask.ReleaseTexture();
}