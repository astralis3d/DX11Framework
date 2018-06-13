#include "PCH.h"
#include "DXUTApp.h"


CDXUTApp11::CDXUTApp11()
{

}

//------------------------------------------------------------------------
CDXUTApp11::~CDXUTApp11()
{
	DXUTSetCallbackDeviceChanging(nullptr);
	DXUTSetCallbackMsgProc(nullptr);
	DXUTSetCallbackKeyboard(nullptr);
	DXUTSetCallbackFrameMove(nullptr);

	DXUTSetCallbackD3D11DeviceAcceptable(nullptr);
	DXUTSetCallbackD3D11DeviceCreated(nullptr);
	DXUTSetCallbackD3D11SwapChainResized(nullptr);
	DXUTSetCallbackD3D11FrameRender(nullptr);
	DXUTSetCallbackD3D11SwapChainReleasing(nullptr);
	DXUTSetCallbackD3D11DeviceDestroyed(nullptr);
}

//------------------------------------------------------------------------
int CDXUTApp11::run()
{
	/* Base operations for DXUT11 */

	auto self = static_cast<void*>(this);

	// General DXUT callbacks
	DXUTSetCallbackFrameMove(OnFrameMove, self);
	DXUTSetCallbackMsgProc(WindowProcedure, self);
	DXUTSetCallbackDeviceChanging(ModifyDeviceSettings, self);

	// Set the D3D11 DXUT callbacks
	DXUTSetCallbackD3D11DeviceAcceptable(IsD3D11DeviceAcceptable, self);
	DXUTSetCallbackD3D11DeviceCreated(OnD3D11CreateDevice, self);
	DXUTSetCallbackD3D11SwapChainResized(OnD3D11SwapChainResized, self);
	DXUTSetCallbackD3D11SwapChainReleasing(OnD3D11SwapChainReleasing, self);
	DXUTSetCallbackD3D11DeviceDestroyed(OnD3D11DestroyDevice, self);
	DXUTSetCallbackD3D11FrameRender(OnD3D11RenderFrame, self);

	DXUTSetIsInGammaCorrectMode(false);

	DXUTInit(true, true, nullptr);
	DXUTSetCursorSettings(true, true);
	DXUTCreateWindow(L"DX11 Framework");

	const LONG style = WS_POPUPWINDOW;
	//SetWindowLong(DXUTGetHWND(), GWL_STYLE, style);

	return 0;
}

//------------------------------------------------------------------------
LRESULT CDXUTApp11::WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool* pBNoFurtherProcessing, void* pUserContext)
{
	auto self = static_cast<CDXUTApp11*>(pUserContext);
	return self->WindowProcedure(hWnd, msg, wParam, lParam, pBNoFurtherProcessing);
}

//------------------------------------------------------------------------
bool CDXUTApp11::ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext)
{
	auto self = static_cast<CDXUTApp11*>(pUserContext);
	return self->ModifyDeviceSettings(pDeviceSettings);
}

//------------------------------------------------------------------------
void CDXUTApp11::OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
{
	auto self = static_cast<CDXUTApp11*>(pUserContext);
	self->OnFrameMove(fTime, fElapsedTime);
}

//------------------------------------------------------------------------
void CDXUTApp11::OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext)
{
	auto self = static_cast<CDXUTApp11*>(pUserContext);
	self->OnKeyboard(nChar, bKeyDown, bAltDown);
}

//------------------------------------------------------------------------
bool CDXUTApp11::IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo, DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext)
{
	auto self = static_cast<CDXUTApp11*>(pUserContext);
	return self->IsD3D11DeviceAcceptable(AdapterInfo, Output, DeviceInfo, BackBufferFormat, bWindowed);
}

//------------------------------------------------------------------------
HRESULT CDXUTApp11::OnD3D11SwapChainResized(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	auto self = static_cast<CDXUTApp11*>(pUserContext);
	return self->OnD3D11SwapChainResized(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc);
}

//------------------------------------------------------------------------
void CDXUTApp11::OnD3D11SwapChainReleasing(void* pUserContext)
{
	auto self = static_cast<CDXUTApp11*>(pUserContext);
	self->OnD3D11SwapChainReleasing();
}

//------------------------------------------------------------------------
HRESULT CDXUTApp11::OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
	auto self = static_cast<CDXUTApp11*>(pUserContext);
	return self->OnD3D11CreateDevice(pd3dDevice, pBackBufferSurfaceDesc);
}

//------------------------------------------------------------------------
void CDXUTApp11::OnD3D11DestroyDevice(void* pUserContext)
{
	auto self = static_cast<CDXUTApp11*>(pUserContext);
	self->OnD3D11DestroyDevice();
}

//------------------------------------------------------------------------
void CDXUTApp11::OnD3D11RenderFrame(ID3D11Device* pDevice, ID3D11DeviceContext* pDevCon, double fTime, float fElapsedTime, void* pUserContext)
{
	auto self = static_cast<CDXUTApp11*>(pUserContext);
	self->OnD3D11RenderFrame(pDevice, pDevCon, fTime, fElapsedTime);
}