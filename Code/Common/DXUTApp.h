#pragma once

class CDXUTApp11
{
public:
	CDXUTApp11();
	virtual ~CDXUTApp11();

	virtual int run();

	virtual bool ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings) { return true; }
	virtual void OnFrameMove(double fTime, float fElapsedTime) {}
	virtual LRESULT WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool* pBNoFurtherProcessing) { return 0; }
	virtual void OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown) {}

	virtual bool IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo* adapterInfo, UINT Output, const CD3D11EnumDeviceInfo* deviceInfo,
										 DXGI_FORMAT backBufferFormat, bool windowed) {
		return true;
	}

	virtual HRESULT OnD3D11SwapChainResized(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
											const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc) {
		return S_OK;
	}
	virtual void OnD3D11SwapChainReleasing() { }

	virtual HRESULT OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc) { return S_OK; }
	virtual void OnD3D11DestroyDevice() {}

	virtual void OnD3D11RenderFrame(ID3D11Device* pDevice, ID3D11DeviceContext* pDevCon, double fTime, float fElapsedTime) {}


private:
	static LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, bool* pBNoFurtherProcessing, void* pUserContext);

	static bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext);
	static void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext);
	static void CALLBACK OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext);

	static bool CALLBACK IsD3D11DeviceAcceptable(const CD3D11EnumAdapterInfo *AdapterInfo, UINT Output, const CD3D11EnumDeviceInfo *DeviceInfo,
												 DXGI_FORMAT BackBufferFormat, bool bWindowed, void* pUserContext);
	static HRESULT CALLBACK OnD3D11SwapChainResized(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
													const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
	static void CALLBACK OnD3D11SwapChainReleasing(void* pUserContext);

	static HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
	static void CALLBACK OnD3D11DestroyDevice(void* pUserContext);

	static void CALLBACK OnD3D11RenderFrame(ID3D11Device* pDevice, ID3D11DeviceContext* pDevCon, double fTime, float fElapsedTime, void* pUserContext);
};