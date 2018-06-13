#pragma once

#include "PCH.h"

struct GBuffer
{
	void InitializeGBuffer( ID3D11Device* pDevice, UINT Width, UINT Height );
	void Set( ID3D11DeviceContext* pDevCon );
	void ClearGBuffer( ID3D11DeviceContext* pDevCon, float DepthClearValue, bool bClearGBuffer );

	void Cleanup();


	DepthStencilBuffer		m_depthStencilBuffer;
	RenderTarget2D			m_RenderTarget0;
	RenderTarget2D			m_RenderTarget1;
	RenderTarget2D			m_RenderTarget2;

	RenderTarget2D			m_RenderTarget3;	// velocity buffer
};