#ifndef __texture_h__
#define __texture_h__

#pragma once

#include <DXUT.h>

class CTexture
{
public:
	CTexture();
	CTexture(ID3D11Device* pDevice, const char* const pFile, D3D11_TEXTURE_ADDRESS_MODE addU, D3D11_TEXTURE_ADDRESS_MODE addV);
	~CTexture();

	void Initialize(ID3D11Device* pDevice, const char* const pFile, D3D11_TEXTURE_ADDRESS_MODE addU, D3D11_TEXTURE_ADDRESS_MODE addV);
	bool LoadTexture(ID3D11Device* pDevice, const char* const pPath);
	void ReleaseTexture();
	
	void SetPS(ID3D11DeviceContext* pDevContext, UINT slot);
	void SetGS(ID3D11DeviceContext* pDevContext, UINT slot);
	void SetHS(ID3D11DeviceContext* pDevContext, UINT slot);
	void SetDS(ID3D11DeviceContext* pDevContext, UINT slot);
	void SetCS(ID3D11DeviceContext* pDevContext, UINT slot);

	bool IsGood() const;

	inline ID3D11ShaderResourceView* GetSRV() const { return m_pSRV; }

private:
	ID3D11ShaderResourceView*	m_pSRV;
	bool						m_bIsGood;
};

#endif