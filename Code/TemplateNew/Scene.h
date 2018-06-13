#ifndef __SCENE_H__
#define __SCENE_H__

#pragma once

#include "PCH.h"
#include "ceMesh.h"
#include "D3DTexture.h"

class CMeshRenderer;
class CViewFrustum;

struct SCBMaterial
{
	Vec3	m_BaseColor;
	float	m_Roughness;

	float	pad[2];
	float	m_Metallic;
	float	m_MaterialID;
};

//------------------------------------------------------------------------
// CScene
//
// Contains all objects & rendering-related stuff.
//------------------------------------------------------------------------

class CScene
{
public:
	struct SCBPerObjectStatic
	{
		Mat44		mWorld;
	};

public:
	CScene();
	~CScene();

	void Initialize(ID3D11Device* pDevice, CViewFrustum* pViewFrustum);
	void Cleanup();

	void GetBoundingBox(Vec3& vMin, Vec3& vMax);

	void RenderScene(ID3D11DeviceContext* pDevCon);
	void RenderSceneDepth(ID3D11DeviceContext* pDevCon, bool bPrepass);

	unsigned int GetNumVertices() const { return m_nVertices; }
	unsigned int GetNumTriangles() const { return m_nTriangles; }

	float& GetScaleFactor() { return m_fScaleFactor; }

	void SetupMesh(const char* path);
	void SaveScaleFactorToRegistry();

private:
	void CreateShaders(ID3D11Device*);
	void LoadTextures(ID3D11Device*);

	void RenderMeshes(ID3D11DeviceContext* pDevCon, const Mat44& matWorld, bool bVisibilityTest=true);


private:
	Vec3				m_sceneMin;
	Vec3				m_sceneMax;

	unsigned int		m_nVertices;
	unsigned int		m_nTriangles;

	CViewFrustum*		m_pViewFrustum;

private:
	// Meshes in this scene
	CceMesh				m_meshSponza;
	float				m_fScaleFactor;
	std::string			m_meshFilename;	// TODO, some kind of map?


private:
	//------------------------------------------------------------------------
	// Scene rendering
	//------------------------------------------------------------------------
	D3DConstantBuffer<SCBPerObjectStatic>	m_CBufferPerObject;
	D3DConstantBuffer<SCBMaterial>			m_CBufferMaterial;

	TVertexShader							m_VS_GBuffer;

	TPixelShader							m_PS_GBuffer_TexAlbedo_TexNorm;
	TPixelShader							m_PS_GBuffer_FromCBuffer;
	
	ID3D11InputLayout*						m_pInputLayout;
	ID3D11InputLayout*						m_pInputLayoutDepth;

	TVertexShader							m_VS_Depth;
};


#endif