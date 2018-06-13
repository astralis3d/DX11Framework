#ifndef __MYAPP_H__
#define __MYAPP_H__

#pragma once

#include "BaseApp.h"
#include "Scene.h"
#include "PostProcessFXAA.h"
#include "SceneGBuffer.h"
#include "Sharpen.h"
#include "AverageLuminance.h"
#include "LensFlares.h"

#ifdef USE_RENDERDOC
# include "RenderDocPluginLoader.h"
#endif




enum EGPUTimestamps
{
	TS_WholeFrame,

	/* Insert your timestamps here */
	TS_MainClear,
	TS_ShadowMap,
	TS_RenderToGBuffer,	
	TS_HBAOPlus,
	TS_DeferredShading,
	TS_ForwardSky,
	TS_AverageLuminance,
	TS_Bloom,
	TS_Sharpen,
	TS_LensFlares,
	TS_Tonemapping,
	TS_FXAA,
	TS_UI,
	/* don't add/remove anything below... */

	TS_Max
};


// Forward decls
class CCameraFree;

class CMyApp : public CBaseApp
{
public:
	CMyApp();
	virtual ~CMyApp();

	virtual int run() override;
	virtual void OnFrameMove(double fTime, float fElapsedTime) override;

	// Win32 stuff
	virtual void OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown) override;

	// D3D11 rarely changed
	virtual bool ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings) final;

	// D3D11 resources that depend on the backbuffer
	virtual HRESULT OnD3D11SwapChainResized(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc) override;
	virtual void OnD3D11SwapChainReleasing() override;

	// D3D11 resources that are not dependant on the backbuffer
	virtual HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc) override;
	virtual void OnD3D11DestroyDevice() override;

	// Main rendering function
	virtual void OnD3D11RenderFrame(ID3D11Device* pDevice, ID3D11DeviceContext* pDevCon, double fTime, float fElapsedTime) override;

	static const float	NEAR_PLANE;
	static const float	FAR_PLANE;

	friend void __stdcall LoadMesh(void* clientData);
	friend void __stdcall ExportMesh(void* clientData);

private:
	void GatherDisplayDebugInfo();
	void UpdateViewConstantBuffer(ID3D11DeviceContext* pDevCon);
	void RenderShadowMap(ID3D11DeviceContext* pDevCon);

private:
	// Render Stages
	void RenderStage_UI(ID3D11DeviceContext* pDevCon);
	void RenderStage_FXAA(ID3D11DeviceContext* pDevCon, ID3D11ShaderResourceView* pSRVInput, ID3D11RenderTargetView* pRTVOutput);

	void PrepareRendering(ID3D11DeviceContext* pDevCon);

	void LoadShaders(ID3D11Device* pDevice);
	void SetupHUD(ID3D11Device* pDevice);

private:
#ifdef USE_RENDERDOC
	CRenderDocPluginLoader	m_renderDoc;
#endif


	GBuffer				m_GBuffer;
	CScene				m_Scene;

	PostProcessFXAA		m_PostProcessFXAA;

	PixelShaderPtr		m_DeferredPixelShader;
	ComputeShaderPtr	m_DeferredComputeShader;
	PixelShaderPtr		m_PixelShaderFullscreenPass;
	PixelShaderPtr		m_PixelShaderFullscreenPassMotionBlur;

	// Render Targets
	RenderTarget2D		m_RenderTargetColor;
	RenderTarget2D		m_RenderTargetFXAAProxy;

	// Shadow mapping
	DepthStencilBuffer	m_ShadowMap;

	// External textures
	CTexture			m_textureLUT;
	CTexture			m_textureNoise;
	CTexture			m_textureNormalsFit;
	CTexture			m_textureVignetteMask;


	/* HDR TONE MAPPING */
	/*
	RenderTarget2D		m_texLogLuminance;
	RenderTarget2D		m_texAdaptedLuminance[2];
	UINT				m_nCurrLumTarget = 0;
	PixelShaderPtr		m_pPixelShaderToneMappingLuminance;
	PixelShaderPtr		m_pPixelShaderToneMapAdaptedLuminance;
	*/

	PixelShaderPtr		m_pPixelShaderToneMapping;

	struct SToneMappingCB
	{
		SToneMappingCB()
		{
			CurveABCD = Vec4(0.25f, 0.30f, 0.10f, 0.35f);
			CurveEF = Vec2(0.02f, 0.35f);
			MinMaxAllowedSceneLuminance = Vec2(0.02f, 20.f);
			WhitePoint = 0.25f;
			NominatorMult = 1.05f;
			powParam = 0.5f;

			zPadding = 0.0f;
		}

		Vec4	CurveABCD;
		Vec2	CurveEF;
		Vec2	MinMaxAllowedSceneLuminance;
		float	WhitePoint;
		float	NominatorMult;
		float	powParam;
		float	zPadding;
	};
	SToneMappingCB		m_toneMappingParams;
	D3DConstantBuffer<SToneMappingCB>	m_cbufferToneMapping;


	/* BLOOM */
	RenderTarget2D		m_bloomTexHalfRes;
	RenderTarget2D		m_bloomTex[2];
	PixelShaderPtr		m_bloomPixelShaderBlurH;
	PixelShaderPtr		m_bloomPixelShaderBlurV;
	PixelShaderPtr		m_bloomPixelShaderThreshold;
	ID3D11BlendState*	m_bloomBlendStateAdditiveBlending;
	

	/* MOTION BLUR */
	RenderTarget2D		m_motionBlurTex;
	PixelShaderPtr		m_motionBlurPixelShader;

	PixelShaderPtr		m_pixelShaderPostprocessFinalStage;
	PixelShaderPtr		m_pixelShaderPostprocessFinalStageShowVignette;

	ID3D11DepthStencilState* m_pDepthStencilReversed = nullptr;
	
	CAverageLuminance	m_averageLuminance;
	CSharpenD3D11		m_sharpen;

	// Lens flares
	CLensFlaresD3D11	m_lensFlares;
	CTexture			m_texLensDirt;
	CTexture			m_texLensStar;

	/* SKYBOX VELOCITY */
	VertexShaderPtr					m_skyboxVelocityPassVS;
	PixelShaderPtr					m_skyboxVelocityPassPS;

	bool						m_bClearGBuffer = true;

	// It turns out that Pixel Shader is faster (wow!)
	bool							m_bDefrredShadingUseComputeShader = false;
	bool							m_bShowVignette = false;
};

#endif