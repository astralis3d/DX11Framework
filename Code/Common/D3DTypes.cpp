#include "PCH.h"
#include "D3DTypes.h"


DepthStencilBuffer::DepthStencilBuffer()
	: m_texture2D(nullptr)
	, m_DSV(nullptr)
	, m_DSVReadOnly(nullptr)
	, m_SRV(nullptr)
	, m_SRVStencil(nullptr)
{
	Width = Height = 0;
}

void DepthStencilBuffer::Initialize(ID3D11Device* pDevice, UINT width, UINT height, DXGI_FORMAT format /*= DXGI_FORMAT_D24_UNORM_S8_UINT*/, bool bAsShaderResource /*= false*/, UINT multiSamples /*= 1*/, UINT msQuality /*= 0*/, UINT arraySize /*= 1*/)
{
	Width = width;
	Height = height;


	UINT BindFlags = D3D11_BIND_DEPTH_STENCIL;
	if (bAsShaderResource == true)
		BindFlags |= D3D11_BIND_SHADER_RESOURCE;

	DXGI_FORMAT dsTexFormat;
	if (!bAsShaderResource)
		dsTexFormat = format;
	else if (format == DXGI_FORMAT_D16_UNORM)
		dsTexFormat = DXGI_FORMAT_R16_TYPELESS;
	else if (format == DXGI_FORMAT_D24_UNORM_S8_UINT)
		dsTexFormat = DXGI_FORMAT_R24G8_TYPELESS;
	else
		dsTexFormat = DXGI_FORMAT_R32_TYPELESS;

	D3D11_TEXTURE2D_DESC Tex2DDesc;
	memset(&Tex2DDesc, 0, sizeof(Tex2DDesc));
	Tex2DDesc.ArraySize = arraySize;
	Tex2DDesc.Width = width;
	Tex2DDesc.Height = height;
	Tex2DDesc.Format = dsTexFormat;
	Tex2DDesc.CPUAccessFlags = 0;
	Tex2DDesc.MipLevels = 1;
	Tex2DDesc.MiscFlags = 0;
	Tex2DDesc.BindFlags = BindFlags;
	Tex2DDesc.SampleDesc.Count = multiSamples;
	Tex2DDesc.SampleDesc.Quality = msQuality;
	Tex2DDesc.Usage = D3D11_USAGE_DEFAULT;

	pDevice->CreateTexture2D(&Tex2DDesc, nullptr, &m_texture2D);

	// Create DSV
	HRESULT hr = E_FAIL;
	m_DSVArraySlices.clear();
	for ( uint32 i=0; i < arraySize; ++i)
	{
		ID3D11DepthStencilView* pArraySliceDSV = nullptr;

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Format = format;
		dsvDesc.Flags = 0;

		if (arraySize == 1)
		{
			dsvDesc.Texture2D.MipSlice = 0;
			dsvDesc.ViewDimension = (multiSamples > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
		}

		else
		{
			dsvDesc.ViewDimension = (multiSamples > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY : D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
			dsvDesc.Texture2DArray.ArraySize = 1;
			dsvDesc.Texture2DArray.FirstArraySlice = i;
			dsvDesc.Texture2DArray.MipSlice = 0;			
		}

		hr = pDevice->CreateDepthStencilView( m_texture2D, &dsvDesc, &pArraySliceDSV );
		V( hr );

		m_DSVArraySlices.push_back( pArraySliceDSV );

		if ( i == 0 )
		{
			// Assign top level array slice DSV
			m_DSV = m_DSVArraySlices[0];

			// Create read-only DSV also
			dsvDesc.Flags = D3D11_DSV_READ_ONLY_DEPTH;
			if (format == DXGI_FORMAT_D24_UNORM_S8_UINT || format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT)
				dsvDesc.Flags |= D3D11_DSV_READ_ONLY_DEPTH;
			hr = pDevice->CreateDepthStencilView( m_texture2D, &dsvDesc, &m_DSVReadOnly );
			V( hr );

			
		}


	}

	// Create SRV if requested for Depth Buffer
	if (bAsShaderResource)
	{
		DXGI_FORMAT dsSRVFormat;
		if (format == DXGI_FORMAT_D16_UNORM)
			dsSRVFormat = DXGI_FORMAT_R16_UNORM;
		else if (format == DXGI_FORMAT_D24_UNORM_S8_UINT)
			dsSRVFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		else
			dsSRVFormat = DXGI_FORMAT_R32_FLOAT;

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = dsSRVFormat;

		if (arraySize == 1)
		{
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.ViewDimension = (multiSamples > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
		}
		else
		{
			srvDesc.ViewDimension = (multiSamples > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY : D3D11_SRV_DIMENSION_TEXTURE2DARRAY;

			srvDesc.Texture2DArray.ArraySize = arraySize;
			srvDesc.Texture2DArray.MipLevels = 1;
			srvDesc.Texture2DArray.MostDetailedMip = 0;
			srvDesc.Texture2DArray.FirstArraySlice = 0;			
		}

		hr = pDevice->CreateShaderResourceView(m_texture2D, &srvDesc, &m_SRV);
		V(hr);


		// Create SRV for each slice separately
		m_SRVArraySlices.clear();
		for (uint32 i = 0; i < arraySize; ++i)
		{
			ID3D11ShaderResourceView* pSliceSRV = nullptr;

			if (arraySize == 1)
			{
				srvDesc.Texture2D.MipLevels = 1;
				srvDesc.Texture2D.MostDetailedMip = 0;
				srvDesc.ViewDimension = (multiSamples > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
			}
			else
			{
				srvDesc.ViewDimension = (multiSamples > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY : D3D11_SRV_DIMENSION_TEXTURE2DARRAY;

				srvDesc.Texture2DArray.ArraySize = 1;
				srvDesc.Texture2DArray.MipLevels = 1;
				srvDesc.Texture2DArray.MostDetailedMip = 0;
				srvDesc.Texture2DArray.FirstArraySlice = i;
			}

			hr = pDevice->CreateShaderResourceView( m_texture2D, &srvDesc, &pSliceSRV );
			V( hr );
			m_SRVArraySlices.push_back( pSliceSRV );
		}
	}
	else
	{
		m_SRV = nullptr;
	}
	

	// Create SRV for Stencil Buffer
	if (format == DXGI_FORMAT_D24_UNORM_S8_UINT)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_X24_TYPELESS_G8_UINT;

		if (arraySize == 1)
		{
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.ViewDimension = (multiSamples > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
		}

		hr = pDevice->CreateShaderResourceView(m_texture2D, &srvDesc, &m_SRVStencil);
		V(hr);
	}
	else
	{
		m_SRVStencil = nullptr;
	}
}

void DepthStencilBuffer::Cleanup()
{
	for ( auto i : m_DSVArraySlices )
	{
		SAFE_RELEASE( i );
	}

	for ( auto i : m_SRVArraySlices )
	{
		SAFE_RELEASE( i );
	}

	SAFE_RELEASE(m_DSVReadOnly);
	SAFE_RELEASE(m_SRV);
	SAFE_RELEASE(m_SRVStencil);
	SAFE_RELEASE(m_texture2D);
}

//----------------------------------------------------------------------
// RenderTarget2D
//----------------------------------------------------------------------

RenderTarget2D::RenderTarget2D()
	: m_Tex2D(nullptr)
	, m_RTV(nullptr)
	, m_SRV(nullptr)
	, m_UAV(nullptr)
{

}

void RenderTarget2D::Initialize(ID3D11Device* pDevice, UINT width, UINT height, DXGI_FORMAT format, bool bUseUAV /*= false*/, bool bGenerateMips /*= false*/, UINT numMipLevels)
{
	Width = width;
	Height = height;

	UINT bindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	if (bUseUAV)
	{
		bindFlags |= D3D11_BIND_UNORDERED_ACCESS;
	}

	UINT miscFlags = 0;
	if (bGenerateMips && numMipLevels != 1)
	{
		miscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
	}

	D3D11_TEXTURE2D_DESC tex2dDesc;
	memset(&tex2dDesc, 0, sizeof(tex2dDesc));

	tex2dDesc.ArraySize = 1;
	tex2dDesc.BindFlags = bindFlags;
	tex2dDesc.Usage = D3D11_USAGE_DEFAULT;
	tex2dDesc.Format = format;
	tex2dDesc.Width = Width;
	tex2dDesc.Height = Height;
	tex2dDesc.MipLevels = numMipLevels;
	tex2dDesc.SampleDesc.Count = 1;
	tex2dDesc.MiscFlags = miscFlags;

	HRESULT hr;
	V( pDevice->CreateTexture2D(&tex2dDesc, nullptr, &m_Tex2D) );

	// Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = tex2dDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	V( pDevice->CreateRenderTargetView(m_Tex2D, &rtvDesc, &m_RTV) );
	
	// Shader Resource View
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = tex2dDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = -1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	V (pDevice->CreateShaderResourceView(m_Tex2D, &srvDesc, &m_SRV) );

	// Unordered Access View
	if (bUseUAV)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = tex2dDesc.Format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;

		V( pDevice->CreateUnorderedAccessView(m_Tex2D, &uavDesc, &m_UAV) );
	}	
}

void RenderTarget2D::Cleanup()
{
	SAFE_RELEASE(m_SRV);
	SAFE_RELEASE(m_RTV);
	SAFE_RELEASE(m_UAV);
	SAFE_RELEASE(m_Tex2D);
}

//----------------------------------------------------------------------
// Shadow Map
//----------------------------------------------------------------------

ShadowMap::ShadowMap()
{

}

void ShadowMap::Initialize(ID3D11Device* pDevice, UINT width, UINT height, DXGI_FORMAT format /*= DXGI_FORMAT_D24_UNORM_S8_UINT*/)
{
	m_width = width;
	m_height = height;

	m_ShadowMapViewport.TopLeftX = 0.f;
	m_ShadowMapViewport.TopLeftY = 0.f;
	m_ShadowMapViewport.Width = static_cast<float>(m_width);
	m_ShadowMapViewport.Height = static_cast<float>(m_height);
	m_ShadowMapViewport.MinDepth = 0.f;
	m_ShadowMapViewport.MaxDepth = 1.f;

	
	DXGI_FORMAT dsTexFormat;
	if (format == DXGI_FORMAT_D16_UNORM)
		dsTexFormat = DXGI_FORMAT_R16_TYPELESS;
	else if (format == DXGI_FORMAT_D24_UNORM_S8_UINT)
		dsTexFormat = DXGI_FORMAT_R24G8_TYPELESS;
	else
		dsTexFormat = DXGI_FORMAT_R32_TYPELESS;

	UINT BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	D3D11_TEXTURE2D_DESC Tex2DDesc;
	memset(&Tex2DDesc, 0, sizeof(Tex2DDesc));
	Tex2DDesc.ArraySize = 1;
	Tex2DDesc.Width = width;
	Tex2DDesc.Height = height;
	Tex2DDesc.Format = dsTexFormat;
	Tex2DDesc.CPUAccessFlags = 0;
	Tex2DDesc.MipLevels = 1;
	Tex2DDesc.MiscFlags = 0;
	Tex2DDesc.BindFlags = BindFlags;
	Tex2DDesc.SampleDesc.Count = 1;
	Tex2DDesc.SampleDesc.Quality = 0;
	Tex2DDesc.Usage = D3D11_USAGE_DEFAULT;

	HRESULT hr = pDevice->CreateTexture2D(&Tex2DDesc, nullptr, &m_tex2D);
	V(hr);

	// Create DSV
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = format;
	dsvDesc.Flags = 0;		
	dsvDesc.Texture2D.MipSlice = 0;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	
	hr = pDevice->CreateDepthStencilView(m_tex2D, &dsvDesc, &m_DSV);
	V(hr);

	// Create SRV if requested

	DXGI_FORMAT dsSRVFormat;
	if (format == DXGI_FORMAT_D16_UNORM)
		dsSRVFormat = DXGI_FORMAT_R16_UNORM;
	else if (format == DXGI_FORMAT_D24_UNORM_S8_UINT)
		dsSRVFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	else
		dsSRVFormat = DXGI_FORMAT_R32_FLOAT;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = dsSRVFormat;

	//if (arraySize == 1)
	{
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		//srvDesc.ViewDimension = (multiSamples > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	}
	/*else
	{
		srvDesc.Texture2DArray.MipLevels = -1;
		srvDesc.Texture2DArray.MostDetailedMip = 0;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.ArraySize = arraySize;

		srvDesc.ViewDimension = (multiSamples > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY : D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	}*/

	hr = pDevice->CreateShaderResourceView(m_tex2D, &srvDesc, &m_SRV);
	V(hr);
}

void ShadowMap::Cleanup()
{
	SAFE_RELEASE(m_SRV);
	SAFE_RELEASE(m_DSV);
	SAFE_RELEASE(m_tex2D);
}

//------------------------------------------------------------------------
// Structured Buffer
//------------------------------------------------------------------------
StructuredBuffer::StructuredBuffer()
	: Size(0)
	, Stride(0)
	, NumElements(0)
	, Buffer(nullptr)
	, SRV(nullptr)
	, UAV(nullptr)
{

}

void StructuredBuffer::Initialize(ID3D11Device* pDevice, uint32 stride, uint32 numElements, bool bUseAsUAV /*= false*/, bool bAppendConsume /*= false*/, bool bHiddenCounter /*= false*/, const void* initData /*= nullptr*/)
{
	Size = stride * numElements;
	Stride = stride;
	NumElements = numElements;

	assert( bAppendConsume == false || bHiddenCounter == false );

	if (bAppendConsume || bHiddenCounter)
		bUseAsUAV = true;

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = stride * numElements;
	bufferDesc.Usage = bUseAsUAV ? D3D11_USAGE_DEFAULT : D3D11_USAGE_IMMUTABLE;
	bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	if (bUseAsUAV)
	{
		bufferDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
	}
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = stride;

	D3D11_SUBRESOURCE_DATA subresourceData;
	subresourceData.pSysMem = initData;
	subresourceData.SysMemPitch = 0;
	subresourceData.SysMemSlicePitch = 0;

	HRESULT hr = E_FAIL;
	V( pDevice->CreateBuffer(&bufferDesc, &subresourceData, &Buffer) );

	// Create SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = numElements;
	V( pDevice->CreateShaderResourceView(Buffer, &srvDesc, &SRV) );

	if (bUseAsUAV)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = numElements;
		uavDesc.Buffer.Flags = 0;
		uavDesc.Buffer.Flags |= bAppendConsume ? D3D11_BUFFER_UAV_FLAG_APPEND : 0;
		uavDesc.Buffer.Flags |= bHiddenCounter ? D3D11_BUFFER_UAV_FLAG_COUNTER : 0;

		V(pDevice->CreateUnorderedAccessView(Buffer, &uavDesc, &UAV));
	}
}

void StructuredBuffer::Cleanup()
{
	SAFE_RELEASE(SRV);
	SAFE_RELEASE(UAV);
	SAFE_RELEASE(Buffer);
}
