// BaseApp.h
// Contains base functionality used for projects

#ifndef __BASEAPP_H__
#define __BASEAPP_H__

#pragma once

#include "DXUTApp.h"
#include "TextRenderer.h"
#include "ViewFrustum.h"
#include "CameraFree.h"
#include "Timer.h"
#include "DebugViewManager.h"
#include "D3DFullscreenPass.h"
#include "Input.h"
#include "HBAOPlus.h"
#include "D3DConstantBuffer.h"
#include "D3DProfiler.h"
#include "Skybox.h"


class States;

struct SBackBuffer
{
	SBackBuffer() : RTV(nullptr) {}

	uint32					Width;
	uint32					Height;
	ID3D11RenderTargetView* RTV;
};

class CBaseApp : public CDXUTApp11
{
public:
	CBaseApp();
	virtual ~CBaseApp();

	virtual int run() override;

	virtual void OnFrameMove(double fTime, float fElapsedTime) override;

	// Win32 stuff
	virtual LRESULT WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool* pBNoFurtherProcessing) override;
	virtual void OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown) override;

	// D3D11 resources that depend on the backbuffer
	virtual HRESULT OnD3D11SwapChainResized(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc) override;
	virtual void OnD3D11SwapChainReleasing() override;

	// D3D11 resources that are not dependant on the backbuffer
	virtual HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc) override;
	virtual void OnD3D11DestroyDevice() override;

	uint32 GetBackbufferWidth() const;
	uint32 GetBackbufferHeight() const;
	ID3D11RenderTargetView* GetBackbuffer() const;

	friend void __stdcall LoadSkySphere(void* clientData);
	friend void __stdcall LoadSkyCubemap(void* clientData);

protected:
	SBackBuffer					m_backBuffer;

	CSkybox						m_Skybox;
	bool						m_bUseCIEClearSky = false;
	bool						m_bUseEnvmap = false;
	bool						m_bDrawClouds = true;

	// Systems
	CCameraFree*				m_pCameraFree = nullptr;
	CTextRenderer				m_textRenderer;
	CViewFrustum				m_viewFrustum;
	FullscreenPass				m_fullscreenPass;
	CTimer						m_timer;
	CInput						m_Input;
	HBAOPlus					m_HBAOPlus;
	SHBAOPlusParams				m_HBAOPlusParams;

	States*						m_pStates = nullptr;

	// Debugging
	CDebugViewManager			m_debugViewMgr;
	ID3D11ShaderResourceView*	m_pDebugSRV = nullptr;
	std::string					m_debugViewDesc;
	EDebugSurface				m_debugSurface = EDebugSurface::NormalRender;

	char						m_szRenderBuffer[512];	
	char						m_szGPUProfilerBuffer[1024];
	D3DProfiler					m_GPUProfiler;

	// Common stuff
	float						m_fVignetteIntensity = 0.35f;
	//Vec3						m_lightDir = Vec3(0.277f, -0.577f, 0.277f);
	Vec3						m_lightDir = Vec3(-0.72f, -0.40f, -0.52f);

	// Settings
	bool				m_bDrawUI = true;
	bool				m_bDebugView = false;
	bool				m_bWireframe = false;
	bool				m_bReversedDepth = true;

	// Shaders
	PixelShaderPtr		m_PSDebugView;
	PixelShaderPtr		m_PSDebugViewDepth;
	PixelShaderPtr		m_pPSDebugViewStencil;


	// Contant Buffers
	D3DConstantBuffer<SCBPerFrame>			m_CBufferPerFrame;
	D3DConstantBuffer<SCBPerView>			m_CBufferPerView;
	D3DConstantBuffer<SCBScene>				m_CBufferScene;
	D3DConstantBuffer<SCBPostProcess>		m_CBufferPostProcess;
	D3DConstantBuffer<SCBPerStaticObject>	m_CBufferPerStaticObject;
};


#endif