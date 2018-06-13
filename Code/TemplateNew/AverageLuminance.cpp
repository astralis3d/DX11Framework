#include "PCH.h"
#include "AverageLuminance.h"
#include "D3DFullscreenPass.h"

CAverageLuminance::CAverageLuminance()
	: m_nCurrLumTarget(0)
{

}

//------------------------------------------------------------------------
CAverageLuminance::~CAverageLuminance()
{

}

//------------------------------------------------------------------------
void CAverageLuminance::Initialize( ID3D11Device* pDevice, FullscreenPass* pFullscreenPass, uint32 fullscreenWidth, uint32 fullscreenHeight, const uint32 luminanceMips /*= 11*/ )
{
	m_pFullscreenPass = pFullscreenPass;


	// Create resources
	m_pPixelShaderToneMappingLuminance = CompilePSFromFile( pDevice, "Luminance.hlsl", "PSToneMappingLogLuminance" );
	m_pPixelShaderToneMapAdaptedLuminance = CompilePSFromFile( pDevice, "AdaptedLuminance.hlsl", "PSToneMappingCalcAdaptedLuminance" );

	m_texLogLuminance.Initialize( pDevice,
								  1 << (luminanceMips - 1),
								  1 << (luminanceMips - 1),
								  DXGI_FORMAT_R16_FLOAT,
								  false,
								  true, 
								  luminanceMips );

	for (uint32 i = 0; i < 2; ++i)
	{
		m_texAdaptedLuminance[i].Initialize( pDevice, 1, 1, DXGI_FORMAT_R16_FLOAT, true );
	}

	m_csBuffer = new D3DStructuredBuffer<uint32>( pDevice, 256, D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE );
	
	// Compute shader approach
	m_renderTargetQuarterRes.Initialize(pDevice, fullscreenWidth / 4, fullscreenHeight / 4, DXGI_FORMAT_R11G11B10_FLOAT, false);

	m_pComputeShaderPhase1 = CompileCSFromFile( pDevice, "Phase1.hlsl", "TW3_Phase1" );
	m_pComputeShaderPhase2 = CompileCSFromFile( pDevice, "Phase2.hlsl", "TW3_Phase2" );

	m_cbuffer.Create(pDevice);
	m_cbuffer.GetBufferData().quarterSize = Vec2( (float) m_renderTargetQuarterRes.Width, (float) m_renderTargetQuarterRes.Height );
	m_cbuffer.GetBufferData().params = Vec2(0.266, 0.766 );
	m_cbuffer.GetBufferData().params2 = Vec4( 0.22, 0.53, 4, 0);
	m_cbuffer.GetBufferData().viewportFull = Vec4( (float) fullscreenWidth, (float) fullscreenHeight, 0.f, 0.f );

}

//------------------------------------------------------------------------
void CAverageLuminance::Release()
{
	m_texLogLuminance.Cleanup();
	for (uint32 i = 0; i < 2; ++i)
	{
		m_texAdaptedLuminance[i].Cleanup();
	}



	m_renderTargetQuarterRes.Cleanup();
	SAFE_DELETE( m_csBuffer );
	m_cbuffer.Release();
}

//------------------------------------------------------------------------
void CAverageLuminance::RenderAvgLuminanceAndEyeAdaptation( ID3D11DeviceContext* pDevContext, ID3D11ShaderResourceView* pInputColor, ID3D11ShaderResourceView* pDepthSRV )
{
	RenderAverageLuminance( pDevContext, pInputColor );
//	CalcAvgLuminance_TW3_CS( pDevContext, pInputColor, pDepthSRV );
	RenderEyeAdaptation( pDevContext );

	// Flip render target index for the next frame.
	// Please note that similar flip is also present in GetAdaptedLuminanceTexture().
	m_nCurrLumTarget = !m_nCurrLumTarget;
}

//------------------------------------------------------------------------
const RenderTarget2D& CAverageLuminance::GetAdaptedLuminanceTexture() const
{
	return m_texAdaptedLuminance[!m_nCurrLumTarget];
}

//------------------------------------------------------------------------
// Calculate log(luminance) and downsample them to 1x1 texture.
//------------------------------------------------------------------------
void CAverageLuminance::RenderAverageLuminance( ID3D11DeviceContext* pDevCon, ID3D11ShaderResourceView* pInputColor )
{
	PIXEvent evt( L"Average Log Luminance (PS)" );
	

	ID3D11RenderTargetView* RTViews[1] = { m_texLogLuminance.m_RTV };
	pDevCon->OMSetRenderTargets( 1, RTViews, nullptr );

	ID3D11ShaderResourceView* pInputTextures[1] = { pInputColor };
	pDevCon->PSSetShaderResources( 0, 1, pInputTextures );

	m_pPixelShaderToneMappingLuminance->Shader()->Bind( pDevCon );

	m_pFullscreenPass->RenderFullscreen( pDevCon, m_texLogLuminance.Width, m_texLogLuminance.Height );


	// Generate mips to get average scene log(luminance) to 1x1
	pDevCon->GenerateMips( m_texLogLuminance.m_SRV );
}

//------------------------------------------------------------------------
void CAverageLuminance::CalcAvgLuminance_TW3_CS( ID3D11DeviceContext* pDevContext, ID3D11ShaderResourceView* pInputColor, ID3D11ShaderResourceView* pDepthSRV )
{
	PIXEvent evt( L"Average Log Luminance (TW3 ComputeShader)" );

	// Downsample to quarter res
	m_pFullscreenPass->RenderFullscreenScaling(pDevContext,
												m_renderTargetQuarterRes.Width,
												m_renderTargetQuarterRes.Height,
												pInputColor,
												m_renderTargetQuarterRes.m_RTV);


	const UINT values[4] = {0, 0, 0, 0};
	pDevContext->ClearUnorderedAccessViewUint( m_csBuffer->GetUAV(), values );


	ID3D11ShaderResourceView* pInputsSRV[2] = { pInputColor, pDepthSRV };
	pDevContext->CSSetShaderResources( 0, 2, pInputsSRV );

	ID3D11UnorderedAccessView* pBufferUAV[2] = { m_csBuffer->GetUAV(), m_texAdaptedLuminance[m_nCurrLumTarget].m_UAV };
	pDevContext->CSSetUnorderedAccessViews(0, 1, pBufferUAV, nullptr);

	m_cbuffer.UpdateBuffer(pDevContext);
	m_cbuffer.SetCS(pDevContext, 5);

	m_pComputeShaderPhase1->Shader()->Bind(pDevContext);

	pDevContext->Dispatch( (UINT) m_renderTargetQuarterRes.Height, 1, 1);



	// Phase 2
	m_pComputeShaderPhase2->Shader()->Bind(pDevContext);
	pDevContext->CSSetUnorderedAccessViews( 0, 2, pBufferUAV, nullptr );

	pDevContext->Dispatch(1, 1, 1);
}


//------------------------------------------------------------------------
// Calculate adapted luminance of scene using current one (from above) 
// and previous one.
//------------------------------------------------------------------------
void CAverageLuminance::RenderEyeAdaptation( ID3D11DeviceContext* pDevCon )
{
	PIXEvent evt( L"Eye Adaptation" );


	// Render Target: Adapted Luminance
	ID3D11RenderTargetView* pAdaptedLuminanceRTV = { m_texAdaptedLuminance[m_nCurrLumTarget].m_RTV };

	float ClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	pDevCon->ClearRenderTargetView( pAdaptedLuminanceRTV, ClearColor );

	// Input: Previous adapted luminance
	ID3D11ShaderResourceView* pLastLuminanceSRV = { m_texAdaptedLuminance[!m_nCurrLumTarget].m_SRV };
	ID3D11ShaderResourceView* pInputsSRV[2] = { m_texLogLuminance, pLastLuminanceSRV };

	pDevCon->OMSetRenderTargets( 1, &pAdaptedLuminanceRTV, nullptr );
	pDevCon->PSSetShaderResources( 0, 2, pInputsSRV );
	m_pPixelShaderToneMapAdaptedLuminance->Shader()->Bind( pDevCon );


	m_pFullscreenPass->RenderFullscreen( pDevCon, 1, 1 );
}
