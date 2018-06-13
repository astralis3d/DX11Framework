#ifndef __d3dtypes_h__
#define __d3dtypes_h__

#ifdef _MSC_VER
# pragma once
#endif

#include <DXUT.h>
#include <DXUTmisc.h>
#include "Defines.h"

struct DepthStencilBuffer
{
	typedef std::vector<ID3D11DepthStencilView*>	TVecDSV;
	typedef std::vector<ID3D11ShaderResourceView*>	TVecSRV;

	DepthStencilBuffer();

	void Initialize(ID3D11Device* pDevice,
					UINT width,
					UINT height,
					DXGI_FORMAT format = DXGI_FORMAT_D24_UNORM_S8_UINT,
					bool bAsShaderResource = false,
					UINT multiSamples = 1,
					UINT msQuality = 0,
					UINT arraySize = 1);

	void Cleanup();

	operator ID3D11ShaderResourceView*() const	{ return m_SRV; }
	operator ID3D11DepthStencilView*() const	{ return m_DSVArraySlices[0]; }

	ID3D11Texture2D* m_texture2D;
	ID3D11DepthStencilView* m_DSV;
	TVecDSV					m_DSVArraySlices;
	ID3D11DepthStencilView* m_DSVReadOnly;
	ID3D11ShaderResourceView* m_SRV;
	TVecSRV						m_SRVArraySlices;
	ID3D11ShaderResourceView* m_SRVStencil;

	UINT Width;
	UINT Height;	
};

//------------------------------------------------------------------------
struct RenderTarget2D
{
	RenderTarget2D();

	void Initialize(ID3D11Device* pDevice,
					UINT width,
					UINT height,
					DXGI_FORMAT format,
					bool bUseUAV = false,
					bool bGenerateMips = false,
					UINT numMipLevels = 1);

	void Cleanup();

	operator ID3D11RenderTargetView*()		const	{ return m_RTV; }
	operator ID3D11ShaderResourceView*()	const	{ return m_SRV;	}
	operator ID3D11UnorderedAccessView*()	const	{ return m_UAV; }


	ID3D11Texture2D*			m_Tex2D;
	ID3D11RenderTargetView*		m_RTV;
	ID3D11ShaderResourceView*	m_SRV;
	ID3D11UnorderedAccessView*	m_UAV;

	UINT Width;
	UINT Height;
};


//------------------------------------------------------------------------
struct StructuredBuffer
{
	StructuredBuffer();

	void Initialize(ID3D11Device* pDevice, uint32 stride, uint32 numElements, bool bUseAsUAV = false,
					bool bAppendConsume = false, bool bHiddenCounter = false, const void* initData = nullptr);

	void Cleanup();

	ID3D11Buffer*				Buffer;
	ID3D11ShaderResourceView*	SRV;
	ID3D11UnorderedAccessView*	UAV;

	uint32						Size;
	uint32						Stride;
	uint32						NumElements;
};




//------------------------------------------------------------------------
struct ShadowMap
{
	ShadowMap();

	void Initialize(ID3D11Device* pDevice,
					UINT width,
					UINT height,
					DXGI_FORMAT format = DXGI_FORMAT_D24_UNORM_S8_UINT);

	void Cleanup();

	ID3D11Texture2D*			m_tex2D;
	ID3D11DepthStencilView*		m_DSV;
	ID3D11ShaderResourceView*	m_SRV;

	D3D11_VIEWPORT				m_ShadowMapViewport;

	UINT						m_width;
	UINT						m_height;
};

//------------------------------------------------------------------------
class PIXEvent
{
public:
	PIXEvent(const wchar_t* markerName)
	{
		DXUT_Dynamic_D3DPERF_BeginEvent(0xFFFFFFFF, markerName);
	}

	~PIXEvent()
	{
		DXUT_Dynamic_D3DPERF_EndEvent();
	}
};

//------------------------------------------------------------------------
class PIXEventEx
{
public:
	PIXEventEx(const wchar_t* markerName) : m_pMarkerName(markerName) {}
	~PIXEventEx() {}

	void Begin()
	{
		DXUT_Dynamic_D3DPERF_BeginEvent(0xFFFFFFFF, m_pMarkerName);
	}


	void End()
	{
		DXUT_Dynamic_D3DPERF_EndEvent();
	}

private:
	const wchar_t*	m_pMarkerName;

};

#endif