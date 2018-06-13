#include "PCH.h"
#include "HBAOPlus.h"
#include "../!external/HBAOPlus/include/GFSDK_SSAO.h"

// Needed library
#pragma comment(lib, "../../Lib/GFSDK_SSAO_D3D11.win64.lib")

HBAOPlus::HBAOPlus()
	: m_pAOContext(nullptr)
	, m_pHBAOParams(nullptr)
{

}

//------------------------------------------------------------------------
bool HBAOPlus::Initialize(ID3D11Device* pDevice, UINT Width, UINT Height, SHBAOPlusParams* const pHBAOPlusParams)
{
	if (!pHBAOPlusParams)
		return false;

	m_pHBAOParams = pHBAOPlusParams;


	GFSDK_SSAO_CustomHeap CustomHeap;
	CustomHeap.new_ = ::operator new;
	CustomHeap.delete_ = ::operator delete;

	GFSDK_SSAO_Status status;
	status = GFSDK_SSAO_CreateContext_D3D11(pDevice, &m_pAOContext, &CustomHeap);

	if (status == GFSDK_SSAO_OK)
	{
		//m_HBAOPlusRT.Initialize(pDevice, Width, Height, DXGI_FORMAT_R32_FLOAT);
		m_HBAOPlusRT.Initialize(pDevice, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM);

		return true;
	}
	else
	{
		return false;
	}
}

//------------------------------------------------------------------------
void HBAOPlus::Cleanup()
{
	SAFE_RELEASE(m_pAOContext);

	m_HBAOPlusRT.Cleanup();
}

//------------------------------------------------------------------------
void HBAOPlus::Render(ID3D11DeviceContext* pDevContext, const class Mat44& matProj, const class Mat44& matView,
					  ID3D11ShaderResourceView* pDepthSRV, ID3D11ShaderResourceView* pNormalsSRV)
{
	if (m_pHBAOParams->m_bEnableHBAOPlus)
	{
		GFSDK_SSAO_InputData_D3D11 Input;
		Input.DepthData.DepthTextureType = GFSDK_SSAO_HARDWARE_DEPTHS;
		Input.DepthData.pFullResDepthTextureSRV = pDepthSRV;
		Input.DepthData.ProjectionMatrix.Data = GFSDK_SSAO_Float4x4((const float*)&matProj);
		Input.DepthData.ProjectionMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
		Input.DepthData.MetersToViewSpaceUnits = m_pHBAOParams->m_fMetersToViewSpaceUnits;

		Input.NormalData.Enable = m_pHBAOParams->m_bEnableNormals;
		Input.NormalData.pFullResNormalTextureSRV = pNormalsSRV;


		//Mat44 matIdentity;
		//MatrixTranspose(matIdentity, &matIdentity);


		Input.NormalData.WorldToViewMatrix.Data = GFSDK_SSAO_Float4x4( (GFSDK_SSAO_FLOAT*) &matView);
		Input.NormalData.WorldToViewMatrix.Layout = GFSDK_SSAO_ROW_MAJOR_ORDER;
		Input.NormalData.DecodeScale = 2.f;
		Input.NormalData.DecodeBias = -1.f;

		GFSDK_SSAO_Output_D3D11 Output;
		Output.pRenderTargetView = m_HBAOPlusRT.m_RTV;
		Output.Blend.Mode = GFSDK_SSAO_OVERWRITE_RGB;

		GFSDK_SSAO_Parameters AOParams;
		AOParams.Radius = m_pHBAOParams->m_fRadius;
		AOParams.Bias = m_pHBAOParams->m_fBias;
		AOParams.SmallScaleAO = m_pHBAOParams->m_fSmallScaleAO;
		AOParams.LargeScaleAO = m_pHBAOParams->m_fLargeScaleAO;
		AOParams.PowerExponent = m_pHBAOParams->m_fPowerExponent;
		AOParams.Blur.Enable = m_pHBAOParams->m_bEnableBlur;
		AOParams.Blur.Sharpness = m_pHBAOParams->m_fBlurSharpness;
		AOParams.Blur.Radius = GFSDK_SSAO_BLUR_RADIUS_4;

		const GFSDK_SSAO_RenderMask RenderMask =/* m_DebugNormals ? GFSDK_SSAO_RENDER_DEBUG_NORMAL_Z :*/ GFSDK_SSAO_RENDER_AO;

		GFSDK_SSAO_Status Status;
		Status = m_pAOContext->RenderAO(pDevContext, Input, AOParams, Output, RenderMask);
		assert(Status == GFSDK_SSAO_OK);
	}
	else
	{
		// When HBAO+ is off, we clear its buffer to white color.
		// TODO: Make sure it will be cleared only once. // Mateusz

		const float ClearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

		pDevContext->ClearRenderTargetView(m_HBAOPlusRT.m_RTV, ClearColor);
	}
}

//------------------------------------------------------------------------
ID3D11ShaderResourceView* HBAOPlus::GetHBAOPlusSRV() const
{
	return m_HBAOPlusRT.m_SRV;
}

