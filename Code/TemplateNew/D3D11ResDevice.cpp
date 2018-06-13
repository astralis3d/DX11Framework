#include "PCH.h"
#include "MyApp.h"
#include "AntTweakBar.h"

void CMyApp::LoadShaders(ID3D11Device* pDevice)
{
	// Shaders
	CD3D11ShaderMacro Defines;

	m_DeferredPixelShader = CompilePSFromFile( pDevice, "DeferredShading.hlsl", "PSMain" );
	m_DeferredComputeShader = CompileCSFromFile( pDevice, "DeferredShadingCS.hlsl", "DeferredShadingCS");
	m_PixelShaderFullscreenPass = CompilePSFromFile( pDevice, "FullscreenPass.hlsl", "QuadPS" );

	Defines.AddMacro( "MOTION_BLUR", "1" );
	Defines.Finish();
	m_PixelShaderFullscreenPassMotionBlur = CompilePSFromFile( pDevice, "FullscreenPass.hlsl", "QuadPS", &Defines );

	m_bloomPixelShaderBlurH = CompilePSFromFile( pDevice, "Bloom.hlsl", "BlurH_PS" );
	m_bloomPixelShaderBlurV = CompilePSFromFile( pDevice, "Bloom.hlsl", "BlurV_PS" );
	m_bloomPixelShaderThreshold = CompilePSFromFile( pDevice, "Bloom.hlsl", "BloomThreshold_PS" );

	m_motionBlurPixelShader = CompilePSFromFile( pDevice, "MotionBlur.hlsl", "MotionBlurPS" );

	m_pixelShaderPostprocessFinalStage = CompilePSFromFile( pDevice, "PostProcessFinalStage.hlsl", "PostProcesFinalPS" );

	// show vignette
	{
		CD3D11ShaderMacro definesVign;
		definesVign.AddMacro("SHOW_VIGNETTE", "1");
		definesVign.Finish();

		m_pixelShaderPostprocessFinalStageShowVignette = CompilePSFromFile( pDevice, "PostProcessFinalStage.hlsl", "PostProcesFinalPS", &definesVign );
	}


	m_pPixelShaderToneMapping = CompilePSFromFile( pDevice, "ToneMapping.hlsl", "ToneMappingPS" );

	

	m_skyboxVelocityPassPS = CompilePSFromFile( pDevice, "..//Common//SkyboxVelocityBuffer.hlsl", "SkyboxVelocityBufferPS" );
	m_skyboxVelocityPassVS = CompileVSFromFile( pDevice, "..//Common//SkyboxVelocityBuffer.hlsl", "SkyboxVelocityBufferVS" );
}
//------------------------------------------------------------------------
void CMyApp::SetupHUD(ID3D11Device* pDevice)
{
	// Setup AntTweakBar
	
	TwBar* bar = TwNewBar( "Demo" );

	char buffer[1024];
	const unsigned int barWidth = 220;
	const unsigned int barHeight = 300;
	_snprintf_s( buffer, sizeof( buffer ),
				 "Demo label='Demo' color='38 38 38' alpha=128 size='%d %d' position='%d %d' ",
				 barWidth, barHeight,
				 10, 400 );

	TwDefine( buffer );

	TwAddVarRW( bar, "CS for deferred shading", TW_TYPE_BOOLCPP, &m_bDefrredShadingUseComputeShader, " ");
	TwAddVarRW( bar, "Clear GBuffer", TW_TYPE_BOOLCPP, &m_bClearGBuffer, "");

	// Tonemapping
	TwAddVarRW( bar, "A", TW_TYPE_FLOAT, &m_toneMappingParams.CurveABCD.x, "Group=Tonemapping step=0.01" );
	TwAddVarRW( bar, "B", TW_TYPE_FLOAT, &m_toneMappingParams.CurveABCD.y, "Group=Tonemapping step=0.01" );
	TwAddVarRW( bar, "C", TW_TYPE_FLOAT, &m_toneMappingParams.CurveABCD.z, "Group=Tonemapping step=0.01" );
	TwAddVarRW( bar, "D", TW_TYPE_FLOAT, &m_toneMappingParams.CurveABCD.w, "Group=Tonemapping step=0.01" );
	TwAddVarRW( bar, "E", TW_TYPE_FLOAT, &m_toneMappingParams.CurveEF.x, "Group=Tonemapping step=0.01" );
	TwAddVarRW( bar, "F", TW_TYPE_FLOAT, &m_toneMappingParams.CurveEF.y, "Group=Tonemapping step=0.01" );
	TwAddVarRW( bar, "whitePointScale", TW_TYPE_FLOAT, &m_toneMappingParams.WhitePoint, "Group=Tonemapping step=0.01" );
	TwAddVarRW( bar, "nominatorMult", TW_TYPE_FLOAT, &m_toneMappingParams.NominatorMult, "Group=Tonemapping step=0.01" );
	TwAddVarRW( bar, "powParam", TW_TYPE_FLOAT, &m_toneMappingParams.powParam, "Group=Tonemapping step=0.01" );
	TwAddVarRW( bar, "minAllowedSceneLuminance", TW_TYPE_FLOAT, &m_toneMappingParams.MinMaxAllowedSceneLuminance.x, "Group=Tonemapping step=0.01" );
	TwAddVarRW( bar, "maxAllowedSceneLuminance", TW_TYPE_FLOAT, &m_toneMappingParams.MinMaxAllowedSceneLuminance.y, "Group=Tonemapping step=0.01" );

	// for vignette
	TwAddVarRW( bar, "TW3 vignette color", TW_TYPE_COLOR3F, &m_CBufferPostProcess.GetBufferData().m_VignetteTW3_Color, "Group=Postprocessing");
	TwAddVarRW( bar, "TW3 vignette weights", TW_TYPE_COLOR3F, &m_CBufferPostProcess.GetBufferData().m_VignetteTW3_Weights, "Group=Postprocessing" );
	TwAddVarRW( bar, "TW3 vignette opacity", TW_TYPE_FLOAT, &m_CBufferPostProcess.GetBufferData().m_VignetteTW3_Opacity, "Group=Postprocessing min=0 max=1 step=0.01");
	TwAddVarRW( bar, "show vignette", TW_TYPE_BOOLCPP, &m_bShowVignette, "Group=Postprocessing");

	// Sharpen
	//TwAddVarRW( bar, "p1 x", TW_TYPE_FLOAT, &m_sharpenParams.params1.x, "Group=Sharpen step=0.01" );
	//TwAddVarRW( bar, "p1 y", TW_TYPE_FLOAT, &m_sharpenParams.params1.y, "Group=Sharpen step=0.01" );
	//TwAddVarRW( bar, "p1 z", TW_TYPE_FLOAT, &m_sharpenParams.params1.z, "Group=Sharpen step=0.001" );
	//TwAddVarRW( bar, "p1 w", TW_TYPE_FLOAT, &m_sharpenParams.params1.w, "Group=Sharpen step=0.01" );
	//TwAddVarRW( bar, "p2 x", TW_TYPE_FLOAT, &m_sharpenParams.params2.x, "Group=Sharpen step=0.01" );
	//TwAddVarRW( bar, "p2 y", TW_TYPE_FLOAT, &m_sharpenParams.params2.y, "Group=Sharpen step=0.01" );
	//TwAddVarRW( bar, "p2 z", TW_TYPE_FLOAT, &m_sharpenParams.params2.z, "Group=Sharpen step=0.01" );
	//TwAddVarRW( bar, "p2 w", TW_TYPE_FLOAT, &m_sharpenParams.params2.w, "Group=Sharpen step=0.01" );
	

	// Mesh load/scale support
	{
		TwBar* pBarCommon = TwGetBarByName( "Common" );

		TwAddButton( pBarCommon, "Load mesh", LoadMesh, this, "Group=Resources" );
		//TwAddButton( pBarCommon, "Export mesh", 

		float& MeshScaleFactor = m_Scene.GetScaleFactor();
		TwAddVarRW( pBarCommon, "Mesh scale", TW_TYPE_FLOAT, &MeshScaleFactor, "Group=Resources" );
	}
}

//------------------------------------------------------------------------
HRESULT CMyApp::OnD3D11CreateDevice(ID3D11Device* pDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	// For some reason, OnD3D11CreateDevice and OnD3D11SwapchainResized are called twice.
	// We do not want to waste processing power to load all resources twice, so perform some magic here.
	static bool bBlockFirstCall = true;
	if (bBlockFirstCall)
	{
		bBlockFirstCall = false;
		return S_OK;
	}

	const uint32 Width = pBackBufferSurfaceDesc->Width;
	const uint32 Height = pBackBufferSurfaceDesc->Height;

	
	CBaseApp::OnD3D11CreateDevice(pDevice, pBackBufferSurfaceDesc);

	LoadShaders(pDevice);
	
	// Systems
	m_GPUProfiler.Initialize( pDevice, TS_Max );

	m_Scene.Initialize(pDevice, &m_viewFrustum);
	m_PostProcessFXAA.Initialize(pDevice);
	

	// Initialize AntTweakBar
	SetupHUD(pDevice);
	

	// Create D3D11 objects
	m_cbufferToneMapping.Create( pDevice );

	m_averageLuminance.Initialize(pDevice, &m_fullscreenPass, Width, Height, 10);
	m_sharpen.Initialize(pDevice, &m_fullscreenPass);

	m_lensFlares.Initialize(pDevice, &m_fullscreenPass, Width, Height);


	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));

	auto& blendRT0 = blendDesc.RenderTarget[0];
	blendRT0.BlendEnable = TRUE;
	blendRT0.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendRT0.SrcBlend = D3D11_BLEND_ONE;
	blendRT0.DestBlend = D3D11_BLEND_ONE;
	blendRT0.BlendOp = D3D11_BLEND_OP_ADD;

	blendRT0.SrcBlendAlpha = D3D11_BLEND_ONE;
	blendRT0.DestBlendAlpha = D3D11_BLEND_ONE;
	blendRT0.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	pDevice->CreateBlendState(&blendDesc, &m_bloomBlendStateAdditiveBlending);

	D3D11_DEPTH_STENCIL_DESC depthstencilState;
	depthstencilState.DepthEnable = TRUE;
	depthstencilState.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthstencilState.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
	depthstencilState.StencilEnable = FALSE;
	pDevice->CreateDepthStencilState( &depthstencilState, &m_pDepthStencilReversed );
	
	return S_OK;
}

//------------------------------------------------------------------------
void CMyApp::OnD3D11DestroyDevice()
{
	// For some reason, OnD3D11CreateDevice and OnD3D11SwapchainResized are called twice.
	// We do not want to waste processing power to load all resources twice, so perform some magic here.
	static bool bBlockFirstCall = true;
	if (bBlockFirstCall)
	{
		bBlockFirstCall = false;
		return;
	}



	CBaseApp::OnD3D11DestroyDevice();

	SAFE_RELEASE( m_bloomBlendStateAdditiveBlending );

	SAFE_RELEASE( m_pDepthStencilReversed );

	m_cbufferToneMapping.Release();
	

	// Systems
	m_Scene.Cleanup();
	m_GPUProfiler.Shutdown();
	m_averageLuminance.Release();
	m_sharpen.Release();
	m_lensFlares.Release();

	m_PostProcessFXAA.Release();
	m_PostProcessFXAA.AfterReset(0, 0);
}