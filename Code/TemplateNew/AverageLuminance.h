#ifndef __AVERAGELUMINANCE_H__
#define __AVERAGELUMINANCE_H__

#pragma once

#include "D3D11StructuredBuffer.h"

class FullscreenPass;


class CAverageLuminance
{
public:
	CAverageLuminance();
	~CAverageLuminance();

	void Initialize(ID3D11Device* pDevice, FullscreenPass* pFullscreenPass, uint32 fullscreenWidth, uint32 fullscreenHeight, const uint32 luminanceMips = 11);
	void Release();

	void RenderAvgLuminanceAndEyeAdaptation(ID3D11DeviceContext* pDevContext, ID3D11ShaderResourceView* pInputColor, ID3D11ShaderResourceView* pDepthSRV);

	void CalcAvgLuminance_TW3_CS(ID3D11DeviceContext* pDevContext, ID3D11ShaderResourceView* pInputColor, ID3D11ShaderResourceView* pDepthSRV);


	const RenderTarget2D& GetAdaptedLuminanceTexture() const;
	
private:
	void RenderAverageLuminance(ID3D11DeviceContext* pDevCon, ID3D11ShaderResourceView* pInputColor);
	void RenderEyeAdaptation(ID3D11DeviceContext* pDevCon);

	RenderTarget2D		m_texLogLuminance;
	RenderTarget2D		m_texAdaptedLuminance[2];
	uint32				m_nCurrLumTarget;

	PixelShaderPtr		m_pPixelShaderToneMappingLuminance;
	PixelShaderPtr		m_pPixelShaderToneMapAdaptedLuminance;



	// "The Witcher 3" compute shader approach
	ComputeShaderPtr			m_pComputeShaderPhase1;
	ComputeShaderPtr			m_pComputeShaderPhase2;
	D3DStructuredBuffer<uint32>*	m_csBuffer;
	RenderTarget2D				m_renderTargetQuarterRes;

	struct SCBuffer
	{
		Vec2 quarterSize;
		Vec2 params;

		Vec4 viewportFull;
		Vec4 params2;
	};
	D3DConstantBuffer<SCBuffer>	m_cbuffer;


	FullscreenPass*		m_pFullscreenPass;
};

#endif