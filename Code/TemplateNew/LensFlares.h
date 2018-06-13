#pragma once

// forward decls
class FullscreenPass;

class CLensFlaresD3D11
{
public:
	CLensFlaresD3D11();
	~CLensFlaresD3D11();

	void Initialize(ID3D11Device* pDevice, FullscreenPass* pFullscreenPass, uint32 FullscreenWidth, uint32 FullscreenHeight);
	void Release();

	void Render(ID3D11DeviceContext* pDevContext, ID3D11ShaderResourceView* pInputSRV );

	RenderTarget2D		m_renderTargetHalfRes;
	RenderTarget2D		m_renderTargetHalfRes2;

private:
	PixelShaderPtr		m_pixelShaderThreshold;
	PixelShaderPtr		m_pixelShaderGhosts;

	PixelShaderPtr		m_pixelShaderBlurH;
	PixelShaderPtr		m_pixelShaderBlurV;

	FullscreenPass*		m_pFullscreenPass;
};