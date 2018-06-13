#ifndef __FULLSCREENPASS_H__
#define __FULLSCREENPASS_H__

#ifdef _MSC_VER
# pragma once
#endif

#include <DXUT.h>
#include "D3DShaders.h"
#include "D3DConstantBuffer.h"

class FullscreenPass
{
public:
	FullscreenPass();
	~FullscreenPass();

	void OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc);
	void OnD3D11DestroyDevice();

	void RenderFullscreenTriangle(ID3D11DeviceContext* pDevContext);
	void RenderFullscreen(ID3D11DeviceContext* pDevContext, UINT Width, UINT Height);
	void RenderFullscreenTriangleEx(ID3D11DeviceContext* pDevContext, ID3D11RenderTargetView* pOutRTV, ID3D11ShaderResourceView* pInSRV, ID3D11PixelShader* pOverridePS);
	void RenderFullscreenTriangleRecopy(ID3D11DeviceContext* pDevContext, ID3D11RenderTargetView* pOutRTV, ID3D11ShaderResourceView* pInSRV);

	// Performs fullscreen pass with user-defined target width/height, input, output and pixel shader
	void RenderFullscreen( ID3D11DeviceContext* pDevContext, UINT targetWidth, UINT targetHeight, ID3D11ShaderResourceView* pInSourceSRV, ID3D11RenderTargetView* pOutTargetRTV, ID3D11PixelShader* pPixelShader );

	// For upscaling/downscaling
	void RenderFullscreenScaling(ID3D11DeviceContext* pDevContext, UINT targetWidth, UINT targetHeight, ID3D11ShaderResourceView* pInSourceSRV, ID3D11RenderTargetView* pOutTargetRTV);

private:
	// Fullscreen triangle vertex shader
	TVertexShader	m_VS;

	TPixelShader	m_PS;
	TPixelShader	m_PSRecopy;

	D3DConstantBuffer<Vec4>	m_CBPostProcessing;
};

#endif