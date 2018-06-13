#include "PCH.h"
#include "D3DTexture.h"
#include <DDSTextureLoader.h>
#include <WICTextureLoader.h>

CTexture::CTexture()
	: m_pSRV(0)
	, m_bIsGood(false)
{

}

CTexture::CTexture(ID3D11Device* pDevice, const char* const pFile, D3D11_TEXTURE_ADDRESS_MODE addU, D3D11_TEXTURE_ADDRESS_MODE addV)
	: m_pSRV(0)
	, m_bIsGood(false)
{
	Initialize(pDevice, pFile, addU, addV);
}

//----------------------------------------------------------------------
CTexture::~CTexture()
{
	ReleaseTexture();
}

//----------------------------------------------------------------------
void CTexture::Initialize(ID3D11Device* pDevice, const char* const pFile, D3D11_TEXTURE_ADDRESS_MODE addU, D3D11_TEXTURE_ADDRESS_MODE addV)
{
	if (!LoadTexture(pDevice, pFile))
	{
		// todo, logger
		return;
	}
}

//----------------------------------------------------------------------
bool CTexture::LoadTexture(ID3D11Device* pDevice, const char* const pPath)
{
	ReleaseTexture();

	wchar_t unicode[256];
	MultiByteToWideChar(CP_ACP, 0, pPath, -1, unicode, 256);

	HRESULT hr = DirectX::CreateDDSTextureFromFile(pDevice, DXUTGetD3D11DeviceContext(), unicode, nullptr, &m_pSRV);
	//HRESULT hr = DirectX::CreateDDSTextureFromFileEx(pDevice, unicode, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, true, nullptr, &m_pSRV);

	if (FAILED(hr))
	{

		hr = DirectX::CreateWICTextureFromFile(pDevice, DXUTGetD3D11DeviceContext(), unicode, nullptr, &m_pSRV);
		//hr = DirectX::CreateWICTextureFromFileEx(pDevice, unicode, 0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0, true, nullptr, &m_pSRV);

		if (FAILED(hr))
		{
			DebugPrintf("ERROR!!! Texture Load Failed: %s\n", pPath);

			return false;
		}
			
	}

	else
		m_bIsGood = true;

	return true;
}

//----------------------------------------------------------------------
void CTexture::ReleaseTexture()
{
	SAFE_RELEASE(m_pSRV);

	m_bIsGood = false;
}

//----------------------------------------------------------------------
bool CTexture::IsGood() const
{
	return m_bIsGood;
}

void CTexture::SetPS(ID3D11DeviceContext* pDevContext, UINT slot)
{
	ID3D11ShaderResourceView* srv[1] = { m_pSRV };
	pDevContext->PSSetShaderResources(slot, 1, srv);
}

void CTexture::SetGS(ID3D11DeviceContext* pDevContext, UINT slot)
{
	ID3D11ShaderResourceView* srv[1] = { m_pSRV };
	pDevContext->GSSetShaderResources(slot, 1, srv);
}

void CTexture::SetHS(ID3D11DeviceContext* pDevContext, UINT slot)
{
	ID3D11ShaderResourceView* srv[1] = { m_pSRV };
	pDevContext->HSSetShaderResources(slot, 1, srv);
}

void CTexture::SetCS(ID3D11DeviceContext* pDevContext, UINT slot)
{
	ID3D11ShaderResourceView* srv[1] = { m_pSRV };
	pDevContext->CSSetShaderResources(slot, 1, srv);
}

void CTexture::SetDS(ID3D11DeviceContext* pDevContext, UINT slot)
{
	ID3D11ShaderResourceView* srv[1] = { m_pSRV };
	pDevContext->DSSetShaderResources(slot, 1, srv);
}
