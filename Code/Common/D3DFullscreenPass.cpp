#include "PCH.h"
#include "D3DFullscreenPass.h"
#include "Utility.h"

FullscreenPass::FullscreenPass()
{

}

FullscreenPass::~FullscreenPass()
{
	OnD3D11DestroyDevice();
}

void FullscreenPass::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	m_PS.reset(new CPixelShader);
	m_PSRecopy.reset(new CPixelShader);
	m_VS.reset(new CVertexShader);

	m_PS->Initialize(pd3dDevice, "FullscreenPass.hlsl", "QuadPS");
	m_PSRecopy->Initialize(pd3dDevice, "..//Common//PostProcessCommon.hlsl", "QuadPSRecopy");
	m_VS->Initialize(pd3dDevice, "..//Common//PostProcessCommon.hlsl", "QuadVS");

	m_CBPostProcessing.Create(pd3dDevice);
}

//------------------------------------------------------------------------
void FullscreenPass::OnD3D11DestroyDevice()
{
	m_PS.reset();
	m_PSRecopy.reset();
	m_VS.reset();

	m_CBPostProcessing.Release();
}

//------------------------------------------------------------------------
void FullscreenPass::RenderFullscreenTriangle(ID3D11DeviceContext* pDevContext)
{
	// Simple fullscreen pass
	pDevContext->IASetInputLayout(nullptr);
	pDevContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	pDevContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_VS->Bind(pDevContext);

	pDevContext->Draw(3, 0);
}

//------------------------------------------------------------------------
// 
//------------------------------------------------------------------------
void FullscreenPass::RenderFullscreen(ID3D11DeviceContext* pDevContext, UINT Width, UINT Height)
{
	D3D11_VIEWPORT vpOld[1];
	UINT nViewPorts = 1;
	pDevContext->RSGetViewports(&nViewPorts, vpOld);

	// Setup the viewport to match the backbuffer
	D3D11_VIEWPORT vp;
	vp.Width =  static_cast<float>( Width );
	vp.Height = static_cast<float>( Height );
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pDevContext->RSSetViewports(1, &vp);

	// Render
	RenderFullscreenTriangle(pDevContext);

	// Restore the Old viewport
	pDevContext->RSSetViewports(nViewPorts, vpOld);
}

//------------------------------------------------------------------------
void FullscreenPass::RenderFullscreenTriangleEx(ID3D11DeviceContext* pDevContext, ID3D11RenderTargetView* pOutRTV, ID3D11ShaderResourceView* pInSRV, ID3D11PixelShader* pOverridePS)
{
	ID3D11RenderTargetView* renderTargets[1] = { pOutRTV };
	pDevContext->OMSetRenderTargets(1, renderTargets, nullptr);

	pDevContext->IASetInputLayout(nullptr);
	pDevContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	pDevContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_VS->Bind(pDevContext);

	ID3D11ShaderResourceView* shaderResources[1] = { pInSRV };
	pDevContext->PSSetShaderResources(0, 1, shaderResources);
	pDevContext->PSSetShader(pOverridePS, nullptr, 0);


	pDevContext->Draw(3, 0);
}

//------------------------------------------------------------------------
void FullscreenPass::RenderFullscreenTriangleRecopy(ID3D11DeviceContext* pDevContext, ID3D11RenderTargetView* pOutRTV, ID3D11ShaderResourceView* pInSRV)
{
	ID3D11RenderTargetView* renderTargets[1] = { pOutRTV };
	pDevContext->OMSetRenderTargets(1, renderTargets, nullptr);

	pDevContext->IASetInputLayout(nullptr);
	pDevContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	pDevContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_VS->Bind(pDevContext);

	ID3D11ShaderResourceView* shaderResources[1] = { pInSRV };
	pDevContext->PSSetShaderResources(0, 1, shaderResources);
	pDevContext->PSSetShader(*m_PSRecopy, nullptr, 0);


	pDevContext->Draw(3, 0);
}

//------------------------------------------------------------------------
void FullscreenPass::RenderFullscreen( ID3D11DeviceContext* pDevContext, UINT targetWidth, UINT targetHeight, ID3D11ShaderResourceView* pInSourceSRV, ID3D11RenderTargetView* pOutTargetRTV, ID3D11PixelShader* pPixelShader )
{
	D3D11_VIEWPORT vpOld[1];
	UINT nViewPorts = 1;
	pDevContext->RSGetViewports( &nViewPorts, vpOld );

	// Setup the viewport for target
	D3D11_VIEWPORT vp;
	vp.Width = static_cast<float>(targetWidth);
	vp.Height = static_cast<float>(targetHeight);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pDevContext->RSSetViewports( 1, &vp );

	// Set target RTV
	ID3D11RenderTargetView* renderTargets[1] = { pOutTargetRTV };
	pDevContext->OMSetRenderTargets( 1, renderTargets, nullptr );

	// Set input SRV
	ID3D11ShaderResourceView* shaderResources[1] = { pInSourceSRV };
	pDevContext->PSSetShaderResources( 0, 1, shaderResources );
	pDevContext->PSSetShader( pPixelShader, nullptr, 0 );

	// Set cbuffer stuff
	Vec4& cbufferData = m_CBPostProcessing.GetBufferData();
	cbufferData.x = vp.Width;
	cbufferData.y = vp.Height;
	cbufferData.z = cbufferData.z = 0.0f;
	m_CBPostProcessing.UpdateBuffer(pDevContext);
	m_CBPostProcessing.SetPS(pDevContext, 6);


	// Set IA, VS and draw
	RenderFullscreenTriangle( pDevContext );

	// Restore the Old viewport
	pDevContext->RSSetViewports( nViewPorts, vpOld );
}

//------------------------------------------------------------------------
void FullscreenPass::RenderFullscreenScaling( ID3D11DeviceContext* pDevContext, UINT targetWidth, UINT targetHeight, ID3D11ShaderResourceView* pInSourceSRV, ID3D11RenderTargetView* pOutTargetRTV )
{
	RenderFullscreen(pDevContext, targetWidth, targetHeight, pInSourceSRV, pOutTargetRTV, *m_PSRecopy);

	/*
	D3D11_VIEWPORT vpOld[1];
	UINT nViewPorts = 1;
	pDevContext->RSGetViewports(&nViewPorts, vpOld);

	// Setup the viewport for target
	D3D11_VIEWPORT vp;
	vp.Width =  static_cast<float>( targetWidth );
	vp.Height = static_cast<float>( targetHeight );
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pDevContext->RSSetViewports(1, &vp);

	// Set target RTV
	ID3D11RenderTargetView* renderTargets[1] = { pOutTargetRTV };
	pDevContext->OMSetRenderTargets( 1, renderTargets, nullptr );

	// Set input SRV
	ID3D11ShaderResourceView* shaderResources[1] = { pInSourceSRV };
	pDevContext->PSSetShaderResources( 0, 1, shaderResources );
	pDevContext->PSSetShader( *m_PSRecopy, nullptr, 0 );

	// Set IA, VS and draw
	RenderFullscreenTriangle(pDevContext);

	// Restore the Old viewport
	pDevContext->RSSetViewports(nViewPorts, vpOld);
	*/
}

