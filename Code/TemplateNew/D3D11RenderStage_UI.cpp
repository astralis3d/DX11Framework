#include "PCH.h"
#include "MyApp.h"
#include "AntTweakBar.h"

void CMyApp::RenderStage_UI(ID3D11DeviceContext* pDevCon)
{
	// Render UI to backbuffer.
	ID3D11RenderTargetView* renderTargets[1];
	renderTargets[0] = { GetBackbuffer() };

	pDevCon->OMSetRenderTargets(1, renderTargets, nullptr);

	// Begin timer
	m_GPUProfiler.StartTimer(pDevCon, TS_UI);

	// Marker name for debugging purposes (eg. RenderDoc)
	PIXEventEx evt( L"UI" );


	evt.Begin();

	if (m_bDrawUI)
	{
		// AntTweakBar
		TwDraw();

		m_textRenderer.Begin();

		m_textRenderer.DrawStringW(DXUTGetFrameStats(true), Vec2(m_RenderTargetColor.Width * 0.6f, 10));
		m_textRenderer.DrawStringW(AnsiToWString(m_szRenderBuffer), Vec2(m_RenderTargetColor.Width * 0.6f, 30));

		if (m_bDebugView)
		{
			m_textRenderer.DrawStringW(AnsiToWString(m_debugViewDesc.c_str()),
									   Vec2(m_RenderTargetColor.Width * 0.5f, m_RenderTargetColor.Height * 0.05f),
									   Vec3(0.f, 0.f, 0.f));
		}


		static int counter = 0;
		counter++;

		if (counter % 5 == 0)
		{
			sprintf_s( m_szGPUProfilerBuffer,
					   "Whole Frame: %0.3f ms\n"
					   "-Main Clear: %0.3f ms\n"
					   "-Shadow Map: %0.3f ms\n"
					   "-RenderToGbuffer: %0.3f ms\n"
					   "-HBAO+: %0.3f ms\n"
					   "-Deferred Shading: %0.3f ms\n"
					   "-Bloom: %0.3f ms\n"
					   "-ForwardSky: %0.3f ms\n"
					   "-AvgLuminance & adaptation: %0.3f ms\n"
					   "-LensFlares: %0.3f ms\n"
					   "-Tonemapping: %0.3f ms\n"
					   "-Sharpen: %0.3f ms\n"
					   "-FXAA: %0.3f ms\n"
					   "-UI: %0.3f ms",
					   m_GPUProfiler.GetGPUTimeinMS(TS_WholeFrame),
					   m_GPUProfiler.GetGPUTimeinMS(TS_MainClear),
					   m_GPUProfiler.GetGPUTimeinMS(TS_ShadowMap),
					   m_GPUProfiler.GetGPUTimeinMS(TS_RenderToGBuffer),
					   m_GPUProfiler.GetGPUTimeinMS(TS_HBAOPlus),
					   m_GPUProfiler.GetGPUTimeinMS(TS_DeferredShading),
					   m_GPUProfiler.GetGPUTimeinMS(TS_Bloom),
					   m_GPUProfiler.GetGPUTimeinMS(TS_ForwardSky),
					   m_GPUProfiler.GetGPUTimeinMS(TS_AverageLuminance),
					   m_GPUProfiler.GetGPUTimeinMS(TS_LensFlares),
					   m_GPUProfiler.GetGPUTimeinMS(TS_Tonemapping),
					   m_GPUProfiler.GetGPUTimeinMS(TS_Sharpen),
					   m_GPUProfiler.GetGPUTimeinMS(TS_FXAA),
					   m_GPUProfiler.GetGPUTimeinMS(TS_UI)
			);

			counter = 0;
		}

		m_textRenderer.DrawStringW(AnsiToWString(m_szGPUProfilerBuffer),
								   Vec2(m_RenderTargetColor.Width * 0.8f, m_RenderTargetColor.Height - 300.0f),
								   Vec3(1.0f, 1.0f, 1.0f));


		m_textRenderer.End();
	}

	evt.End();

	// End timer for UI
	m_GPUProfiler.EndTimer(pDevCon, TS_UI);
}
