#include "PCH.h"
#include "BaseApp.h"
#include "D3DStates.h"
#include "ShaderHotReload.h"
#include "AntTweakBar.h"

#include <commdlg.h>


// TODO: Rename
bool LoadingHelper(const char* lookupDirectory, std::string& outPath)
{
	char szEXEPath[MAX_PATH];
	GetModuleFileNameA(NULL, szEXEPath, _MAX_PATH);

	// Find path to "Media" directory
	const int MAX_DIRS_UP = 5;


	std::string path;
	std::string::size_type pos = std::string(szEXEPath).find_last_of("\\/");
	path = std::string(szEXEPath).substr(0, pos);

	bool bSuccess = false;
	int dirsUp = 0;
	do
	{
		path.append(lookupDirectory);


		DWORD dwAttrib = GetFileAttributesA(path.c_str());
		if ((dwAttrib != INVALID_FILE_ATTRIBUTES) && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
		{
			bSuccess = true;
			break;
		}
		else
		{
			path = path.substr(0, pos);
			pos = path.find_last_of("\\/");
			path = path.substr(0, pos);
		}
	} while (dirsUp++ < MAX_DIRS_UP);


	if (bSuccess)
	{
		// GetOpenFileName/GetSaveFileName changes the Current Working Directory (CWD)
		// on success. Copy it for now.
		char szCWD[MAX_PATH];
		GetCurrentDirectoryA(MAX_PATH, szCWD);


		char szFinalPath[MAX_PATH] = "";

		OPENFILENAMEA ofn;
		memset(&ofn, 0, sizeof(OPENFILENAME));

		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = DXUTGetHWND();
		ofn.lpstrFilter = "All files\0*.*\0";
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrInitialDir = path.c_str();
		ofn.lpstrFile = szFinalPath;
		ofn.nMaxFile = MAX_PATH;

		ofn.Flags = OFN_HIDEREADONLY | OFN_ENABLESIZING | OFN_FILEMUSTEXIST | OFN_EXPLORER;

		bSuccess = !!::GetOpenFileNameA(&ofn);

		if (bSuccess)
		{
			// Restore CWD
			SetCurrentDirectoryA(szCWD);

			outPath = std::string(szFinalPath);
		}
	}

	return bSuccess;
}


//------------------------------------------------------------------------
void TW_CALL LoadSkySphere(void* clientData)
{
	const char* lookupDir = "\\Media\\Textures\\Sky\\";
	std::string path;

	if ( LoadingHelper(lookupDir, path) )
	{
		CBaseApp* pApp = static_cast<CBaseApp*>(clientData);

		pApp->m_Skybox.SetTextureSphere( DXUTGetD3D11Device(), path.c_str() );
	}
}

//------------------------------------------------------------------------
void TW_CALL LoadSkyCubemap(void* clientData)
{
	const char* lookupDir = "\\Media\\Textures\\Envmaps\\";
	std::string path;

	if ( LoadingHelper(lookupDir, path) )
	{
		CBaseApp* pApp = static_cast<CBaseApp*>(clientData);

		pApp->m_Skybox.SetTextureCubemap( DXUTGetD3D11Device(), path.c_str() );
	}
}

//------------------------------------------------------------------------
CBaseApp::CBaseApp()
	: CDXUTApp11()
{
	memset(m_szRenderBuffer, 0, sizeof(m_szRenderBuffer));
	memset(m_szGPUProfilerBuffer, 0, sizeof(m_szGPUProfilerBuffer));
}

//------------------------------------------------------------------------
CBaseApp::~CBaseApp()
{
	// Cleanup
	SAFE_DELETE(m_pCameraFree);

	
}

//------------------------------------------------------------------------
int CBaseApp::run()
{
	CDXUTApp11::run();

	// Input
	const DWORD MouseCoopFlags = DISCL_NONEXCLUSIVE | DISCL_FOREGROUND;
	const DWORD KeyboardCoopFlags = DISCL_NONEXCLUSIVE | DISCL_FOREGROUND;

	m_Input.Initialize(DXUTGetHWND(), DXUTGetHINSTANCE(), KeyboardCoopFlags, MouseCoopFlags);

	return 0;
}

//------------------------------------------------------------------------
void CBaseApp::OnFrameMove(double fTime, float fElapsedTime)
{
	m_timer.Update();
	m_Input.poll();
}

//------------------------------------------------------------------------
LRESULT CBaseApp::WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool* pBNoFurtherProcessing)
{
	if (m_bDrawUI)
	{
		if (TwEventWin(hWnd, msg, wParam, lParam))
		{
			return 0;
		}
	}

	switch (msg)
	{
		case WM_SETFOCUS:
		{
			// Uncomment these lines if you want to have auto reloading of shaders

			// CShaderHotReload::Get()->ReloadShaders();
			// DebugPrintf("Reloading shaders..\n");
		}
		break;

		case WM_RBUTTONDOWN:
		{
			RECT rect;
			GetWindowRect(DXUTGetHWND(), &rect);

			rect.left += 2;
			rect.top += 2;
			rect.bottom -= 2;
			rect.right -= 2;
			ClipCursor(&rect);

			m_pCameraFree->OnRMBDown();
			m_pCameraFree->SetRMB(true);

						
			int r = ShowCursor(FALSE);
			while (r >= 0)
				r = ShowCursor(FALSE);
			

			break;
		}

		case WM_RBUTTONUP:
		{
			ClipCursor(nullptr);

			m_pCameraFree->OnRMBUp();
			m_pCameraFree->SetRMB(false);

			int r = ShowCursor(TRUE);
			while (r < 0)
				r = ShowCursor(TRUE);

			break;
		}

		case WM_SIZE:
		{
			//HWND hWnd = DXUTGetHWND();
			RECT rect;
			GetClientRect(hWnd, &rect);
			
			const int Width = rect.right - rect.left;
			const int Height = rect.bottom - rect.top;
			
			m_pCameraFree->SetDimensions(static_cast<float>(Width), static_cast<float>(Height));

		} break;

		case WM_KEYDOWN:
		{
			OnKeyboard((UINT)wParam, true, false);
		}
		break;

		case WM_MOUSEMOVE:
		{
			m_pCameraFree->HandleMousemove();

		} break;

		case WM_MOUSEWHEEL:
		{
			const float sign = (GET_WHEEL_DELTA_WPARAM(wParam) > 0) ? 1.f : -1.f;

			float moveSpeed = m_pCameraFree->GetSpeedMove();
			moveSpeed += 10.f * sign;
			moveSpeed = std::max<float>(10.f, moveSpeed);

			m_pCameraFree->SetSpeedMove(moveSpeed);
		}
		break;

		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return 0;
}

//------------------------------------------------------------------------
void CBaseApp::OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown)
{
	if (bKeyDown)
	{
		switch( nChar )
		{
			// Normal rendering - no debug view
			case '0':
			{
				m_bDebugView = false;
			}
			break;

			// Depth view
			case '1':
			{
				m_bDebugView = true;
				m_debugSurface = EDebugSurface::DepthBuffer;

				m_pDebugSRV = m_debugViewMgr.GetDebugSurface((char)nChar);
				m_debugViewDesc = m_debugViewMgr.GetDebugViewDesc((char)nChar);
			}
			break;

			// Stencil buffer
			case '2':
			{
				m_bDebugView = true;
				m_debugSurface = EDebugSurface::StencilBuffer;

				m_pDebugSRV = m_debugViewMgr.GetDebugSurface((char)nChar);
				m_debugViewDesc = m_debugViewMgr.GetDebugViewDesc((char)nChar);
			}
			break;

			// HUD
			case 'H':
			{
				m_bDrawUI = !m_bDrawUI;		

			} break;

			// Wireframe on/off
			case VK_F4:
				m_bWireframe = !m_bWireframe;	break;

			// Take a screenshot
			case VK_F5:
			{
				IDXGISwapChain* pSwapchain = DXUTGetDXGISwapChain();
				ID3D11DeviceContext* pDevContext = DXUTGetD3D11DeviceContext();

				SaveBackbufferToFile(pSwapchain, pDevContext);
			}
			break;


			// Hot-reloading of shaders (only modified ones)
			case VK_F9:
				CShaderHotReload::Get()->ReloadShaders(false);	break;

			// Reload all shaders
			case VK_F11: 
				CShaderHotReload::Get()->ReloadShaders(true);	break;
			

			case VK_ESCAPE:
				PostQuitMessage(0);			break;
		}
	}
}

//------------------------------------------------------------------------
HRESULT CBaseApp::OnD3D11CreateDevice(ID3D11Device* pDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	// Shaders
	m_PSDebugView =			CompilePSFromFile( pDevice, "..//Common//DebugView.hlsl", "DebugViewPS" );
	m_PSDebugViewDepth =	CompilePSFromFile( pDevice, "..//Common//DebugView.hlsl", "DebugViewDepthPS" );
	m_pPSDebugViewStencil = CompilePSFromFile( pDevice, "..//Common//DebugView.hlsl", "DebugViewStencilPS" );

	// Constant buffers
	m_CBufferPerFrame.Create(pDevice);
	m_CBufferPerView.Create(pDevice);
	m_CBufferScene.Create(pDevice);
	m_CBufferPostProcess.Create(pDevice);
	m_CBufferPerStaticObject.Create(pDevice);

	// Render states
	States::Claim();

	m_pStates = States::Get();
	m_pStates->CreateResources(pDevice);

	// Systems
	ID3D11DeviceContext* pDevCon = DXUTGetD3D11DeviceContext();

	m_textRenderer.Initialize(pDevice, pDevCon);
	m_fullscreenPass.OnD3D11CreateDevice(pDevice, pBackBufferSurfaceDesc);

	// AntTweakBar (init only)
	TwInit(TW_DIRECT3D11, (void*)pDevice);


	// AntTweakBar for base components of framework
	{
		TwBar* bar;
		bar = TwNewBar("Common");

		// Buttons
		TwAddButton(bar, "Load envmap",  LoadSkyCubemap,  this, "Group=Resources");
		TwAddButton(bar, "Load sphere",	 LoadSkySphere, this, "Group=Resources");
		TwAddVarRW(	bar, "Use envmap", TW_TYPE_BOOLCPP, &m_bUseEnvmap, "Group=Resources");
		TwAddVarRW( bar, "Use CIEClearSky", TW_TYPE_BOOLCPP, &m_bUseCIEClearSky, "Group=Resources");
		TwAddVarRW( bar, "Clouds", TW_TYPE_BOOLCPP, &m_bDrawClouds, "Group=Resources");
		TwAddVarRW( bar, "Use Reversed Depth", TW_TYPE_BOOLCPP, &m_bReversedDepth, "" );

		// HBAO+
		TwAddVarRW(bar, "Enable", TW_TYPE_BOOLCPP, &m_HBAOPlusParams.m_bEnableHBAOPlus, "Group=HBAO+");
		TwAddVarRW(bar, "Normals", TW_TYPE_BOOLCPP, &m_HBAOPlusParams.m_bEnableNormals, "Group=HBAO+");
		TwAddVarRW(bar, "MetersToViewSpace", TW_TYPE_FLOAT, &m_HBAOPlusParams.m_fMetersToViewSpaceUnits, "min=0 max=20 Group=HBAO+");
		TwAddVarRW(bar, "Radius", TW_TYPE_FLOAT, &m_HBAOPlusParams.m_fRadius, "min=0.01 max=8.0 Group=HBAO+");
		TwAddVarRW(bar, "Bias", TW_TYPE_FLOAT, &m_HBAOPlusParams.m_fBias, "min=0.0 max=0.5 group=HBAO+");
		TwAddVarRW(bar, "Power exponent", TW_TYPE_FLOAT, &m_HBAOPlusParams.m_fPowerExponent, "min=0 max=8.0 Group=HBAO+");
		TwAddVarRW(bar, "SmallScaleAO", TW_TYPE_FLOAT, &m_HBAOPlusParams.m_fSmallScaleAO, "min=0.0 max=2.0 group=HBAO+");
		TwAddVarRW(bar, "LargeScaleAO", TW_TYPE_FLOAT, &m_HBAOPlusParams.m_fLargeScaleAO, "min=0.0 max=2.0 group=HBAO+");
		TwAddVarRW(bar, "Enable blur", TW_TYPE_BOOLCPP, &m_HBAOPlusParams.m_bEnableBlur, "group=HBAO+");
		TwAddVarRW(bar, "Blur Sharpness", TW_TYPE_FLOAT, &m_HBAOPlusParams.m_fBlurSharpness, "group=HBAO+ min=0.0 max=100.0");

		// Post processing
		TwAddVarRW(bar, "Vignette Intensity", TW_TYPE_FLOAT, (void*)&m_fVignetteIntensity, "min=0 max=1 step=0.05 group=PostProcessing");
		TwAddVarRW(bar, "Sunlight dir.", TW_TYPE_DIR3F, (void*)&m_lightDir, "opened=true axisz=-z showval=true group=Lights");

		char buffer[1024];
		const unsigned int barWidth = 220;
		const unsigned int barHeight = 350;
		_snprintf_s(buffer, sizeof(buffer),
					"Common label='Common' color='38 38 38' alpha=128 size='%d %d' position='%d %d' ",
					barWidth, barHeight,
					10, 10);

		TwDefine(buffer);
	}

	return S_OK;
}

//------------------------------------------------------------------------
void CBaseApp::OnD3D11DestroyDevice()
{
	// Constant Buffers
	m_CBufferPerFrame.Release();
	m_CBufferPerView.Release();
	m_CBufferScene.Release();
	m_CBufferPostProcess.Release();
	m_CBufferPerStaticObject.Release();

	// Render States
	m_pStates->ReleaseResources();
	States::Destroy();
	m_pStates = nullptr;
	

	// Systems
	m_textRenderer.Release();
	m_fullscreenPass.OnD3D11DestroyDevice();

	CShaderHotReload::Get()->Flush();
	// Shaders
	ShutdownShaders();

	// AntTweakBar
	TwTerminate();
}

//------------------------------------------------------------------------
HRESULT CBaseApp::OnD3D11SwapChainResized(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	m_backBuffer.Width = pBackBufferSurfaceDesc->Width;
	m_backBuffer.Height = pBackBufferSurfaceDesc->Height;

	m_Skybox.Initialize(pd3dDevice);

	m_HBAOPlus.Initialize(pd3dDevice, m_backBuffer.Width, m_backBuffer.Height, &m_HBAOPlusParams);
	
	// Load backbuffer's RTV
	{
		HRESULT hr = E_FAIL;
		ID3D11Texture2D* pBackbuffer = nullptr;

		hr = pSwapChain->GetBuffer(0, __uuidof(*pBackbuffer), (void**)&pBackbuffer);
		V(hr);

		hr = pd3dDevice->CreateRenderTargetView(pBackbuffer, nullptr, &m_backBuffer.RTV);
		V(hr);

		SAFE_RELEASE(pBackbuffer);
	}

	// Debug views (TODO)
	/* ... */

	// AntTweakBar
	TwWindowSize(m_backBuffer.Width, m_backBuffer.Height);


	return S_OK;
}

//------------------------------------------------------------------------
void CBaseApp::OnD3D11SwapChainReleasing()
{
	SAFE_RELEASE(m_backBuffer.RTV);

	m_Skybox.Release();
	m_HBAOPlus.Cleanup();
	m_debugViewMgr.ClearDebugViews();
}

//------------------------------------------------------------------------
uint32 CBaseApp::GetBackbufferWidth() const
{
	return m_backBuffer.Width;
}

//------------------------------------------------------------------------
uint32 CBaseApp::GetBackbufferHeight() const
{
	return m_backBuffer.Height;
}

//------------------------------------------------------------------------
ID3D11RenderTargetView* CBaseApp::GetBackbuffer() const
{
	return m_backBuffer.RTV;
}
