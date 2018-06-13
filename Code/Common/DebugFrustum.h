#ifndef __debugFrustum_h__
#define __debugFrustum_h__

#ifdef _MSC_VER
# pragma once
#endif

#include <DXUT.h>
#include "mathlib.h"
#include "D3DShaders.h"

//----------------------------------------------------------------------
// BoundingBox
//----------------------------------------------------------------------

struct SDebugFrustum
{
	SDebugFrustum();

	void Init(const Vec3& nearTopLeft, const Vec3& nearTopRight, const Vec3& nearBottomLeft, const Vec3& nearBottomRight,
			  const Vec3& farTopLeft, const Vec3& farTopRight, const Vec3& farBottomLeft, const Vec3& farBottomRight);
	void Update(const Mat44& matTransform);

	void InitializeD3D11(ID3D11Device* pDevice);
	void ReleaseD3D11();

	void Draw(ID3D11DeviceContext* pDevCon);
	
	void SetColorBox(const Vec3& clr) { m_clrBox = clr; }
	

	// Original OBB
	Vec3 m_frustum[8];

	// D3D11 (temp)
	TPixelShader		m_pixelShader;
	TVertexShader		m_vertexShader;
	ID3D11InputLayout*	m_pInputLayout;
	ID3D11RasterizerState*	m_pRasterizerState;

	ID3D11Buffer*		m_vertexBuffer;
	ID3D11Buffer*		m_indexBuffer;

	// Drawing params
	Vec3				m_clrBox;
};

#endif