#include "PCH.h"
#include "PostProcessorBase.h"

PostProcessorBase::PostProcessorBase()
	: m_pDevice(nullptr)
	, m_pDevContext(nullptr)
{

}

PostProcessorBase::~PostProcessorBase()
{
	ClearTempRenderTargetCache();
}

void PostProcessorBase::Initialize(ID3D11Device* device)
{
	m_pDevice = device;

	m_constants.Create(device);
	pFullScreenVS.reset(new CVertexShader(m_pDevice, "..//Common//PostProcessCommon.hlsl", "QuadVS"));
}

void PostProcessorBase::Render(ID3D11DeviceContext* pDevCon, ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output)
{
	m_pDevContext = pDevCon;

	m_pDevContext->IASetInputLayout(nullptr);
	m_pDevContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	m_pDevContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	pFullScreenVS->Bind(m_pDevContext);
	m_pDevContext->GSSetShader(nullptr, 0, 0);
	m_pDevContext->DSSetShader(nullptr, 0, 0);
	m_pDevContext->HSSetShader(nullptr, 0, 0);
}

void PostProcessorBase::AfterReset(UINT width, UINT height)
{
	ClearTempRenderTargetCache();

	inputWidth = width;
	inputHeight = height;
}

TempRenderTarget* PostProcessorBase::GetTempRenderTarget(UINT width, UINT height, DXGI_FORMAT format, UINT msCount /*= 1*/, UINT msQuality /*= 0*/, UINT mipLevels /*= 1*/, bool generateMipMaps /*= false*/, bool useAsUAV /*= false*/)
{
	// Look through existing render targets
	for (size_t i = 0; i < tempRenderTargets.size(); ++i)
	{
		TempRenderTarget* rt = tempRenderTargets[i];
		if (!rt->bInUse && rt->Width == width && rt->Height == height && rt->Format == format
			&& rt->MSCount == msCount && rt->MSQuality == msQuality && (rt->UAView != nullptr) == useAsUAV)
		{
			rt->bInUse = true;
			return rt;
		}
	}

	// Didn't find one, have to make one
	TempRenderTarget* rt = new TempRenderTarget();
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = width;
	desc.Height = height;
	desc.ArraySize = 1;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	if (useAsUAV)
		desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
	desc.CPUAccessFlags = 0;
	desc.Format = format;
	desc.MipLevels = mipLevels;
	desc.MiscFlags = generateMipMaps ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
	desc.SampleDesc.Count = msCount;
	desc.SampleDesc.Quality = msQuality;
	desc.Usage = D3D11_USAGE_DEFAULT;

	HRESULT hr = E_FAIL;
	V(m_pDevice->CreateTexture2D(&desc, nullptr, &rt->Texture));
	V(m_pDevice->CreateRenderTargetView(rt->Texture, nullptr, &rt->RTView));
	V(m_pDevice->CreateShaderResourceView(rt->Texture, nullptr, &rt->SRView));

	if (useAsUAV)
		m_pDevice->CreateUnorderedAccessView(rt->Texture, nullptr, &rt->UAView);
	else
		rt->UAView = nullptr;

	rt->Width = width;
	rt->Height = height;
	rt->MSCount = msCount;
	rt->MSQuality = msQuality;
	rt->Format = format;
	rt->bInUse = true;
	tempRenderTargets.push_back(rt);

	return tempRenderTargets[tempRenderTargets.size() - 1];
}

void PostProcessorBase::ClearTempRenderTargetCache()
{
	for (size_t i=0; i < tempRenderTargets.size(); ++i)
	{
		SAFE_RELEASE( tempRenderTargets[i]->Texture);
		SAFE_RELEASE( tempRenderTargets[i]->RTView );
		SAFE_RELEASE( tempRenderTargets[i]->SRView );
		SAFE_RELEASE( tempRenderTargets[i]->UAView );		

		SAFE_DELETE( tempRenderTargets[i] );
	}

	tempRenderTargets.erase(tempRenderTargets.begin(), tempRenderTargets.end());
}

void PostProcessorBase::PostProcess(ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output, ID3D11PixelShader* pixelShader, const wchar_t* name)
{
	inputs.push_back(input);
	outputs.push_back(output);

	PostProcess(pixelShader, name);
}

void PostProcessorBase::PostProcess(ID3D11PixelShader* pixelShader, const wchar_t* name)
{
	_ASSERT( m_pDevContext );

	PIXEvent pixEvent(name);

	// Set the outputs
	ID3D11RenderTargetView** rtViews = reinterpret_cast<ID3D11RenderTargetView**>( &outputs[0] );
	UINT numRTViews = (UINT) outputs.size();
	
	if (uaViews.size() == 0)
		m_pDevContext->OMSetRenderTargets(numRTViews, rtViews, nullptr);
	else
	{
		// ...
	}

	// Set the input textures.
	ID3D11ShaderResourceView** srViews = reinterpret_cast<ID3D11ShaderResourceView**>( &inputs[0]);
	UINT numSRViews = (UINT) inputs.size();

	m_pDevContext->PSSetShaderResources(0, numSRViews, srViews);

	// Set contant buffer
	auto& constants = m_constants.GetBufferData();

	for (size_t i = 0; i < inputs.size(); ++i)
	{
		if (inputs[i] == nullptr)
		{
			constants.InputSize[i].x = 0.0f;
			constants.InputSize[i].y = 0.0f;
			continue;
		}

		ID3D11Texture2D* texture;
		D3D11_TEXTURE2D_DESC desc;
		D3D11_SHADER_RESOURCE_VIEW_DESC srDesc;
		inputs[i]->GetDesc(&srDesc);

		UINT mipLevel = srDesc.Texture2D.MostDetailedMip;
		inputs[i]->GetResource((ID3D11Resource**)&texture);
		
		texture->GetDesc(&desc);

		constants.InputSize[i].x = static_cast<float>(std::max<UINT>(desc.Width / (1 << mipLevel), 1));
		constants.InputSize[i].y = static_cast<float>(std::max<UINT>(desc.Height / (1 << mipLevel), 1));

		SAFE_RELEASE(texture);
	}


	ID3D11Texture2D* pTexture;
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	D3D11_TEXTURE2D_DESC tex2DDesc;
	outputs[0]->GetResource( (ID3D11Resource**) &pTexture);
	outputs[0]->GetDesc(&rtvDesc);
	
	UINT mipLevel = rtvDesc.Texture2D.MipSlice;

	pTexture->GetDesc(&tex2DDesc);

	constants.OutputSize.x = static_cast<float>(std::max<UINT>(tex2DDesc.Width / (1 << mipLevel), 1));
	constants.OutputSize.y = static_cast<float>(std::max<UINT>(tex2DDesc.Height / (1 << mipLevel), 1));

	m_constants.UpdateBuffer(m_pDevContext);
	m_constants.SetPS(m_pDevContext, 6);



	D3D11_VIEWPORT viewports[16];
	for (UINT i=0; i < 16; ++i)
	{
		viewports[i].Width = static_cast<float>( std::max<UINT>(tex2DDesc.Width / (1 << mipLevel), 1) );
		viewports[i].Height = static_cast<float>(std::max<UINT>(tex2DDesc.Height / (1 << mipLevel), 1));
		viewports[i].TopLeftX = 0;
		viewports[i].TopLeftY = 0;
		viewports[i].MinDepth = 0.0f;
		viewports[i].MaxDepth = 1.0f;
	}

	m_pDevContext->RSSetViewports(static_cast<UINT>(outputs.size()), viewports);

	m_pDevContext->PSSetShader(pixelShader, 0, 0);

	m_pDevContext->Draw(3, 0);

	// Clear RTVs and SRVs
	ID3D11ShaderResourceView* nullSRVs[16] = { nullptr };
	m_pDevContext->PSSetShaderResources(0, static_cast<UINT>( inputs.size() ), nullSRVs);

	ID3D11RenderTargetView* nullRTVs[16] = { nullptr };
	m_pDevContext->OMSetRenderTargets(static_cast<UINT>( outputs.size() + uaViews.size() ), nullRTVs, nullptr);

	inputs.clear();
	outputs.clear();
	uaViews.clear();

	SAFE_RELEASE(pTexture);
}

void PostProcessorBase::Release()
{
	pFullScreenVS.reset();

	m_constants.Release();
}
