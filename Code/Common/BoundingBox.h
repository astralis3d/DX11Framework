#ifndef __boundingbox_h__
#define __boundingbox_h__

#ifdef _MSC_VER
# pragma once
#endif

#include <DXUT.h>
#include "mathlib.h"
#include "D3DShaders.h"

//----------------------------------------------------------------------
// BoundingBox
//----------------------------------------------------------------------

struct SBoundingBox
{
	SBoundingBox();

	void Init(const Vec3& vMin, const Vec3& vMax);
	void Update(const Mat44& matTransform);

	void InitializeD3D11(ID3D11Device* pDevice);
	void ReleaseD3D11();

	void Draw(ID3D11DeviceContext* pDevCon);


	void SetColorBox(const Vec3& clr) { m_clrBox = clr; }
	void SetColorDiagonal(const Vec3& clr) { m_clrDiagonal = clr; }
	void DrawDiagonal(bool bDraw) { m_bDrawDiagonal = bDraw; }


	// Original OBB
	Vec3 m_obb[8];

	// Transformed OBB
	Vec3 m_obbFinal[8];

	// Current bounding box
	Vec3	m_boundingBoxMin;
	Vec3	m_boundingBoxMax;

	// D3D11 (temp)
	TPixelShader		m_pixelShader;
	TVertexShader		m_vertexShader;
	ID3D11InputLayout*	m_pInputLayout;

	ID3D11Buffer*		m_vertexBuffer;
	ID3D11Buffer*		m_indexBuffer;

	// Drawing params
	Vec3				m_clrBox;
	Vec3				m_clrDiagonal;
	bool				m_bDrawDiagonal;
};

#endif