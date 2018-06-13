#include "PCH.h"
#include "LensFlares.h"
#include "D3DFullscreenPass.h"

CLensFlaresD3D11::CLensFlaresD3D11()
{

}

CLensFlaresD3D11::~CLensFlaresD3D11()
{

}

//------------------------------------------------------------------------
void CLensFlaresD3D11::Initialize( ID3D11Device* pDevice, FullscreenPass* pFullscreenPass, uint32 FullscreenWidth, uint32 FullscreenHeight )
{
	m_pFullscreenPass = pFullscreenPass;

	// Textures
	m_renderTargetHalfRes.Initialize(pDevice, FullscreenWidth / 4, FullscreenHeight / 4, DXGI_FORMAT_R11G11B10_FLOAT);
	m_renderTargetHalfRes2.Initialize( pDevice, FullscreenWidth / 4, FullscreenHeight / 4, DXGI_FORMAT_R11G11B10_FLOAT );

	// Shaders
	m_pixelShaderThreshold = CompilePSFromFile(pDevice, "LensFlares.hlsl", "LensFlaresThreshold");
	m_pixelShaderGhosts = CompilePSFromFile( pDevice, "LensFlares.hlsl", "LensFlaresGhosts" );

	m_pixelShaderBlurH = CompilePSFromFile( pDevice, "LensFlaresBlur.hlsl", "LensFlaresBlurH_PS" );
	m_pixelShaderBlurV = CompilePSFromFile( pDevice, "LensFlaresBlur.hlsl", "LensFlaresBlurV_PS" );
}

//------------------------------------------------------------------------
void CLensFlaresD3D11::Release()
{
	m_renderTargetHalfRes.Cleanup();
	m_renderTargetHalfRes2.Cleanup();
}

//------------------------------------------------------------------------
void CLensFlaresD3D11::Render( ID3D11DeviceContext* pDevContext, ID3D11ShaderResourceView* pInputSRV )
{
	PIXEvent evt(L"Lens flares");

	// Unbound.
	ID3D11RenderTargetView* pOutputs[1] = {nullptr };
	pDevContext->OMSetRenderTargets(1, pOutputs, nullptr);

	// The first step is to render threshold to get the brightest parts of image.
	ID3D11ShaderResourceView* pInputs[1] = { pInputSRV };
	pDevContext->PSSetShaderResources(0, 1, pInputs );

	pOutputs[0] = { m_renderTargetHalfRes.m_RTV };

	pDevContext->OMSetRenderTargets( 1, pOutputs, nullptr );
	
	m_pixelShaderThreshold->Shader()->Bind( pDevContext );

	m_pFullscreenPass->RenderFullscreen( pDevContext, m_renderTargetHalfRes.Width, m_renderTargetHalfRes.Height );


	// stage 2
	
	// unbound
	pOutputs[0] = nullptr;
	pDevContext->OMSetRenderTargets( 1, pOutputs, nullptr );

	pInputs[0] = m_renderTargetHalfRes.m_SRV;
	pDevContext->PSSetShaderResources(0, 1, pInputs);

	// Render to m_renderTargetHalfRes2
	pOutputs[0] = m_renderTargetHalfRes2.m_RTV;
	pDevContext->OMSetRenderTargets(1, pOutputs, nullptr );

	m_pixelShaderGhosts->Shader()->Bind(pDevContext);
	m_pFullscreenPass->RenderFullscreen( pDevContext, m_renderTargetHalfRes2.Width, m_renderTargetHalfRes2.Height);

	// unbound
	

	RenderTarget2D* pTextures[2] = { &m_renderTargetHalfRes, &m_renderTargetHalfRes2 };
	
	const uint32 TextureWidth = m_renderTargetHalfRes2.Width;
	const uint32 TextureHeight = m_renderTargetHalfRes2.Height;

	// Blur
	const PixelShader* PS_BlurH = m_pixelShaderBlurH->Shader();
	const PixelShader* PS_BlurV = m_pixelShaderBlurV->Shader();

	for ( uint32 i = 0; i < 4; i++ )
	{
		pInputs[0] = nullptr;
		pDevContext->PSSetShaderResources( 0, 1, pInputs );

		m_pFullscreenPass->RenderFullscreen( pDevContext, TextureWidth, TextureHeight, m_renderTargetHalfRes2.m_SRV, m_renderTargetHalfRes.m_RTV, *PS_BlurH );

		pDevContext->PSSetShaderResources( 0, 1, pInputs );
		m_pFullscreenPass->RenderFullscreen( pDevContext, TextureWidth, TextureHeight, m_renderTargetHalfRes.m_SRV, m_renderTargetHalfRes2.m_RTV, *PS_BlurV );
	}

	pOutputs[0] = nullptr;
	pDevContext->OMSetRenderTargets( 1, pOutputs, nullptr );
}
