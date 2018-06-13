#pragma once

#include "D3DTypes.h"

// Forward decls
class GFSDK_SSAO_Context_D3D11;

class Mat44;

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11ShaderResourceView;


struct SHBAOPlusParams
{
	SHBAOPlusParams()
		: m_bEnableHBAOPlus(true)
		, m_bEnableNormals(true)
		, m_fMetersToViewSpaceUnits(10.0f)
		, m_fRadius(5.0f)
		, m_fBias(0.2f)
		, m_fPowerExponent(2.5f)
		, m_fSmallScaleAO(1.0f)
		, m_fLargeScaleAO(1.0f)
		, m_bEnableBlur(true)
		, m_fBlurSharpness(8.f)
	{

	}

	bool	m_bEnableHBAOPlus;
	bool	m_bEnableNormals;

	float	m_fMetersToViewSpaceUnits;

	float	m_fRadius;
	float	m_fBias;
	float	m_fPowerExponent;
	float	m_fSmallScaleAO;
	float	m_fLargeScaleAO;
	bool	m_bEnableBlur;
	float	m_fBlurSharpness;
};


class HBAOPlus
{
public:
	HBAOPlus();

	bool Initialize(ID3D11Device* pDevice, UINT Width, UINT Height, SHBAOPlusParams* const pHBAOPlusParams);
	void Cleanup();

	void Render(ID3D11DeviceContext* pDevContext, const Mat44& matProj, const Mat44& matView, ID3D11ShaderResourceView* pDepthSRV, ID3D11ShaderResourceView* pNormalsSRV);

	ID3D11ShaderResourceView* GetHBAOPlusSRV() const;

private:
	GFSDK_SSAO_Context_D3D11*	m_pAOContext;
	RenderTarget2D				m_HBAOPlusRT;

	SHBAOPlusParams*			m_pHBAOParams;
};