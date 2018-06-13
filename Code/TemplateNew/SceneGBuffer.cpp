#include "PCH.h"
#include "SceneGBuffer.h"

//------------------------------------------------------------------------
void GBuffer::InitializeGBuffer( ID3D11Device* pDevice, UINT Width, UINT Height )
{
	// Depth/Stencil buffer
	DXGI_FORMAT fmtDepthStencil = DXGI_FORMAT_D24_UNORM_S8_UINT;
	m_depthStencilBuffer.Initialize( pDevice, Width, Height, fmtDepthStencil, true );

	// Render Targets:
	m_RenderTarget0.Initialize( pDevice, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM );
	m_RenderTarget1.Initialize( pDevice, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM );
	m_RenderTarget2.Initialize( pDevice, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM );

	m_RenderTarget3.Initialize( pDevice, Width, Height, DXGI_FORMAT_R16G16_FLOAT );
}

//------------------------------------------------------------------------
void GBuffer::Set( ID3D11DeviceContext* pDevCon )
{
	ID3D11RenderTargetView* GBufferRTVs[4] =
	{
		m_RenderTarget0.m_RTV,
		m_RenderTarget1.m_RTV,
		m_RenderTarget2.m_RTV,
		m_RenderTarget3.m_RTV
	};

	pDevCon->OMSetRenderTargets( 4, GBufferRTVs, m_depthStencilBuffer.m_DSV );
}

//------------------------------------------------------------------------
void GBuffer::ClearGBuffer( ID3D11DeviceContext* pDevCon, float DepthClearValue, bool bClearGBuffer )
{
	pDevCon->ClearDepthStencilView( m_depthStencilBuffer.m_DSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, DepthClearValue, 0 );

	const float ClearColor[4] = { 0.f, 0.f, 0.f, 0.f };
	if ( bClearGBuffer )
	{
		pDevCon->ClearRenderTargetView( m_RenderTarget0.m_RTV, ClearColor );
		pDevCon->ClearRenderTargetView( m_RenderTarget1.m_RTV, ClearColor );
		pDevCon->ClearRenderTargetView( m_RenderTarget2.m_RTV, ClearColor );	
	}

	// motion blur
	pDevCon->ClearRenderTargetView( m_RenderTarget3.m_RTV, ClearColor );
}

//------------------------------------------------------------------------
void GBuffer::Cleanup()
{
	m_depthStencilBuffer.Cleanup();

	m_RenderTarget0.Cleanup();
	m_RenderTarget1.Cleanup();
	m_RenderTarget2.Cleanup();
	m_RenderTarget3.Cleanup();
}
