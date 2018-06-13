#include "PCH.h"
#include "Scene.h"
#include "ViewFrustum.h"

CScene::CScene()
	: m_fScaleFactor(1.0f)
{
	m_pInputLayout = nullptr;
	m_pInputLayoutDepth = nullptr;
}

CScene::~CScene()
{
	
}

void CScene::Initialize(ID3D11Device* pDevice, CViewFrustum* pViewFrustum)
{
	m_pViewFrustum = pViewFrustum;

	// Load all needed meshes	
	m_meshSponza.Initialize(pDevice);
	
	
	//------------------------------------------------------------------------
	//SetupMesh("..//..//Media//Meshes//Sponza//sponza2_opt.cemesh");
	//SetupMesh( "..//..//Media//Meshes//Soldier//Soldier.obj" );
	SetupMesh("..//..//Media//Meshes//Powerplant//powerplant.cemesh");
	//SetupMesh( "..//..//Media//Meshes//SunTemple//suntemple." );
	//SetupMesh( "..//..//Media//Meshes//Dragon//dragon.cemesh" );

	//------------------------------------------------------------------------

	CreateShaders(pDevice);
	LoadTextures(pDevice);

	// Constant buffer(s)
	m_CBufferMaterial.Create(pDevice);
	m_CBufferPerObject.Create(pDevice);

	// Create input layouts
	{
		const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc[] =
		{
			{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, 12,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		1, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT,		1, 12,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BITANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT,		1, 24,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		HRESULT hr = pDevice->CreateInputLayout(InputLayoutDesc, ARRAYSIZE(InputLayoutDesc), m_VS_GBuffer->GetBlob()->GetBufferPointer(), m_VS_GBuffer->GetBlob()->GetBufferSize(), &m_pInputLayout);
		V(hr);

		hr = pDevice->CreateInputLayout(InputLayoutDesc, 2, m_VS_Depth->GetBlob()->GetBufferPointer(), m_VS_Depth->GetBlob()->GetBufferSize(), &m_pInputLayoutDepth);

		V(hr);
	}
		
}


//------------------------------------------------------------------------
void CScene::CreateShaders(ID3D11Device* pDevice)
{
	// Deferred Shading - GBuffer VS
	m_VS_GBuffer.reset(new CVertexShader(pDevice, "GBuffer.hlsl", "RenderSceneToGBufferVS"));
	m_VS_Depth.reset( new CVertexShader(pDevice, "..//Common//Depth.hlsl", "DepthVSShadowMap") );

	// GBuffer PS with Albedo only from texture
	{
		CD3D11ShaderMacro Defines;
		Defines.AddMacro("ALBEDO_FROM_TEXTURE", "1");
		Defines.AddMacro("USE_NORMAL_MAPPING", "1");
		Defines.AddMacro("METALLIC_FROM_TEXTURE", "1");
		Defines.Finish();

		m_PS_GBuffer_TexAlbedo_TexNorm.reset(new CPixelShader());
		m_PS_GBuffer_TexAlbedo_TexNorm->Initialize(pDevice, "GBuffer.hlsl", "RenderSceneToGBufferPS", &Defines);
	}

	// PS with all from cbuffer
	{
		m_PS_GBuffer_FromCBuffer.reset(new CPixelShader());
		m_PS_GBuffer_FromCBuffer->Initialize(pDevice, "GBuffer.hlsl", "RenderSceneToGBufferPS", nullptr);
	}

}

//------------------------------------------------------------------------
void CScene::LoadTextures(ID3D11Device* pDevice)
{
	
}

//------------------------------------------------------------------------
void CScene::RenderMeshes(ID3D11DeviceContext* pDevCon, const Mat44& matWorld, bool bVisibilityTest/*=true*/)
{
	// Set constant buffers
	m_CBufferPerObject.GetBufferData().mWorld = matWorld;
	m_CBufferPerObject.UpdateBuffer(pDevCon);
	m_CBufferPerObject.SetVS(pDevCon, 2);

	UINT strides[2] = { sizeof(SceMeshVertexStream1), sizeof(SceMeshVertexStream2) };
	UINT offsets[2] = { 0, 0 };
	ID3D11Buffer* VertexBuffers[2] = { m_meshSponza.GetVertexBufferStream1(), m_meshSponza.GetVertexBufferStream2() };

	pDevCon->IASetVertexBuffers(0, 2, VertexBuffers, strides, offsets);
	pDevCon->IASetIndexBuffer(m_meshSponza.GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	pDevCon->IASetInputLayout(m_pInputLayout);
	pDevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	float BlendFactor[4] = { 1.f, 1.f, 1.f, 1.f };
	pDevCon->OMSetBlendState(States::Get()->pNoBlend_BS, BlendFactor, 0xFFFFFFFF);

	// Render chunks of model

	const unsigned int Submeshes = m_meshSponza.GetNumSubmeshes();

	for (unsigned int i = 0; i < Submeshes; ++i)
	{
		const CMeshChunk& Submesh = m_meshSponza.GetSubmesh(i);
		auto materialID = Submesh.m_materialID;

		Vec3 bbMin = matWorld.TransformPoint(Submesh.m_min);
		Vec3 bbMax = matWorld.TransformPoint(Submesh.m_max);

		if (!m_pViewFrustum->TestAgainstBoundingBox(bbMin, bbMax) && bVisibilityTest == true)
			continue;

		m_nVertices += Submesh.m_numVertices;
		m_nTriangles += Submesh.m_numIndices / 3;

		// Textures
		const CTexture& TexDiffuse = m_meshSponza.GetTexture(materialID, ceMESH_TEXTURE_DIFFUSE);
		const CTexture& TexNormal = m_meshSponza.GetTexture(materialID, ceMESH_TEXTURE_HEIGHT);
		const CTexture& TexOpacity = m_meshSponza.GetTexture(materialID, ceMESH_TEXTURE_OPACITY);
		const CTexture& TexSpecular = m_meshSponza.GetTexture( materialID, ceMESH_TEXTURE_SPECULAR );

		ID3D11ShaderResourceView* pSRV[4] = { TexDiffuse.GetSRV(), TexOpacity.GetSRV(), TexSpecular.GetSRV(), TexNormal.GetSRV() };

		pDevCon->PSSetShaderResources(1, 4, pSRV);


		pDevCon->DrawIndexed(Submesh.m_numIndices, Submesh.m_startIndexLocation, Submesh.m_baseVertexLocation);
	}
}

//------------------------------------------------------------------------
void CScene::SetupMesh(const char* path)
{
	m_meshFilename = GetFilenameFromPath(path);

	HKEY hKey;
	if ( RegCreateKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\DX11Framework\\Meshes"), 0, NULL, 0, KEY_READ | KEY_WRITE, NULL, &hKey, NULL) == ERROR_SUCCESS )
	{
		// Attempt to find value
		DWORD dwValueType = REG_BINARY;
		DWORD dwDataSize = sizeof(float);

		if ( RegQueryValueExA(hKey, m_meshFilename.c_str(), nullptr, &dwValueType, (LPBYTE) &m_fScaleFactor, &dwDataSize) != ERROR_SUCCESS )
		{
			// If failed, set scale factor to 1.0
			m_fScaleFactor = 1.0f;
		}
	}

	RegCloseKey(hKey);



	m_meshSponza.Clear();

	bool bResult;
	bResult = m_meshSponza.ImportFile( path, true, true, true );
	

	if (!bResult)
	{
		DebugPrintf("Failed to load sponza mesh!\n");
		return;
	}
	

	// Find bounding box
	CceMesh* pMeshes[] = { &m_meshSponza };

	m_sceneMin = Vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	m_sceneMax = Vec3(FLT_MIN, FLT_MIN, FLT_MIN);

	for (const auto mesh : pMeshes)
	{
		Vec3 vMin, vMax;
		mesh->GetBoundingBox(vMin, vMax);

		Vec3Min(m_sceneMin, vMin, &m_sceneMin);
		Vec3Max(m_sceneMax, vMax, &m_sceneMax);
	}
}

//------------------------------------------------------------------------
void CScene::SaveScaleFactorToRegistry()
{
	HKEY hKey;
	if (RegCreateKeyEx( HKEY_LOCAL_MACHINE, TEXT( "SOFTWARE\\DX11Framework\\Meshes" ), 0, NULL, 0, KEY_READ | KEY_WRITE, NULL, &hKey, NULL ) == ERROR_SUCCESS)
	{
		RegSetValueExA( hKey, m_meshFilename.c_str(), 0, REG_BINARY, (LPBYTE)&m_fScaleFactor, sizeof( float ) );
	}

	RegCloseKey( hKey );
}

//------------------------------------------------------------------------
void CScene::Cleanup()
{
	SaveScaleFactorToRegistry();
	m_meshSponza.Clear();	

	m_CBufferMaterial.Release();
	m_CBufferPerObject.Release();

	SAFE_RELEASE(m_pInputLayout);
	SAFE_RELEASE(m_pInputLayoutDepth);

	// Shaders
	m_VS_GBuffer.reset();
	m_PS_GBuffer_TexAlbedo_TexNorm.reset();

	m_VS_Depth.reset();

	m_PS_GBuffer_FromCBuffer.reset();
	
}

//------------------------------------------------------------------------
void CScene::GetBoundingBox(Vec3& vMin, Vec3& vMax)
{
	vMin = m_sceneMin;
	vMax = m_sceneMax;
}

//------------------------------------------------------------------------
void CScene::RenderSceneDepth( ID3D11DeviceContext* pDevCon, bool bPrepass )
{
	m_VS_Depth->Bind(pDevCon);
	pDevCon->PSSetShader(nullptr, nullptr, 0);

	// Matrix for translation objects
	Mat44 matWorld;
	MatrixScaling( &matWorld, m_fScaleFactor, m_fScaleFactor, m_fScaleFactor );

	// mesh
	{
		RenderMeshes( pDevCon, matWorld, false );
	}
}

//------------------------------------------------------------------------
void CScene::RenderScene(ID3D11DeviceContext* pDevCon)
{
	// Clear stats
	m_nTriangles = 0;
	m_nVertices = 0;
	

	//------------------------------------------------------------------------
	// Deferred Shading.
	//------------------------------------------------------------------------
	PIXEvent evt(L"Scene Rendering");

	// Bind GBuffer VS
	m_VS_GBuffer->Bind(pDevCon);

	// Matrix for translation objects
	Mat44 matWorld;
	MatrixScaling(&matWorld, m_fScaleFactor, m_fScaleFactor, m_fScaleFactor );

	// Sponza
	{
		m_PS_GBuffer_TexAlbedo_TexNorm->Bind(pDevCon);

		SCBMaterial sponzaMat;

		sponzaMat.m_MaterialID = 0;
		sponzaMat.m_Roughness = 0.5f;
		sponzaMat.m_Metallic = 0.04f;


		m_CBufferMaterial.GetBufferData() = sponzaMat;
		m_CBufferMaterial.UpdateBuffer(pDevCon);
		m_CBufferMaterial.SetPS(pDevCon, 5);

		RenderMeshes(pDevCon, matWorld);
	}
	
}



