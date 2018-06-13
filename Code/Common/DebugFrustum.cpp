#include "PCH.h"
#include "DebugFrustum.h"
#include "Defines.h"
#include "D3DStates.h"

SDebugFrustum::SDebugFrustum()
	: m_clrBox(1.f, 0.f, 0.f)
{

}

void SDebugFrustum::Init(const Vec3& nearTopLeft, const Vec3& nearTopRight, const Vec3& nearBottomLeft, const Vec3& nearBottomRight, const Vec3& farTopLeft, const Vec3& farTopRight, const Vec3& farBottomLeft, const Vec3& farBottomRight)
{
	m_frustum[0] = nearBottomLeft;
	m_frustum[1] = nearTopLeft;
	m_frustum[2] = nearBottomRight;
	m_frustum[3] = nearTopRight;
	m_frustum[4] = farBottomLeft;
	m_frustum[5] = farTopLeft;
	m_frustum[6] = farBottomRight;
	m_frustum[7] = farTopRight;
}

void SDebugFrustum::Update(const Mat44& matTransform)
{

}

void SDebugFrustum::InitializeD3D11(ID3D11Device* pDevice)
{
	// *** SHADERS ***
	m_pixelShader.reset(new CPixelShader(pDevice, "..//Common//PosColor.hlsl", "PSSimple"));
	m_vertexShader.reset(new CVertexShader(pDevice, "..//Common//PosColor.hlsl", "VSSimple"));

	// *** INPUT LAYOUT ***
	const D3D11_INPUT_ELEMENT_DESC elemDescSimple[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	HRESULT hr = S_OK;
	pDevice->CreateInputLayout(elemDescSimple, 2, (const void*)m_vertexShader->GetBlob()->GetBufferPointer(), m_vertexShader->GetBlob()->GetBufferSize(), &m_pInputLayout);
	V(hr);

	// *** BUFFERS ***
	D3D11_BUFFER_DESC bufDesc;
	memset(&bufDesc, 0, sizeof(bufDesc));

	// Dynamic vertex buffer for OBB
	bufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufDesc.ByteWidth = 10 * sizeof(SVertexPosColor);
	bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bufDesc.MiscFlags = 0;
	bufDesc.StructureByteStride = 0;
	bufDesc.Usage = D3D11_USAGE_DYNAMIC;

	V(pDevice->CreateBuffer(&bufDesc, nullptr, &m_vertexBuffer));

	// Index buffer for OBB
	bufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufDesc.ByteWidth = 24 * sizeof(USHORT);
	bufDesc.CPUAccessFlags = 0;
	bufDesc.MiscFlags = 0;
	bufDesc.StructureByteStride = 0;
	bufDesc.Usage = D3D11_USAGE_IMMUTABLE;
	{
		USHORT indicesData[24] =
		{
			0, 1,
			1, 3,
			3, 2,
			2, 0,

			1, 5,
			3, 7,
			0, 4,
			2, 6,

			5, 7,
			7, 6,
			6, 4,
			4, 5
		};

		D3D11_SUBRESOURCE_DATA pData;
		pData.pSysMem = indicesData;

		V(pDevice->CreateBuffer(&bufDesc, &pData, &m_indexBuffer));
	}

	D3D11_RASTERIZER_DESC	rsDesc;
	rsDesc.AntialiasedLineEnable = FALSE;
	rsDesc.CullMode = D3D11_CULL_NONE;
	rsDesc.DepthBias = 0;
	rsDesc.DepthBiasClamp = 0.f;
	rsDesc.DepthClipEnable = FALSE;
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.FrontCounterClockwise = FALSE;
	rsDesc.MultisampleEnable = FALSE;
	rsDesc.ScissorEnable = FALSE;
	rsDesc.SlopeScaledDepthBias = 0.f;
	V(pDevice->CreateRasterizerState(&rsDesc, &m_pRasterizerState));
}

//------------------------------------------------------------------------
void SDebugFrustum::ReleaseD3D11()
{
	m_pixelShader.reset();
	m_vertexShader.reset();

	SAFE_RELEASE(m_pInputLayout);
	SAFE_RELEASE(m_indexBuffer);
	SAFE_RELEASE(m_vertexBuffer);

	SAFE_RELEASE(m_pRasterizerState);
}

//------------------------------------------------------------------------
void SDebugFrustum::Draw(ID3D11DeviceContext* pDevCon)
{
	D3D11_MAPPED_SUBRESOURCE mappedSubr;

	SVertexPosColor data[8];
	for (int i = 0; i < 8; ++i)
	{
		data[i].color = m_clrBox;
		data[i].pos = m_frustum[i];
	}	
					  
	pDevCon->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mappedSubr);
	memcpy(mappedSubr.pData, data, sizeof(data));
	pDevCon->Unmap(m_vertexBuffer, 0);


	UINT offset = 0;
	UINT stride = sizeof(SVertexPosColor);

	pDevCon->PSSetShader(*m_pixelShader, nullptr, 0);
	pDevCon->VSSetShader(*m_vertexShader, nullptr, 0);

	pDevCon->IASetInputLayout(m_pInputLayout);
	pDevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	pDevCon->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	pDevCon->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	pDevCon->RSSetState(m_pRasterizerState);
	pDevCon->OMSetDepthStencilState( States::Get()->pNoDepthNoStencil_DS, 0);

	pDevCon->DrawIndexed(24, 0, 0);
}
