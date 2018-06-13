#include "PCH.h"
#include "MyApp.h"
#include "AntTweakBar.h"

const float CMyApp::NEAR_PLANE = 0.2f;
const float CMyApp::FAR_PLANE =  5000.f;


CMyApp::CMyApp()
	: CBaseApp()
{
	// For this sample, use CIE ClearSky sky model
	m_bUseCIEClearSky = true;

	// The Witcher 3 "Toussaint" tonemapping params
	m_toneMappingParams.CurveABCD.x = 0.98506f;	// A
	m_toneMappingParams.CurveABCD.y = 1.03957f; // B
	m_toneMappingParams.CurveABCD.z = 1.03862f; // C
	m_toneMappingParams.CurveABCD.w = 3.04301f; // D
	m_toneMappingParams.CurveEF.x = 0.025f; // E
	m_toneMappingParams.CurveEF.y = 1.04592f; // F

	m_toneMappingParams.MinMaxAllowedSceneLuminance.x = 0.0f;
	m_toneMappingParams.powParam = 0.45f;
	m_toneMappingParams.WhitePoint = 1.03f;
	m_toneMappingParams.NominatorMult = 0.8912f;
	

#ifdef USE_RENDERDOC
	m_renderDoc.Initialize();
#endif
}

//------------------------------------------------------------------------
CMyApp::~CMyApp()
{
#ifdef USE_RENDERDOC
	m_renderDoc.Release();
#endif
}



//------------------------------------------------------------------------
bool CMyApp::ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings)
{
#if defined (_DEBUG) | defined (DEBUG)
	pDeviceSettings->d3d11.CreateFlags |= D3D11_CREATE_DEVICE_DEBUG;	
#endif

	pDeviceSettings->d3d11.sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// VSync on (1) / off (0)
	// TODO: Change in realtime?

#if (VSYNC == 1)
	pDeviceSettings->d3d11.SyncInterval = 1;
#endif

	// Don't auto create depth stencil - we use our own, because depth stencil buffer
	// can be bindable as SRV this way.
	pDeviceSettings->d3d11.AutoCreateDepthStencil = false;

	return true;
}

//------------------------------------------------------------------------
int CMyApp::run()
{
	CBaseApp::run();

	// Create free camera
	m_pCameraFree = new CCameraFree(-54.85f, 117.82f, -40.54f, 250.0f, 1.25f, DegToRad(0.0), DegToRad(0.0));

	m_pCameraFree->SetHWND(DXUTGetHWND());
	m_pCameraFree->SetFOV(65.0f);
	
	// Create D3D11 device
	const float dWidth =  static_cast<float>( GetSystemMetrics( SM_CXSCREEN ) );
	const float dHeight = static_cast<float>( GetSystemMetrics( SM_CYSCREEN ) );
	
	const int nWidth =  static_cast<int>(dWidth * 0.8);
	const int nHeight = static_cast<int>(dHeight * 0.8);

	{
		RECT rect;
		SetRect( &rect, 0, 0, nWidth, nHeight );

		LONG style = GetWindowLong( DXUTGetHWND(), GWL_STYLE );
		LONG styleEx = GetWindowLong( DXUTGetHWND(), GWL_EXSTYLE );
		AdjustWindowRectEx( &rect, style, FALSE, styleEx );

		DXUTCreateDevice( D3D_FEATURE_LEVEL_11_0, true, rect.right - rect.left, rect.bottom - rect.top );
		//DXUTCreateDevice( D3D_FEATURE_LEVEL_11_0, true, 1600, 900 );
	}

	

	// Enter in the main DXUT loop
	DXUTMainLoop();

	// We have to call these ones manually, can't rely only on polymorphism
	OnD3D11SwapChainReleasing();
	OnD3D11DestroyDevice();

		

	int nReturnCode =  DXUTGetExitCode();
	return nReturnCode;
}

//------------------------------------------------------------------------
void CMyApp::OnFrameMove(double fTime, float fElapsedTime)
{
	CBaseApp::OnFrameMove(fTime, fElapsedTime);

	// Inverted depth requires swapping near/far planes.
	if (m_bReversedDepth)
	{
		m_pCameraFree->SetClippingDistances( FAR_PLANE, NEAR_PLANE );
	}
	else
	{
		m_pCameraFree->SetClippingDistances( NEAR_PLANE, FAR_PLANE );
	}


	m_pCameraFree->HandleInput( &m_Input, m_timer.DeltaSeconds() );
}

//------------------------------------------------------------------------
void CMyApp::GatherDisplayDebugInfo()
{
	static float fElapsedTime = 0.0f;
	fElapsedTime += m_timer.DeltaSeconds();

	if (fElapsedTime >= 0.5f)
	{
		float fCamYaw =   RadToDeg( m_pCameraFree->GetYaw() );
		float fCamPitch = RadToDeg( m_pCameraFree->GetPitch() );
		fCamPitch = fmodf(fCamPitch, 360.0f);
		fCamYaw = fmod(fCamYaw, 360.0f);


		float fClipNear, fClipFar;
		m_pCameraFree->GetClippingDistances(fClipNear, fClipFar);
				
		sprintf_s(m_szRenderBuffer,
				  "Vertices: %d\nTriangles:%d\nCamPos: %.2f, %.2f, %.2f\nFOV: %.1f, CamYaw: %.1f, CamPitch: %.1f\nCamSpeed: %.1f, near: %.1f, far: %1.f",
				  m_Scene.GetNumVertices(), m_Scene.GetNumTriangles(),
				  m_pCameraFree->GetPosition().x, m_pCameraFree->GetPosition().y, m_pCameraFree->GetPosition().z,
				  m_pCameraFree->GetFOV(), fCamYaw, fCamPitch, m_pCameraFree->GetSpeedMove(), fClipNear, fClipFar);


		fElapsedTime = 0.0f;
	}
}

//------------------------------------------------------------------------
void CMyApp::UpdateViewConstantBuffer(ID3D11DeviceContext* pDevCon)
{
	/*
	if (!m_pCameraFree->IsChanged())
	return;
	*/

	PIXEvent evt(L"View CBuffer update");

	//const Mat44& view = m_pCameraFree->GetMatrixView();
	const Mat44& proj = m_pCameraFree->GetMatrixProj();
	const Mat44& viewProj = m_pCameraFree->GetMatrixViewProj();

	Mat44 invViewProj;
	MatrixInverse(viewProj, &invViewProj);

	// Calculate useful viewport-based matrix for determining WorldPos from depth
	// during reading from GBuffer
	Mat44 invViewProjViewport;

	const float Width = static_cast<float>(m_RenderTargetColor.Width);
	const float Height = static_cast<float>(m_RenderTargetColor.Height);

	{
		Mat44 f4x4Viewport(2.0f / Width, 0.0f, 0.0f, 0.0f,
						   0.0f, -2.0f / Height, 0.0f, 0.0f,
						   0.0f, 0.0f, 1.0f, 0.0f,
						   -1.0f, 1.0f, 0.0f, 1.0f);

		MatrixMult(f4x4Viewport, invViewProj, &invViewProjViewport);
	}

	Mat44 mProjInv;
	MatrixInverse(proj, &mProjInv);

	auto& data = m_CBufferPerView.GetBufferData();
	data.mView = m_pCameraFree->GetMatrixView();
	data.mProj = m_pCameraFree->GetMatrixProj();
	data.mProjInv = mProjInv;
	data.mViewProj = m_pCameraFree->GetMatrixViewProj();
	data.mViewProjInv = invViewProj;
	data.mViewProjInvViewport = invViewProjViewport;
	data.vCamPos = m_pCameraFree->GetPosition();
	data.vViewport = Vec4(Width, Height, 1.f / Width, 1.f / Height);
	data.zNear = NEAR_PLANE;
	data.zFar = FAR_PLANE;

	if (m_bReversedDepth)
	{
		data.depthScaleFactors.x = -1.0f;
		data.depthScaleFactors.y = 1.0f;
	}
	else
	{
		data.depthScaleFactors.x = 1.0f;
		data.depthScaleFactors.y = 0.0f;
	}
	
	// farPlaneScale = far plane / 1000
	data.cameraNearFar.y = FAR_PLANE / 1000.0f;

	// nearPlaneScale = -farPlaneScale + ( ( nearPlane * farPlaneScale ) / FAR_PLANE )
	data.cameraNearFar.x = -data.cameraNearFar.y + ( ( NEAR_PLANE * data.cameraNearFar.y) / FAR_PLANE );

	m_CBufferPerView.UpdateBuffer(pDevCon);
	m_CBufferPerView.SetVS(pDevCon, 1);
	m_CBufferPerView.SetPS(pDevCon, 1);

	if (m_bDefrredShadingUseComputeShader)
	{
		m_CBufferPerView.SetCS(pDevCon, 1);
	}
}
//------------------------------------------------------------------------
// not implemented
//------------------------------------------------------------------------
void TW_CALL ExportMesh(void* clientData)
{
	char szEXEPath[MAX_PATH];
	GetModuleFileNameA( NULL, szEXEPath, _MAX_PATH );

	// Find path to "Media" directory
	const int MAX_DIRS_UP = 5;

	std::string::size_type pos = std::string( szEXEPath ).find_last_of( "\\/" );
	std::string path = std::string( szEXEPath ).substr( 0, pos );

	bool bSuccess = false;

	int dirsUp = 0;
	do
	{
		path.append( "\\Media\\Meshes\\" );

		DWORD dwAttrib = GetFileAttributesA( path.c_str() );
		if ((dwAttrib != INVALID_FILE_ATTRIBUTES) && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
		{
			bSuccess = true;
			break;
		}
		else
		{
			path = path.substr( 0, pos );
			pos = path.find_last_of( "\\/" );
			path = path.substr( 0, pos );
		}
	} while (dirsUp++ < MAX_DIRS_UP);

	if (bSuccess)
	{
		// GetOpenFileName/GetSaveFileName changes the CWD on success. Copy it for now.
		char szCWD[MAX_PATH];
		GetCurrentDirectoryA( MAX_PATH, szCWD );


		char szFinalPath[MAX_PATH] = "";

		OPENFILENAMEA ofn;
		memset( &ofn, 0, sizeof( OPENFILENAME ) );

		ofn.lStructSize = sizeof( OPENFILENAME );
		ofn.hwndOwner = DXUTGetHWND();
		ofn.lpstrFilter = "All files\0*.*\0";
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrInitialDir = path.c_str();
		ofn.lpstrFile = szFinalPath;
		ofn.nMaxFile = MAX_PATH;

		ofn.Flags = OFN_HIDEREADONLY | OFN_ENABLESIZING | OFN_FILEMUSTEXIST | OFN_EXPLORER;

		bSuccess = !!::GetSaveFileNameA( &ofn );

		if (bSuccess)
		{
			// Restore CWD
			SetCurrentDirectoryA( szCWD );

			CMyApp* pApp = static_cast<CMyApp*>(clientData);

			//pApp->m_Scene.
		}
	}

}

//------------------------------------------------------------------------
void TW_CALL LoadMesh(void* clientData)
{
	char szEXEPath[MAX_PATH];
	GetModuleFileNameA(NULL, szEXEPath, _MAX_PATH);

	// Find path to "Media" directory
	const int MAX_DIRS_UP = 5;

	std::string::size_type pos = std::string(szEXEPath).find_last_of("\\/");
	std::string path = std::string(szEXEPath).substr(0, pos);

	bool bSuccess = false;

	int dirsUp = 0;
	do
	{
		path.append("\\Media\\Meshes\\");
		
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
		// GetOpenFileName/GetSaveFileName changes the CWD on success. Copy it for now.
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

			CMyApp* pApp = static_cast<CMyApp*>(clientData);
			pApp->m_Scene.SaveScaleFactorToRegistry();
			pApp->m_Scene.SetupMesh(szFinalPath);
		}
	}
}