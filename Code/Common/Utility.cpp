#include "PCH.h"
#include "Utility.h"
#include "D3DStates.h"
#include <Shlwapi.h>
#include <ScreenGrab.h>

// Based on "Game Engine Architecture" by J. Gregory, CRC Press 2014
int VDebugPrintf(const char* format, va_list args)
{
	const unsigned int MAX_CHARS = 8192;
	static char s_buffer[MAX_CHARS];

	int charsWritten = vsprintf_s(s_buffer, format, args);

	OutputDebugStringA(s_buffer);

	return charsWritten;
}

//------------------------------------------------------------------------
int DebugPrintf(const char* format, ...)
{
	va_list argList;
	va_start(argList, format);

	int charsWritten = VDebugPrintf(format, argList);

	va_end(argList);

	return charsWritten;
}

//------------------------------------------------------------------------
HRESULT CreateInputLayoutDescFromVertexShaderSignature(ID3DBlob* pShaderBlob, ID3D11Device* pD3DDevice, ID3D11InputLayout** pInputLayout)
{
	// Reflect shader info
	ID3D11ShaderReflection* pVertexShaderReflection = NULL;
	if (FAILED(D3DReflect(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pVertexShaderReflection)))
	{
		return S_FALSE;
	}

	// Get shader info
	D3D11_SHADER_DESC shaderDesc;
	pVertexShaderReflection->GetDesc(&shaderDesc);

	// Read input layout description from shader info
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
	for (UINT i = 0; i < shaderDesc.InputParameters; i++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
		pVertexShaderReflection->GetInputParameterDesc(i, &paramDesc);

		// fill out input element desc
		D3D11_INPUT_ELEMENT_DESC elementDesc;
		elementDesc.SemanticName = paramDesc.SemanticName;
		elementDesc.SemanticIndex = paramDesc.SemanticIndex;
		elementDesc.InputSlot = 0;
		elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elementDesc.InstanceDataStepRate = 0;

		// determine DXGI format
		if (paramDesc.Mask == 1)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
		}
		else if (paramDesc.Mask <= 3)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		}
		else if (paramDesc.Mask <= 7)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		}
		else if (paramDesc.Mask <= 15)
		{
			if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
			else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		}

		//save element desc
		inputLayoutDesc.push_back(elementDesc);
	}

	// Try to create Input Layout
	HRESULT hr = pD3DDevice->CreateInputLayout(&inputLayoutDesc[0], (UINT) inputLayoutDesc.size(), pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pInputLayout);

	//Free allocation shader reflection memory
	pVertexShaderReflection->Release();
	return hr;
}

//------------------------------------------------------------------------
void SetSamplerStates(ID3D11DeviceContext* pDevCon)
{
	auto ps = States::Get();

	ID3D11SamplerState* pSamplers[] =
	{
		ps->pSamplerLinearWrap,
		ps->pSamplerLinearClamp,
		ps->pSamplerPointWrap,
		ps->pSamplerPointClamp,
		ps->pSamplerAnisoWrap,
		ps->pSamplerAnisoClamp,
		ps->pSamplerComparisonLinear
	};

	const unsigned int nSamplers = ARRAYSIZE(pSamplers);

	pDevCon->VSSetSamplers(0, nSamplers, pSamplers);
	pDevCon->PSSetSamplers(0, nSamplers, pSamplers);
	pDevCon->HSSetSamplers(0, nSamplers, pSamplers);
	pDevCon->DSSetSamplers(0, nSamplers, pSamplers);
	pDevCon->CSSetSamplers(0, nSamplers, pSamplers);
}

//------------------------------------------------------------------------
void SaveBackbufferToFile(IDXGISwapChain* pSwapchain, ID3D11DeviceContext* pDevContext)
{
	ID3D11Texture2D* pBackbuffer = nullptr;
	HRESULT hr = pSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackbuffer);

	if (SUCCEEDED(hr))
	{
		// Determine name of file
		SYSTEMTIME systemTime;
		GetLocalTime(&systemTime);

		char buffer[256];
		sprintf_s(buffer, "Screenshot_%02d-%02d-%02d_%02d-%02d-%02d.png", systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour, systemTime.wMinute, systemTime.wSecond);

		hr = DirectX::SaveWICTextureToFile(pDevContext, pBackbuffer, GUID_ContainerFormatPng, AnsiToWString(buffer).c_str());
		if (SUCCEEDED(hr))
		{
			DebugPrintf("Screenshot %s saved\n", buffer);
		}
	}

	SAFE_RELEASE(pBackbuffer);
}

void SaveTextureToFile(ID3D11ShaderResourceView* pSRV, ID3D11DeviceContext* pDevCon)
{
	ID3D11Texture2D* pTex2D = nullptr;
	pSRV->GetResource( (ID3D11Resource**)&pTex2D);

	if (pTex2D != nullptr)
	{
		// Determine name of file
		SYSTEMTIME systemTime;
		GetLocalTime(&systemTime);

		char buffer[256];
		sprintf_s(buffer, "Screenshot_%02d-%02d-%02d_%02d-%02d-%02d.png", systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour, systemTime.wMinute, systemTime.wSecond);

		HRESULT hr = DirectX::SaveWICTextureToFile(pDevCon, pTex2D, GUID_ContainerFormatPng, AnsiToWString(buffer).c_str());
		if (SUCCEEDED(hr))
		{
			DebugPrintf("Texture %s saved\n", buffer);
		}
	}

	SAFE_RELEASE(pTex2D);
}

//------------------------------------------------------------------------
void SaveCubemapToFile(ID3D11ShaderResourceView* pSRV, ID3D11DeviceContext* pDevCon)
{
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

	pSRV->GetDesc(&srvDesc);

	//srvDesc.Format
}

//------------------------------------------------------------------------
uint32 CalculateShaderTimestamp(unsigned short seconds, unsigned short miliseconds)
{
	return static_cast<unsigned int>(seconds * miliseconds + miliseconds + 1);
}

