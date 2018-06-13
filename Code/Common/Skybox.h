#ifndef __skybox_h__
#define __skybox_h__

#ifdef _MSC_VER
# pragma once
#endif

#include <DXUT.h>
#include "ceMesh.h"
#include "D3DShaders.h"
#include "D3DTexture.h"
#include "D3DConstantBuffer.h"
#include "mathlib.h"

class CSkybox
{
public:
	CSkybox();
	~CSkybox();

	void Initialize(ID3D11Device* pDevice);
	void Release();

	void RenderCubeMap(ID3D11DeviceContext* pDevContext);
	void Render(ID3D11DeviceContext* pDevContext, const Vec3& eye, const Mat44& matViewProj, bool bRenderClouds, bool bEnvmap, bool bCIEClearSky);
	void PreRender(ID3D11DeviceContext* pDevContext);

	// For static textures
	void SetTextureSphere(ID3D11Device* pDevice, const char* path);
	void SetTextureCubemap(ID3D11Device* pDevice, const char* path);

	ID3D11ShaderResourceView* GetCubeMapSRV() const { return m_texCubemap.GetSRV(); }
	ID3D11ShaderResourceView* GetSphereMapSRV() const { return m_texDiffuse.GetSRV(); }

	ID3D11ShaderResourceView* GetDynamicEnvironmentMap() const { return m_cubeMapSRV; }

	

	const CceMesh& GetSkyboxMesh() const { return m_meshSkybox; }

	void DrawSkySphereOnly(ID3D11DeviceContext* pDevContext);

private:

	struct SCBPerObjectStatic
	{
		Mat44	mWorld;
		Mat44	mViewProj;
	};

	struct SCBCloudsCB
	{
		Vec4	params0;
		Vec4	params1;
	};

private:
	CceMesh		m_meshSkybox;
	CceMesh		m_meshClouds;

	PixelShaderPtr	m_pixelShader;
	PixelShaderPtr	m_pPixelShaderEnvmap;
	PixelShaderPtr	m_pixelShaderCIEClearSky;
	PixelShaderPtr	m_pixelShaderClouds[2];

	VertexShaderPtr	m_vertexShader;

	CTexture		m_texDiffuse;
	CTexture		m_texFalloff;
	CTexture		m_texClouds[2];
	CTexture		m_texPerlin;
	CTexture		m_texNormals;

	CTexture		m_texCubemap;

	ID3D11DepthStencilState*	m_pSkyboxDSState;
	ID3D11BlendState*			m_pBS_Clouds;
	ID3D11RasterizerState*		m_pSkyboxRSState = nullptr;

	D3DConstantBuffer<SCBPerObjectStatic>	m_CBPerObject;
	D3DConstantBuffer<SCBCloudsCB>			m_CBClouds;

	SCBCloudsCB		m_cloudsData[3];

	ID3D11InputLayout*	m_pInputLayout;



	// For cubemap
	ID3D11RenderTargetView*	m_cubeMapRTV[6];
	ID3D11ShaderResourceView* m_cubeMapSRV;
	ID3D11DepthStencilView*	m_cubeMapDSV;
	D3D11_VIEWPORT		m_cubeMapViewport;

	Mat44	m_cubeMapMatView[6];
	Mat44	m_cubeMapMatProj;

};

#endif