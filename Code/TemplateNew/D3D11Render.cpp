#include "PCH.h"
#include "MyApp.h"

//------------------------------------------------------------------------
void CMyApp::OnD3D11RenderFrame(ID3D11Device* pDevice, ID3D11DeviceContext* pDevCon, double fTime, float fElapsedTime)
{
	// Set samplers, constant buffers and other standard stuff
	PrepareRendering(pDevCon);


	//------------------------------------------------------------------------
	// CLEAR BUFFERS
	//------------------------------------------------------------------------
	m_GPUProfiler.StartTimer(pDevCon, TS_MainClear);
	{
		// Marker
		PIXEvent pixEvent(L"Main Clear");

		// Clear depth/stencil buffers and render targets
		static const float ClearColor[4] = { 0.f, 0.f, 0.f, 0.f };

		pDevCon->ClearRenderTargetView( m_backBuffer.RTV, ClearColor );
		pDevCon->ClearRenderTargetView( m_RenderTargetColor.m_RTV, ClearColor );
		pDevCon->ClearRenderTargetView( m_RenderTargetFXAAProxy.m_RTV, ClearColor );	

		const float DepthClearValue = (m_bReversedDepth) ? 0.0f : 1.0f;
		m_GBuffer.ClearGBuffer( pDevCon, DepthClearValue, m_bClearGBuffer );
	}

	m_GPUProfiler.EndTimer(pDevCon, TS_MainClear);

	

	//------------------------------------------------------------------------
	// Simple shadow mapping
	//------------------------------------------------------------------------
	m_GPUProfiler.StartTimer(pDevCon, TS_ShadowMap);
	{
		RenderShadowMap(pDevCon);
	}
	m_GPUProfiler.EndTimer(pDevCon, TS_ShadowMap);

	
	//------------------------------------------------------------------------
	// RENDERING TO GBUFFER
	//------------------------------------------------------------------------
	D3D11_VIEWPORT vp;

	m_GPUProfiler.StartTimer(pDevCon, TS_RenderToGBuffer);
	{
		PIXEvent pixEvent(L"Render to GBuffer");

		// Set viewport for frame
		
		vp.Width = static_cast<float>(m_RenderTargetColor.Width);
		vp.Height = static_cast<float>(m_RenderTargetColor.Height);
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		pDevCon->RSSetViewports( 1, &vp );

		// Set GBuffer
		m_GBuffer.Set(pDevCon);
		if (m_bReversedDepth)
		{
			pDevCon->OMSetDepthStencilState( m_pDepthStencilReversed, 0 );			
		}
		else
		{
			pDevCon->OMSetDepthStencilState( States::Get()->pDepthNoStencil_DS, 0 );			
		}

		// Set "normals fit" texture for Crytek's BFN.
		m_textureNormalsFit.SetPS(pDevCon, 13);


		// Set proper rasterizer state (solid/wireframe)
		ID3D11RasterizerState* pRasterizerState = m_bWireframe ? States::Get()->pBackfaceCull_WireFrame_RS : States::Get()->pBackfaceCull_RS;
		pDevCon->RSSetState(pRasterizerState);


		// Render scene to GBuffer
		m_Scene.RenderScene(pDevCon);


		// Set solid rasterizer state
		pDevCon->RSSetState(States::Get()->pBackfaceCull_RS);
	}
	m_GPUProfiler.EndTimer(pDevCon, TS_RenderToGBuffer);

	
	//------------------------------------------------------------------------
	// HBAO+
	//------------------------------------------------------------------------
	m_GPUProfiler.StartTimer(pDevCon, TS_HBAOPlus);
	{
		PIXEvent pixEvent(L"HBAO+");

		ID3D11ShaderResourceView* pDepthSRV =   m_GBuffer.m_depthStencilBuffer;
		ID3D11ShaderResourceView* pNormalsSRV = m_GBuffer.m_RenderTarget1;
		
		m_HBAOPlus.Render(pDevCon,
						  m_pCameraFree->GetMatrixProj(),
						  m_pCameraFree->GetMatrixView(),
						  pDepthSRV,
						  pNormalsSRV);
	}
	m_GPUProfiler.EndTimer(pDevCon, TS_HBAOPlus);

	
	//------------------------------------------------------------------------
	// DEFERRED SHADING
	//------------------------------------------------------------------------
	m_GPUProfiler.StartTimer(pDevCon, TS_DeferredShading);
	{
		PIXEvent pixEvent(L"Deferred Shading");

		ID3D11ShaderResourceView* GBufferSRV[] =
		{
			m_GBuffer.m_depthStencilBuffer.m_SRV,
			m_GBuffer.m_RenderTarget0.m_SRV,
			m_GBuffer.m_RenderTarget1.m_SRV,
			m_GBuffer.m_RenderTarget2.m_SRV,
			m_HBAOPlus.GetHBAOPlusSRV(),
			m_ShadowMap.m_SRV
		};


		// Pixel Shader Approach
		if ( m_bDefrredShadingUseComputeShader == false )
		{
			// Assign render targets
			ID3D11RenderTargetView* RTViews[1] = { m_RenderTargetColor.m_RTV };
			pDevCon->OMSetRenderTargets( 1, RTViews, nullptr );

			
			// Assign proper shader resource view
			pDevCon->PSSetShaderResources( 1, 6, GBufferSRV );
			m_DeferredPixelShader->Shader()->Bind( pDevCon );

			m_fullscreenPass.RenderFullscreenTriangle( pDevCon );

			// Cleanup
			for (uint32 i = 0; i < ARRAYSIZE( GBufferSRV ); ++i)
				GBufferSRV[i] = nullptr;

			pDevCon->PSSetShaderResources( 1, 6, GBufferSRV );
		}
		// Compute Shader Approach
		else
		{
			// Unbind render targets from PS.
			ID3D11RenderTargetView* pNullRTV[3] = { nullptr, nullptr, nullptr };
			pDevCon->OMSetRenderTargets(3, pNullRTV, nullptr);


			// Assign unordered access view
			ID3D11UnorderedAccessView* UAViews[1] = { m_RenderTargetColor.m_UAV };
			UINT nInitialsCounts[1] = { 0 };
			pDevCon->CSSetUnorderedAccessViews(0, 1, UAViews, nInitialsCounts );

			// Assign proper shader resource view
			pDevCon->CSSetShaderResources( 1, 6, GBufferSRV );
			m_DeferredComputeShader->Shader()->Bind( pDevCon );

			
			UINT tilesX = (UINT) std::ceil( m_GBuffer.m_RenderTarget0.Width / 16.0f );
			UINT tilesY = (UINT) std::ceil( m_GBuffer.m_RenderTarget0.Height / 16.0f );
			pDevCon->Dispatch( tilesX, tilesY, 1 );

			// Cleanup
			for (uint32 i = 0; i < ARRAYSIZE( GBufferSRV ); ++i)
				GBufferSRV[i] = nullptr;

			pDevCon->CSSetShaderResources( 1, 6, GBufferSRV );
			UAViews[0] = nullptr;
			pDevCon->CSSetUnorderedAccessViews( 0, 1, UAViews, nInitialsCounts );
		}
		
	}
	m_GPUProfiler.EndTimer(pDevCon, TS_DeferredShading);

	

	// Forward pass
	
	//------------------------------------------------------------------------
	// FORWARD SKY
	//------------------------------------------------------------------------
	m_GPUProfiler.StartTimer(pDevCon, TS_ForwardSky);
	{
		PIXEvent pixEvent(L"Forward Sky");
	
		{
			// Assign render targets
			ID3D11RenderTargetView* renderTargets[1];
			renderTargets[0] = m_RenderTargetColor.m_RTV;
			pDevCon->OMSetRenderTargets( 1, renderTargets, m_GBuffer.m_depthStencilBuffer.m_DSVReadOnly );

			D3D11_VIEWPORT skyViewport = vp;
			skyViewport.MinDepth = (m_bReversedDepth) ? 0.0f : 1.0f;
			skyViewport.MaxDepth = skyViewport.MinDepth;
			pDevCon->RSSetViewports( 1, &skyViewport );

			// Set proper rasterizer state (solid/wireframe)
			ID3D11RasterizerState* pRasterizerState = m_bWireframe ? States::Get()->pBackfaceCull_WireFrame_RS : States::Get()->pBackfaceCull_RS;
			pDevCon->RSSetState( pRasterizerState );

			// also depth for skybox
			ID3D11ShaderResourceView* pDepthSRV[1] = { m_GBuffer.m_depthStencilBuffer.m_SRV };
			pDevCon->PSSetShaderResources(10, 1, pDepthSRV);

			m_Skybox.PreRender( pDevCon );
			m_Skybox.Render( pDevCon, m_pCameraFree->GetPosition(), m_pCameraFree->GetMatrixViewProj(), m_bDrawClouds, m_bUseEnvmap, m_bUseCIEClearSky );

			pDepthSRV[0] = nullptr;
			pDevCon->PSSetShaderResources( 10, 1, pDepthSRV );
		}

		// We need an additional pass for sky to generate motion vectors
		{
			// Assign render targets
			ID3D11RenderTargetView* renderTargets[1];
			renderTargets[0] = m_GBuffer.m_RenderTarget3.m_RTV;
			pDevCon->OMSetRenderTargets( 1, renderTargets, nullptr);

			float BlendFactor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
			pDevCon->OMSetBlendState(m_bloomBlendStateAdditiveBlending, BlendFactor, 0xFFFFFFFF);

			pDevCon->OMSetDepthStencilState( States::Get()->pNoDepthNoStencil_DS, 0 );

			m_skyboxVelocityPassPS->Shader()->Bind(pDevCon);
			m_skyboxVelocityPassVS->Shader()->Bind(pDevCon);

			m_Skybox.PreRender(pDevCon);
			m_Skybox.DrawSkySphereOnly(pDevCon);

			pDevCon->OMSetBlendState( nullptr, BlendFactor, 0xFFFFFFFF );
		}
		

		pDevCon->RSSetState(States::Get()->pBackfaceCull_RS);
	}
	m_GPUProfiler.EndTimer(pDevCon, TS_ForwardSky);
	

	//------------------------------------------------------------------------
	// Average luminance & eye adaptation
	//------------------------------------------------------------------------
	m_GPUProfiler.StartTimer(pDevCon, TS_AverageLuminance);
	m_averageLuminance.RenderAvgLuminanceAndEyeAdaptation(pDevCon, m_RenderTargetColor.m_SRV, m_GBuffer.m_depthStencilBuffer.m_SRV);
	m_GPUProfiler.EndTimer(pDevCon, TS_AverageLuminance );



	//------------------------------------------------------------------------	
	// Motion Blur
	//------------------------------------------------------------------------
	{
		PIXEvent evt(L"Motion Blur");

		D3D11_VIEWPORT mbViewport;
		mbViewport.Width = (float) m_RenderTargetColor.Width;
		mbViewport.Height = (float) m_RenderTargetColor.Height;
		mbViewport.TopLeftX = 0.0f;
		mbViewport.TopLeftY = 0.0f;
		mbViewport.MinDepth = 0.0f;
		mbViewport.MaxDepth = 1.0f;
		pDevCon->RSSetViewports(1, &mbViewport);

		ID3D11RenderTargetView* pOutputRTV[1] = { m_motionBlurTex.m_RTV };
		pDevCon->OMSetRenderTargets( 1, pOutputRTV, nullptr );

		ID3D11ShaderResourceView* pInputsSRV[2] = { m_RenderTargetColor.m_SRV, m_GBuffer.m_RenderTarget3.m_SRV };
		pDevCon->PSSetShaderResources(0, 2, pInputsSRV);

		m_motionBlurPixelShader->Shader()->Bind(pDevCon);	

		m_fullscreenPass.RenderFullscreenTriangle(pDevCon);
	}

	//------------------------------------------------------------------------
	// Bloom.
	//------------------------------------------------------------------------
	m_GPUProfiler.StartTimer( pDevCon, TS_Bloom );
	{
		PIXEvent evt( L"Bloom" );

		/* Additive blending */

		float BlendFactor[4] = { 1,1,1,1 };
		//pDevCon->OMSetBlendState( m_bloomBlendStateAdditiveBlending, BlendFactor, 0xFFFFFFFF );
		//m_fullscreenPass.RenderFullscreenScaling( pDevCon, m_motionBlurTex.Width, m_motionBlurTex.Height, m_bloomTex[0].m_SRV, m_motionBlurTex.m_RTV );
		//pDevCon->OMSetBlendState( nullptr, BlendFactor, 0xFFFFFFFF );

		// Downsample to halfres
		m_fullscreenPass.RenderFullscreenScaling( pDevCon, m_bloomTexHalfRes.Width, m_bloomTexHalfRes.Height, m_motionBlurTex.m_SRV, m_bloomTexHalfRes.m_RTV );

		// Downsample to 1/4 res (+threshold)
		UINT nBloomWidth = m_bloomTex[0].Width;
		UINT nBloomHeigth = m_bloomTex[0].Height;
		UINT nBloomTexIndex = 0;

		PixelShader* pShader_Threshold = m_bloomPixelShaderThreshold->Shader();
		m_fullscreenPass.RenderFullscreen( pDevCon, nBloomWidth, nBloomHeigth, m_bloomTexHalfRes.m_SRV, m_bloomTex[nBloomTexIndex].m_RTV, *pShader_Threshold );

		// Blur.
		PixelShader* pShader_BlurH = m_bloomPixelShaderBlurH->Shader();
		PixelShader* pShader_BlurV = m_bloomPixelShaderBlurV->Shader();

		for ( uint32 i = 0; i < 3; ++i )
		{
			ID3D11ShaderResourceView* pNullSRV[1] = { nullptr };
			pDevCon->PSSetShaderResources( 0, 1, pNullSRV );

			m_fullscreenPass.RenderFullscreen( pDevCon, nBloomWidth, nBloomHeigth, m_bloomTex[nBloomTexIndex].m_SRV, m_bloomTex[!nBloomTexIndex].m_RTV, *pShader_BlurH );
						

			//ID3D11RenderTargetView* pNullRTV[1] = { nullptr };
			//pDevCon->OMSetRenderTargets(1, pNullRTV, nullptr);
			pDevCon->PSSetShaderResources( 0, 1, pNullSRV );

			m_fullscreenPass.RenderFullscreen( pDevCon, nBloomWidth, nBloomHeigth, m_bloomTex[!nBloomTexIndex].m_SRV, m_bloomTex[nBloomTexIndex].m_RTV, *pShader_BlurV );
		}
	}
	m_GPUProfiler.EndTimer( pDevCon, TS_Bloom );

	//------------------------------------------------------------------------
	// Lens flares (HDR)
	//------------------------------------------------------------------------
	m_GPUProfiler.StartTimer( pDevCon, TS_LensFlares );
	{
		// Input is fullscreen color (after tonemapping) . We render it to halfres to obtain the most bright areas.
		m_lensFlares.Render( pDevCon, m_motionBlurTex.m_SRV );

	}
	m_GPUProfiler.EndTimer( pDevCon, TS_LensFlares );
	//----------------------------------------------------------------------
	// TONE MAPPING
	//----------------------------------------------------------------------
	m_GPUProfiler.StartTimer(pDevCon, TS_Tonemapping);
	{
		PIXEvent pixEvent(L"Post Processing - tonemapping");		

		// We need solid fill mode during post processing
		pDevCon->RSSetState(States::Get()->pBackfaceCull_RS);

		// Bind render targets
		ID3D11RenderTargetView* RTViews[1] = { m_RenderTargetFXAAProxy.m_RTV };
		pDevCon->OMSetRenderTargets(1, RTViews, nullptr);


		// Bind shader resources
		ID3D11ShaderResourceView* SRVs[] = 
		{
			m_motionBlurTex.m_SRV,
			m_averageLuminance.GetAdaptedLuminanceTexture().m_SRV,
			m_textureLUT.GetSRV(),
			m_GBuffer.m_depthStencilBuffer.m_SRV,
			m_bloomTex[0].m_SRV,
			m_lensFlares.m_renderTargetHalfRes2.m_SRV,
			m_texLensDirt.GetSRV(),
			m_texLensStar.GetSRV()
		};
		
		pDevCon->PSSetShaderResources(0, 8, SRVs);

		// Tonemapping cbuffer
		m_cbufferToneMapping.GetBufferData() = m_toneMappingParams;
		m_cbufferToneMapping.UpdateBuffer(pDevCon);
		m_cbufferToneMapping.SetPS(pDevCon, 5);

		m_pPixelShaderToneMapping->Shader()->Bind( pDevCon );
		
		m_fullscreenPass.RenderFullscreenTriangle(pDevCon);

	
		// Unbind output texture.
		SRVs[0] = nullptr;
		pDevCon->PSSetShaderResources( 0, 1, SRVs );
	}

	m_GPUProfiler.EndTimer(pDevCon, TS_Tonemapping);

	
	//------------------------------------------------------------------------
	// Update postprocessing cbuffer
	//------------------------------------------------------------------------
	{
		m_CBufferPostProcess.UpdateBuffer( pDevCon );
		m_CBufferPostProcess.SetPS( pDevCon, 4 );
		m_CBufferPostProcess.SetVS( pDevCon, 4 );
	}


	//------------------------------------------------------------------------
	// SHARPEN
	//------------------------------------------------------------------------
	m_GPUProfiler.StartTimer(pDevCon, TS_Sharpen);
	{
		ID3D11RenderTargetView* pRTVOutput = { m_motionBlurTex.m_RTV };
		
		// Bind shader resources
		ID3D11ShaderResourceView* pSRVInputs[] =
		{
			m_RenderTargetFXAAProxy.m_SRV,
			m_GBuffer.m_depthStencilBuffer.m_SRV
		};

		m_sharpen.Render(pDevCon, pSRVInputs, _countof(pSRVInputs), pRTVOutput);
	}
	m_GPUProfiler.EndTimer(pDevCon, TS_Sharpen);

	//------------------------------------------------------------------------
	// LENS FLARES
	//------------------------------------------------------------------------
	
	
	//------------------------------------------------------------------------
	// Postprocessing - stage 2
	// (chromatic aberration, vignette, gamma correction)
	//------------------------------------------------------------------------	 
	{
		PIXEvent evt(L"PostProcessing - final stage");

		ID3D11ShaderResourceView* pTextureNoise = m_textureNoise.GetSRV();
		pDevCon->PSSetShaderResources(1, 1, &pTextureNoise);

		ID3D11ShaderResourceView* pTextureVignetteTW3 = m_textureVignetteMask.GetSRV();
		pDevCon->PSSetShaderResources(2, 1, &pTextureVignetteTW3);

		PixelShader* ps = (m_bShowVignette) ? m_pixelShaderPostprocessFinalStageShowVignette->Shader() : m_pixelShaderPostprocessFinalStage->Shader();
		m_fullscreenPass.RenderFullscreen(pDevCon, m_RenderTargetColor.Width, m_RenderTargetColor.Height, m_motionBlurTex.m_SRV, m_RenderTargetColor.m_RTV, *ps);
	}

	//------------------------------------------------------------------------
	// Postprocessing - final stage (FXAA)
	//------------------------------------------------------------------------
	ID3D11ShaderResourceView* pFXAAInputSRV = (m_bDebugView) ? m_motionBlurTex.m_SRV : m_RenderTargetColor.m_SRV;
	RenderStage_FXAA( pDevCon, pFXAAInputSRV, GetBackbuffer() );
	




	//------------------------------------------------------------------------
	// DEBUG TEXTURE RENDERING
	//------------------------------------------------------------------------
	if (m_bDebugView)
	{
		PIXEvent pixEvent( L"Debug Texture" );
		
		// Assign output RTV, don't use depth/stencil buffers.
		ID3D11RenderTargetView* renderTargets[1];
		//renderTargets[0] = m_motionBlurTex.m_RTV;
		renderTargets[0] = GetBackbuffer();

		pDevCon->OMSetRenderTargets( 1, renderTargets, nullptr );

		// Assign proper shader for debug
		switch (m_debugSurface)
		{
			case EDebugSurface::DepthBuffer:	m_PSDebugViewDepth->Shader()->Bind( pDevCon );	break;
			case EDebugSurface::StencilBuffer:	m_pPSDebugViewStencil->Shader()->Bind( pDevCon );	break;
			default:							m_PSDebugView->Shader()->Bind( pDevCon );			break;
		}

		UINT shaderTextureSlot = 0;

		// Stencil buffer uses different texture register
		if (m_debugSurface == EDebugSurface::StencilBuffer)
			shaderTextureSlot = 1;

		// Assign texture to shader and render fullscreen triangle
		pDevCon->PSSetShaderResources( shaderTextureSlot, 1, &m_pDebugSRV );
		m_fullscreenPass.RenderFullscreenTriangle( pDevCon );
	}



	//------------------------------------------------------------------------
	// Render HUD & UI to backbuffer
	//-----------------------------------------------------------------------
	RenderStage_UI(pDevCon);


	//------------------------------------------------------------------------
	// Cleanup
	//------------------------------------------------------------------------
	ID3D11ShaderResourceView* pSRV[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	pDevCon->PSSetShaderResources(0, 16, pSRV);
	pDevCon->GSSetShaderResources(0, 16, pSRV);
	pDevCon->HSSetShaderResources(0, 16, pSRV);
	pDevCon->DSSetShaderResources(0, 16, pSRV);
	pDevCon->CSSetShaderResources(0, 16, pSRV);


	//------------------------------------------------------------------------
	// End of profiling
	//------------------------------------------------------------------------
	m_GPUProfiler.EndFrame(pDevCon);


	//------------------------------------------------------------------------
	// post-frame updates
	//------------------------------------------------------------------------

	// Update previous view-proj matrix		
	m_CBufferPerView.GetBufferData().mViewProjPrevFrame = m_pCameraFree->GetMatrixViewProj();
}

//------------------------------------------------------------------------
void CMyApp::PrepareRendering(ID3D11DeviceContext* pDevCon)
{
	// Some "common" stuff for rendering
	PIXEvent evtPreRender(L"Prepare Rendering");


	// Get information about scene (camera position/angles, vertex/triangle count etc..)
	GatherDisplayDebugInfo();

	// Begin profiling (start of current frame)
	m_GPUProfiler.BeginFrame(pDevCon);

	// Samplers for all types of shaders
	SetSamplerStates(pDevCon);


	// Updating constant buffers
	{
		PIXEventEx pixEvent(L"CBufferPerFrame update");
		pixEvent.Begin();

		SCBPerFrame& cbPerFrame = m_CBufferPerFrame.GetBufferData();
		cbPerFrame.time.x += m_timer.DeltaSeconds() * 0.01f;
		cbPerFrame.time.y = m_timer.DeltaSeconds();

		m_CBufferPerFrame.UpdateBuffer(pDevCon);
		m_CBufferPerFrame.SetPS(pDevCon, 0);

		if (m_bDefrredShadingUseComputeShader)
		{
			m_CBufferPerFrame.SetCS(pDevCon, 0);
		}

		pixEvent.End();

		PIXEventEx pixEvent2(L"ViewConstantBuffer update");
		pixEvent2.Begin();
		UpdateViewConstantBuffer(pDevCon);
		pixEvent2.End();
	}

	m_viewFrustum.CalculateViewFrustum(m_pCameraFree->GetMatrixViewProj());
}

//------------------------------------------------------------------------
void CMyApp::RenderShadowMap(ID3D11DeviceContext* pDevCon)
{
	// Simple directional shadow map. More to come. (CSM etc. )

	PIXEvent pixEvent(L"Shadow Map");


	// Calculate bounding sphere of mesh(scene)
	
	Vec3 meshMin, meshMax;
	m_Scene.GetBoundingBox(meshMin, meshMax);


	const float ScaleFactor = m_Scene.GetScaleFactor();
	Mat44 matrixScaling;
	MatrixScaling(&matrixScaling, ScaleFactor, ScaleFactor, ScaleFactor);
	meshMin = matrixScaling.TransformPoint(meshMin);
	meshMax = matrixScaling.TransformPoint(meshMax);

	Vec3 sphereCenter = (meshMin + meshMax) * 0.5f;
	float SphereRadius = (meshMax - sphereCenter).GetLength();


	// Calculate light view matrix (lookAt)
	Vec3 lightDir = m_lightDir;
	lightDir.Normalize();

	Vec3 lightPos = -2.f * SphereRadius * lightDir;
	Vec3 targetPos = sphereCenter;

	Mat44 lightView, lightProj;
	

	const Vec3 up( 0.f, 1.f, 0.f );


	//MatrixLookAtLH(lightPos, targetPos, up, &lightView);

	MatrixLookAtLH( Vec3(), lightDir, up, &lightView );


	// Now xform scene center by lightView
	Vec3 sphereCenterLS;	// sphere center in light space
	sphereCenterLS = lightView.TransformPoint(sphereCenterLS);

	// Calculate lightProj matrix
	{
		
		float l = sphereCenterLS.x - SphereRadius;
		float b = sphereCenterLS.y - SphereRadius;
		float n = sphereCenterLS.z - SphereRadius;
		float r = sphereCenterLS.x + SphereRadius;
		float t = sphereCenterLS.y + SphereRadius;
		float f = sphereCenterLS.z + SphereRadius;
		

		MatrixOrthographicOffCenterLH(l, r, b, t, n, f, &lightProj);
		//MatrixOrtographicLH(1500, 1500, 0.1, FAR_PLANE, &lightProj);
	}

	Mat44 lightViewProj;
	MatrixMult(lightView, lightProj, &lightViewProj);

	m_CBufferScene.GetBufferData().mViewProjShadow = lightViewProj;
	m_CBufferScene.GetBufferData().vLightDir = lightDir;
	m_CBufferScene.GetBufferData().pad01 = 0.f;
	m_CBufferScene.UpdateBuffer(pDevCon);

	m_CBufferScene.SetVS(pDevCon, 3);
	m_CBufferScene.SetPS(pDevCon, 3);
	if (m_bDefrredShadingUseComputeShader)
	{
		m_CBufferScene.SetCS(pDevCon, 3);
	}


	// TODO!!

	// Render depth of scene from point of light - shadow map
	ID3D11RenderTargetView* renderTargets[1];
	renderTargets[0] = nullptr;

	pDevCon->ClearDepthStencilView(m_ShadowMap.m_DSV, D3D11_CLEAR_DEPTH, 1.f, 0);
	pDevCon->OMSetDepthStencilState(States::Get()->pDepthNoStencil_DS, 0);
	pDevCon->OMSetRenderTargets(1, renderTargets, m_ShadowMap.m_DSV);


	// Set viewport for shadow map
	D3D11_VIEWPORT vp;
	vp.Width =  static_cast<float>(m_ShadowMap.Width);
	vp.Height = static_cast<float>(m_ShadowMap.Height);
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	pDevCon->RSSetViewports( 1, &vp );


	m_Scene.RenderSceneDepth(pDevCon, false);
}