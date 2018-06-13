// Interface for structured buffers

#pragma once

template <typename BufferType>
class D3DStructuredBuffer
{
public:
	D3DStructuredBuffer(ID3D11Device* pDevice, unsigned int nElements, UINT bindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS,
						bool bDynamic = false, bool bAppendConsume = false, bool bHiddenCounter = false);

	~D3DStructuredBuffer();

	ID3D11Buffer*				GetBuffer() const { return Buffer; }
	ID3D11ShaderResourceView*	GetSRV() const { return SRV; }
	ID3D11UnorderedAccessView*	GetUAV() const { return UAV; }

	// Valid only for dynamic buffers
	BufferType* MapDiscard(ID3D11DeviceContext* pDevContext);
	void Unmap(ID3D11DeviceContext* pDevContext);

private:
	uint32						Elements;
	ID3D11Buffer*				Buffer;
	ID3D11ShaderResourceView*	SRV;
	ID3D11UnorderedAccessView*	UAV;

};

template <typename BufferType>
D3DStructuredBuffer<BufferType>::D3DStructuredBuffer(ID3D11Device* pDevice, uint32 nElements,
													 UINT bindFlags /*= D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS*/,
													 bool bDynamic /*= false*/, bool bAppendConsume /*= false*/, bool bHiddenCounter /*= false*/)
	: Elements(nElements), Buffer(nullptr), SRV(nullptr), UAV(nullptr)
{
	//assert(bAppendConsume == false || bHiddenCounter == false);

	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.ByteWidth = sizeof(BufferType) * nElements;
	bufferDesc.Usage = bDynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = bindFlags;	
	bufferDesc.CPUAccessFlags = bDynamic ? D3D11_CPU_ACCESS_WRITE : 0;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	bufferDesc.StructureByteStride = sizeof(BufferType);

	/*
	D3D11_SUBRESOURCE_DATA subresourceData;
	subresourceData.pSysMem = initData;
	subresourceData.SysMemPitch = 0;
	subresourceData.SysMemSlicePitch = 0;
	*/

	HRESULT hr = E_FAIL;
	V(pDevice->CreateBuffer(&bufferDesc, nullptr, &Buffer));

	if (bindFlags & D3D11_BIND_SHADER_RESOURCE)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = nElements;

		V(pDevice->CreateShaderResourceView(Buffer, &srvDesc, &SRV));

	}

	if (bindFlags & D3D11_BIND_UNORDERED_ACCESS)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = nElements;
		uavDesc.Buffer.Flags = 0;
		uavDesc.Buffer.Flags |= bAppendConsume ? D3D11_BUFFER_UAV_FLAG_APPEND : 0;
		uavDesc.Buffer.Flags |= bHiddenCounter ? D3D11_BUFFER_UAV_FLAG_COUNTER : 0;

		V(pDevice->CreateUnorderedAccessView(Buffer, &uavDesc, &UAV));
	}
}

//------------------------------------------------------------------------
template <typename BufferType>
D3DStructuredBuffer<BufferType>::~D3DStructuredBuffer()
{
	SAFE_RELEASE( UAV );
	SAFE_RELEASE( SRV );

	SAFE_RELEASE( Buffer );
}

//------------------------------------------------------------------------
template <typename BufferType>
BufferType* D3DStructuredBuffer<BufferType>::MapDiscard(ID3D11DeviceContext* pDevContext)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	pDevContext->Map(Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	return static_cast<BufferType*>(mappedResource.pData);
}

//------------------------------------------------------------------------
template <typename BufferType>
void D3DStructuredBuffer<BufferType>::Unmap(ID3D11DeviceContext* pDevContext)
{
	pDevContext->Unmap(Buffer, 0);
}



