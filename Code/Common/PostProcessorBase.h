#ifndef __postprocessorbase_h__
#define __postprocessorbase_h__

#ifdef _MSC_VER
# pragma once
#endif

#include "PCH.h"
#include "D3DTypes.h"
#include "D3DConstantBuffer.h"
#include "D3DShaders.h"

struct TempRenderTarget
{
	ID3D11Texture2D*	Texture;
	ID3D11RenderTargetView* RTView;
	ID3D11ShaderResourceView* SRView;
	ID3D11UnorderedAccessView* UAView;
	UINT Width;
	UINT Height;
	DXGI_FORMAT Format;
	UINT MSCount;
	UINT MSQuality;
	bool bInUse;
};

class PostProcessorBase
{
public:
	PostProcessorBase();
	virtual ~PostProcessorBase();

	virtual void Initialize(ID3D11Device* pDevice);
	virtual void Render(ID3D11DeviceContext* pDevContext, ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output);
	virtual void AfterReset(UINT width, UINT height);

	virtual void Release();
	
protected:
	static const int MAX_INPUTS = 4;

	struct PSConstants
	{
		Vec2	InputSize[MAX_INPUTS];	// 8 * 4 = 32 bytes (32)
		Vec2	OutputSize;				// 2 * 4 = 8 bytes  (40)
		Vec2	pad;					// 2 * 4 = 8 bytes  (48)

	};


	TempRenderTarget* GetTempRenderTarget(UINT width, UINT height, DXGI_FORMAT format, UINT msCount = 1, UINT msQuality = 0,
										  UINT mipLevels = 	1, bool generateMipMaps = false, bool useAsUAV = false);

	void ClearTempRenderTargetCache();

	void PostProcess(ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output, ID3D11PixelShader* pixelShader, const wchar_t* name);
	virtual void PostProcess(ID3D11PixelShader* pixelShader, const wchar_t* name);

	ID3D11Device*			m_pDevice;
	ID3D11DeviceContext*	m_pDevContext;

	// Full screen triangle
	TVertexShader			pFullScreenVS;


	std::vector<TempRenderTarget*>	tempRenderTargets;

	std::vector<ID3D11ShaderResourceView*>	inputs;
	std::vector<ID3D11RenderTargetView*>	outputs;
	std::vector<ID3D11UnorderedAccessView*> uaViews;

	D3DConstantBuffer<PSConstants>			m_constants;

	UINT inputWidth;
	UINT inputHeight;
};

#endif