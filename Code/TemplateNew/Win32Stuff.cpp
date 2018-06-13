//------------------------------------------------------------------------
// 
// File : Win32Stuff.cpp
//
// WindowProcedure, Keyboard, etc...
//
//------------------------------------------------------------------------

#include "PCH.h"
#include "MyApp.h"
#include "AntTweakBar.h"
#include "ShaderHotReload.h"

//------------------------------------------------------------------------
void CMyApp::OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown)
{
	CBaseApp::OnKeyboard(nChar, bKeyDown, bAltDown);

	if (bKeyDown)
	{
		switch (nChar)
		{
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			{
				m_debugSurface = EDebugSurface::NormalRender;

				m_pDebugSRV = m_debugViewMgr.GetDebugSurface((char)nChar);


				if (m_pDebugSRV)
				{
					m_bDebugView = true;
					m_debugViewDesc = m_debugViewMgr.GetDebugViewDesc((char)nChar);
				}
				else
					m_bDebugView = false;

				//g_debugSurface = eDebugSurface::Unknown;
			}

				break;

			// Reset camera
			case 'R':
			{
				//g_pCameraFree->ResetCamera(-7.f, 20.f, 0.f, g_pCameraFree->GetYaw(), g_pCameraFree->GetPitch());

				//UpdateViewConstantBuffer(DXUTGetD3D11DeviceContext());
			}
			break;		

		#ifdef USE_RENDERDOC
			case 'H':
			{
				auto pRenderDocAPI = m_renderDoc.GetRenderDocAPI();
				if (pRenderDocAPI)
				{
					const uint32_t renderDocMaskOverlayBits = pRenderDocAPI->GetOverlayBits();
					pRenderDocAPI->MaskOverlayBits( ~renderDocMaskOverlayBits, ~renderDocMaskOverlayBits );
				}
			}
			break;

			// Opens RenderDoc directly with the latest capture.
			// We assume here that .rdc files are associated with RenderDoc
			case VK_F3:
			{
				auto pRenderDocAPI = m_renderDoc.GetRenderDocAPI();
				if (pRenderDocAPI)
				{
					const uint32_t numCaptures = pRenderDocAPI->GetNumCaptures();
					if (numCaptures > 0)
					{
						char LogFile[512];
						uint64_t timestamp;
						uint32_t LogPathLength = 512;
						pRenderDocAPI->GetCapture( numCaptures - 1, LogFile, &LogPathLength, &timestamp );

						ShellExecuteA( 0, 0, LogFile, 0, 0, SW_SHOW );
					}
					else
					{
						DebugPrintf( "[INFO] There are no RenderDoc captures yet.\n" );
					}
				}
			}
			break;


		#endif // USE_RENDERDOC
		}
	}
}