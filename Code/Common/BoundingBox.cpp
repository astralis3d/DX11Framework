#include "PCH.h"
#include "BoundingBox.h"
#include "Defines.h"

SBoundingBox::SBoundingBox()
	: m_clrBox(1.f, 0.f, 0.f)
	, m_clrDiagonal(0.f, 0.5f, 1.f)
	, m_bDrawDiagonal(true)
{

}

void SBoundingBox::Init(const Vec3& vMin, const Vec3& vMax)
{
	m_obb[0] = vMin;							// xyz
	m_obb[1] = Vec3(vMax.x, vMin.y, vMin.z);	// Xyz
	m_obb[2] = Vec3(vMin.x, vMax.y, vMin.z);	// xYz
	m_obb[3] = Vec3(vMax.x, vMax.y, vMin.z);	// XYz
	m_obb[4] = Vec3(vMin.x, vMin.y, vMax.z);	// xyZ
	m_obb[5] = Vec3(vMax.x, vMin.y, vMax.z);	// XyZ
	m_obb[6] = Vec3(vMin.x, vMax.y, vMax.z);	// xYZ
	m_obb[7] = vMax;							// XYZ
}

void SBoundingBox::Update(const Mat44& matTransform)
{
	for (int i = 0; i < 8; ++i)
		m_obbFinal[i] = matTransform.TransformPoint(m_obb[i]);

	m_boundingBoxMin = m_boundingBoxMax = m_obbFinal[0];

	for (int i = 1; i < 8; ++i)
	{
		const float fSum = m_obbFinal[i].x + m_obbFinal[i].y + m_obbFinal[i].z;
		const float fSumMin = m_boundingBoxMin.x + m_boundingBoxMin.y + m_boundingBoxMin.z;
		const float fSumMax = m_boundingBoxMax.x + m_boundingBoxMax.y + m_boundingBoxMax.z;

		if (fSum < fSumMin)
			m_boundingBoxMin = m_obbFinal[i];
		else if (fSum > fSumMax)
			m_boundingBoxMax = m_obbFinal[i];
	}
}

//------------------------------------------------------------------------
void SBoundingBox::InitializeD3D11(ID3D11Device* pDevice)
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
	bufDesc.ByteWidth = 26 * sizeof(USHORT);
	bufDesc.CPUAccessFlags = 0;
	bufDesc.MiscFlags = 0;
	bufDesc.StructureByteStride = 0;
	bufDesc.Usage = D3D11_USAGE_IMMUTABLE;
	{
		USHORT indicesData[26] =
		{
			0, 1,
			2, 3,
			4, 5,
			6, 7,

			0, 4,
			1, 5,
			2, 6,
			3, 7,

			0, 2,
			1, 3,
			4, 6,
			5, 7,

			8, 9
		};

		D3D11_SUBRESOURCE_DATA pData;
		pData.pSysMem = indicesData;

		V(pDevice->CreateBuffer(&bufDesc, &pData, &m_indexBuffer));
	}
}

//------------------------------------------------------------------------
void SBoundingBox::ReleaseD3D11()
{
	m_pixelShader.reset();
	m_vertexShader.reset();

	SAFE_RELEASE(m_pInputLayout);

	SAFE_RELEASE(m_indexBuffer);
	SAFE_RELEASE(m_vertexBuffer);
}

//------------------------------------------------------------------------
void SBoundingBox::Draw(ID3D11DeviceContext* pDevCon)
{
	D3D11_MAPPED_SUBRESOURCE mappedSubr;

	SVertexPosColor data[10];
	for (int i = 0; i < 8; ++i)
	{
		data[i].color = m_clrBox;
		data[i].pos = m_obbFinal[i];
	}

	data[8].color = data[9].color = m_clrDiagonal;
	data[8].pos = m_boundingBoxMin;
	data[9].pos = m_boundingBoxMax;

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

	pDevCon->DrawIndexed( (m_bDrawDiagonal) ? 26 : 24, 0, 0);
}
