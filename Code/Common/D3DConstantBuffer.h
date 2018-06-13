// Simplified interface for constant buffers

#pragma once

#include <DXUT.h>

template <typename BufferType>
class D3DConstantBuffer
{
public:
	D3DConstantBuffer() : m_pBuffer(nullptr) {}
	~D3DConstantBuffer() { Release(); }

	void Create(ID3D11Device* pDevice)
	{
		D3D11_BUFFER_DESC BufferDesc;

		BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		BufferDesc.ByteWidth = sizeof(BufferType);
		BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		BufferDesc.MiscFlags = 0;
		BufferDesc.StructureByteStride = 0;
		BufferDesc.Usage = D3D11_USAGE_DYNAMIC;		

		HRESULT Result = pDevice->CreateBuffer(&BufferDesc, nullptr, &m_pBuffer);
		_ASSERT(SUCCEEDED(Result));
	}

	void Release()
	{
		SAFE_RELEASE(m_pBuffer);
	}

	void UpdateBuffer(ID3D11DeviceContext* pDevContext)
	{
		void* pData = Lock(pDevContext);
		memcpy(pData, &m_bufferData, sizeof(m_bufferData));
		Unlock(pDevContext);
	}

	// Setters for buffers
	void SetVS(ID3D11DeviceContext* pDevContext, UINT slot)
	{
		ID3D11Buffer* pBuffers[1];
		pBuffers[0] = m_pBuffer;

		pDevContext->VSSetConstantBuffers(slot, 1, pBuffers);
	}

	void SetPS(ID3D11DeviceContext* pDevContext, UINT slot)
	{
		ID3D11Buffer* pBuffers[1];
		pBuffers[0] = m_pBuffer;

		pDevContext->PSSetConstantBuffers(slot, 1, pBuffers);
	}

	void SetGS(ID3D11DeviceContext* pDevContext, UINT slot)
	{
		ID3D11Buffer* pBuffers[1];
		pBuffers[0] = m_pBuffer;

		pDevContext->GSSetConstantBuffers(slot, 1, pBuffers);
	}

	void SetDS(ID3D11DeviceContext* pDevContext, UINT slot)
	{
		ID3D11Buffer* pBuffers[1];
		pBuffers[0] = m_pBuffer;

		pDevContext->DSSetConstantBuffers(slot, 1, pBuffers);
	}

	void SetHS(ID3D11DeviceContext* pDevContext, UINT slot)
	{
		ID3D11Buffer* pBuffers[1];
		pBuffers[0] = m_pBuffer;

		pDevContext->HSSetConstantBuffers(slot, 1, pBuffers);
	}

	void SetCS(ID3D11DeviceContext* pDevContext, UINT slot)
	{
		ID3D11Buffer* pBuffers[1];
		pBuffers[0] = m_pBuffer;

		pDevContext->CSSetConstantBuffers(slot, 1, pBuffers);
	}


	BufferType& GetBufferData() { return m_bufferData; }
	ID3D11Buffer* GetBuffer() { return m_pBuffer; }	

private:
	void* Lock(ID3D11DeviceContext* pDevContext)
	{
		D3D11_MAPPED_SUBRESOURCE Resource;
		HRESULT hr = pDevContext->Map(m_pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &Resource);
		_ASSERT ( SUCCEEDED(hr) );

		return Resource.pData;
	}

	void Unlock(ID3D11DeviceContext* pDevContext)
	{
		pDevContext->Unmap(m_pBuffer, 0);
	}

private:
	BufferType		m_bufferData;
	ID3D11Buffer*	m_pBuffer;
};