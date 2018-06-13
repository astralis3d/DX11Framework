#include "PCH.h"
#include "Skybox.h"
#include "Utility.h"
#include "D3DStates.h"

CSkybox::CSkybox()
	: m_pBS_Clouds(nullptr)
{
	// hard-coded clouds params

	// ** data layout: **
	// float4 cloudsParams0;		// xy: uv scale, zw - move dir
	// float4 cloudsParams1;		// x: move speed, y: perlin scale, z: brightness, w: unused

	
	auto& clouds0 = m_cloudsData[0];
	clouds0.params0 = Vec4( 1.52, 1.52, 0.05f, -0.16f );
	clouds0.params1 = Vec4( 0.35f, 0.08f, 1.85, 0.3 );

	auto& clouds1 = m_cloudsData[1];
	clouds1.params0 = Vec4( 0.85f, 0.85f, 0.10f, -0.15f );
	clouds1.params1 = Vec4( 0.75f, -0.1, 1, 0 );


	auto& clouds2 = m_cloudsData[2];
	clouds2.params0 = Vec4( 0.865, 0.865, 0.12f, -0.14f );
	clouds2.params1 = Vec4( 0.38, -0.19, 0.87, 0.5 );
}

//------------------------------------------------------------------------
CSkybox::~CSkybox()
{
	Release();
}

//------------------------------------------------------------------------
void CSkybox::Initialize(ID3D11Device* pDevice)
{
	m_CBPerObject.Create(pDevice);
	m_CBClouds.Create(pDevice);

	// Create shaders
	m_pixelShader = CompilePSFromFile( pDevice, "..//Common//Skybox.hlsl", "PS" );
	m_pixelShaderClouds[0] = CompilePSFromFile( pDevice, "..//Common//Skybox.hlsl", "PS_Clouds" );
	m_pixelShaderClouds[1] = CompilePSFromFile( pDevice, "..//Common//Skybox.hlsl", "PS_Clouds2" );
	m_vertexShader = CompileVSFromFile( pDevice, "..//Common//Skybox.hlsl", "VS" );

	{
		CD3D11ShaderMacro macros;
		macros.AddMacro("USE_ENVMAP", "1");
		macros.Finish();

		m_pPixelShaderEnvmap = CompilePSFromFile(pDevice, "..//Common//Skybox.hlsl", "PS", &macros);
	}

	m_pixelShaderCIEClearSky = CompilePSFromFile( pDevice, "..//Common//Skybox.hlsl", "PS_CIEClearSky");

	
	const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, 12,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		1, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT,		1, 12,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BITANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT,		1, 24,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	HRESULT hr = pDevice->CreateInputLayout( InputLayoutDesc, ARRAYSIZE( InputLayoutDesc ), m_vertexShader->Shader()->GetBlob()->GetBufferPointer(), m_vertexShader->Shader()->GetBlob()->GetBufferSize(), &m_pInputLayout );
	V( hr );

	
	// Skybox mesh
	m_meshSkybox.Initialize(pDevice);
	m_meshSkybox.ImportFile("..//..//media//skybox.obj");
	//m_meshSkybox.ImportFile( "..//..//media//w3_skybox.obj" );
	m_meshClouds.Initialize(pDevice);
	m_meshClouds.ImportFile("..//..//media//clouds.obj");
	
	// Textures
	//m_texDiffuse.LoadTexture(pDevice, "..//..//media//Textures//Sky//skybox_lakeside.dds");					
	//m_texDiffuse.LoadTexture(pDevice, "..//..//media//Textures//Sky//sky_storm.dds");							
	//m_texDiffuse.LoadTexture(pDevice, "..//..//media//Textures//Sky//sky-photo-new-night.png");				
	//m_texDiffuse.LoadTexture(pDevice, "..//..//media//Textures//Sky//Skymap_goodmorning_beautiful_gen2.png");
	//m_texDiffuse.LoadTexture(pDevice, "..//..//media//Textures//Sky//milkyway.png");		
	//m_texDiffuse.LoadTexture(pDevice, "..//..//media//Textures//Sky//skybox_cloudy.dds");						


	m_texFalloff.LoadTexture(pDevice, "..//..//media//skybox_falloff.dds");

	m_texClouds[0].LoadTexture( pDevice, "..//..//media//!clouds_normal.dds" );
	m_texClouds[1].LoadTexture( pDevice, "..//..//media//w3_clouds.dds" );

	m_texPerlin.LoadTexture(pDevice, "..//..//media//!clouds_noise.dds");
	m_texNormals.LoadTexture(pDevice, "..//..//media//w3_clouds_normal.dds");

	// For cubemaps
	const int CUBEMAP_SIZE = 256;

	ID3D11Texture2D* pCubeMap = nullptr;	
	D3D11_TEXTURE2D_DESC CubeMapDesc;
	CubeMapDesc.Width = CUBEMAP_SIZE;
	CubeMapDesc.Height = CUBEMAP_SIZE;
	CubeMapDesc.ArraySize = 6;
	CubeMapDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	CubeMapDesc.CPUAccessFlags = 0;
	CubeMapDesc.MipLevels = 0;
	CubeMapDesc.Usage = D3D11_USAGE_DEFAULT;
	CubeMapDesc.SampleDesc.Count = 1;
	CubeMapDesc.SampleDesc.Quality = 0;
	CubeMapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	CubeMapDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE;

	pDevice->CreateTexture2D(&CubeMapDesc, nullptr, &pCubeMap);

	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = CubeMapDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.MipSlice = 0;
	rtvDesc.Texture2DArray.ArraySize = 1;
	for (int i=0; i < 6; ++i)
	{
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		
		pDevice->CreateRenderTargetView(pCubeMap, &rtvDesc, &m_cubeMapRTV[i]);
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = CubeMapDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.MipLevels = 1;

	pDevice->CreateShaderResourceView(pCubeMap, &srvDesc, &m_cubeMapSRV);

	SAFE_RELEASE(pCubeMap);




	D3D11_DEPTH_STENCIL_DESC dsDesc;
	dsDesc.StencilEnable = FALSE;
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthFunc = D3D11_COMPARISON_EQUAL;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;

	pDevice->CreateDepthStencilState(&dsDesc, &m_pSkyboxDSState);


	// Blending state for clouds
	{
		D3D11_BLEND_DESC blendState = {};
		blendState.AlphaToCoverageEnable = FALSE;
		blendState.IndependentBlendEnable = FALSE; // new in D3D11

		auto& rtDesc = blendState.RenderTarget[0];
		rtDesc.BlendEnable = TRUE;
		rtDesc.SrcBlend = D3D11_BLEND_ONE;
		rtDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		rtDesc.BlendOp = D3D11_BLEND_OP_ADD;
		rtDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		rtDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
		rtDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
		rtDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		pDevice->CreateBlendState( &blendState, &m_pBS_Clouds );
	}

	CD3D11_RASTERIZER_DESC rsDesc = CD3D11_RASTERIZER_DESC( CD3D11_DEFAULT() );
	rsDesc.CullMode = D3D11_CULL_FRONT;
	pDevice->CreateRasterizerState(&rsDesc, &m_pSkyboxRSState);



	D3D11_TEXTURE2D_DESC DepthDesc;
	DepthDesc.Width = DepthDesc.Height = CUBEMAP_SIZE;
	DepthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	DepthDesc.Usage = D3D11_USAGE_DEFAULT;
	DepthDesc.ArraySize = 1;
	DepthDesc.MipLevels = 1;
	DepthDesc.SampleDesc.Count = 1;
	DepthDesc.SampleDesc.Quality = 0;
	DepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	DepthDesc.MiscFlags = 0;
	DepthDesc.CPUAccessFlags = 0;

	ID3D11Texture2D* pDepthCubeMap = nullptr;
	pDevice->CreateTexture2D(&DepthDesc, nullptr, &pDepthCubeMap);

	D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc;
	depthViewDesc.Format = DepthDesc.Format;
	depthViewDesc.Flags = 0;
	depthViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthViewDesc.Texture2D.MipSlice = 0;

	pDevice->CreateDepthStencilView(pDepthCubeMap, &depthViewDesc, &m_cubeMapDSV);

	m_cubeMapViewport.Width = (float) CUBEMAP_SIZE;
	m_cubeMapViewport.Height = (float) CUBEMAP_SIZE;
	m_cubeMapViewport.TopLeftX = 0.f;
	m_cubeMapViewport.TopLeftY = 0.f;
	m_cubeMapViewport.MinDepth = 0.f;
	m_cubeMapViewport.MaxDepth = 1.f;

	SAFE_RELEASE(pDepthCubeMap);

	// Generate matrices
	{
		Vec3 center;

		Vec3 targets[6] =
		{
			Vec3(center.x + 1.f, center.y, center.z),			// +X
			Vec3(center.x - 1.f, center.y, center.z),			// -X
			Vec3(center.x, center.y + 1.f, center.z),			// +Y
			Vec3(center.x, center.y - 1.f, center.z),			// -Y
			Vec3(center.x, center.y, center.z + 1.f),	// +Z
			Vec3(center.x, center.y, center.z - 1.f),	// -Z
		};

		Vec3 ups[6] =
		{
			Vec3(0.f, 1.f, 0.f),
			Vec3(0.f, 1.f, 0.f),
			Vec3(0.f, 0.f, -1.f),
			Vec3(0.f, 0.f, 1.f),
			Vec3(0.f, 1.f, 0.f),
			Vec3(0.f, 1.f, 0.f),
		};

		for (int i = 0; i < 6; ++i)
			MatrixLookAtLH(center, targets[i], ups[i], &m_cubeMapMatView[i]);
	}
	MatrixPerspectiveFovLH(PI * 0.5f, 1.f, 0.1f, 3000.f, &m_cubeMapMatProj);

}

//----------------------------------------------------------------------
void CSkybox::Release()
{
	m_CBPerObject.Release();
	m_CBClouds.Release();
	
	// Textures
	m_texCubemap.ReleaseTexture();
	m_texDiffuse.ReleaseTexture();
	m_texFalloff.ReleaseTexture();
	m_texClouds[0].ReleaseTexture();
	m_texClouds[1].ReleaseTexture();
	m_texPerlin.ReleaseTexture();
	m_texNormals.ReleaseTexture();

	SAFE_RELEASE( m_pSkyboxRSState );

	m_meshSkybox.Clear();
	m_meshClouds.Clear();

	SAFE_RELEASE(m_pInputLayout);

	SAFE_RELEASE(m_pSkyboxDSState);
	SAFE_RELEASE( m_pBS_Clouds );

	for (int i=0; i < 6; ++i)
		SAFE_RELEASE(m_cubeMapRTV[i]);

	SAFE_RELEASE(m_cubeMapSRV);
	SAFE_RELEASE(m_cubeMapDSV);
}

//----------------------------------------------------------------------
void CSkybox::Render(ID3D11DeviceContext* pDevContext, const Vec3& eye, const Mat44& matViewProj, bool bRenderClouds, bool bEnvmap, bool bCIEClearSky)
{
	m_vertexShader->Shader()->Bind( pDevContext );

	ID3D11ShaderResourceView* pSRVs[6] = 
	{	m_texCubemap.GetSRV(), 
		m_texDiffuse.GetSRV(),
		m_texFalloff.GetSRV(),
		nullptr,
		m_texPerlin.GetSRV(), 
		m_texNormals.GetSRV() 
	};

	pDevContext->PSSetShaderResources( 0, 6, pSRVs );

	pDevContext->OMSetDepthStencilState( m_pSkyboxDSState, 0 );


	// Set constant buffers
	Mat44 matWorld;
	MatrixTranslation(eye.x, eye.y, eye.z, &matWorld);

	Mat44 matScaling;
	MatrixScaling( &matScaling, 100.0f, 100.0f, 100.0f );

	MatrixMult( matScaling, matWorld, &m_CBPerObject.GetBufferData().mWorld );

	ID3D11Buffer* pVertexBuffers[2] = { m_meshSkybox.GetVertexBufferStream1(), m_meshSkybox.GetVertexBufferStream2() };
	UINT strides[2] = { sizeof( SceMeshVertexStream1), sizeof( SceMeshVertexStream2 ) };
	UINT offsets[2] = { 0, 0 };
	
	//m_CBPerObject.GetBufferData().mWorld = matWorld;
	m_CBPerObject.GetBufferData().mViewProj = matViewProj;
	m_CBPerObject.UpdateBuffer(pDevContext);
	m_CBPerObject.SetVS(pDevContext, 2);
	
	// Set proper pixel shader
	if (bCIEClearSky)
	{
		pDevContext->PSSetShader( *m_pixelShaderCIEClearSky->Shader(), nullptr, 0 );
	}
	else
	{
		pDevContext->PSSetShader( (bEnvmap) ? *m_pPixelShaderEnvmap->Shader() : *m_pixelShader->Shader(), nullptr, 0 );
	}


	//pDevContext->RSSetState( m_pSkyboxRSState );
	const unsigned int numSubmeshes = m_meshSkybox.GetNumSubmeshes();
	for (unsigned int i=0; i < numSubmeshes; ++i)
	{
		const CMeshChunk& Submesh = m_meshSkybox.GetSubmesh(i);

		pDevContext->DrawIndexed(Submesh.m_numIndices, 0, 0);
	}	
	//pDevContext->RSSetState( 0 );

	
	// Clouds
	if (bRenderClouds)
	{
		float BlendFactor[4] = { 1, 1, 1, 1 };

		for ( uint32 index=1; index < 2; ++index )
		{
			Mat44 matScaling, matTranslate;
			MatrixScaling(&matScaling, 1.0f, 1.0f, 1.0f);			
			MatrixTranslation( eye.x, eye.y - 300, eye.z, &matTranslate );
			MatrixMult( matScaling, matTranslate, &m_CBPerObject.GetBufferData().mWorld);



			m_CBPerObject.UpdateBuffer( pDevContext );
			m_CBPerObject.SetVS( pDevContext, 2 );

			m_CBClouds.GetBufferData() = m_cloudsData[index];
			m_CBClouds.UpdateBuffer(pDevContext);
			m_CBClouds.SetPS(pDevContext, 5);

			ID3D11ShaderResourceView* pSRVClouds =  m_texClouds[index].GetSRV();
			pDevContext->PSSetShaderResources(3, 1, &pSRVClouds);
			
			pDevContext->OMSetBlendState( m_pBS_Clouds, BlendFactor, 0xffffffff );

			pVertexBuffers[0] = m_meshClouds.GetVertexBufferStream1();
			pVertexBuffers[1] = m_meshClouds.GetVertexBufferStream2();
			pDevContext->IASetVertexBuffers( 0, 2, pVertexBuffers, strides, offsets );
			pDevContext->IASetIndexBuffer( m_meshClouds.GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0 );

			m_pixelShaderClouds[index]->Shader()->Bind( pDevContext );

			for (unsigned int i = 0; i < 1; ++i)
			{
				const CMeshChunk& Submesh = m_meshClouds.GetSubmesh( i );

				const uint32 drawCalls = ( index == 0 ) ? 1 : 2;
				for ( uint32 j=0; j < drawCalls; ++j)
				{
					pDevContext->DrawIndexed( Submesh.m_numIndices, 0, 0 );
				}				
			}			
		}

		pDevContext->OMSetBlendState( 0, BlendFactor, 0xffffffff );
	}	
}

//----------------------------------------------------------------------
void CSkybox::RenderCubeMap(ID3D11DeviceContext* pDevContext)
{
	// Get viewport(s)
	UINT numViewports = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	D3D11_VIEWPORT oldViewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
	pDevContext->RSGetViewports(&numViewports, oldViewports);

	// Get old RTV and DSV
	ID3D11RenderTargetView* pOldRTV = nullptr;
	ID3D11DepthStencilView* pOldDSV = nullptr;
	pDevContext->OMGetRenderTargets(1, &pOldRTV, &pOldDSV);

	float ClearColor[4] = {0.0f, 0.0f, 0.0f, 1.f};
	for (int i=0; i < 6; ++i)
	{			
		pDevContext->ClearRenderTargetView(m_cubeMapRTV[i], ClearColor);
		pDevContext->ClearDepthStencilView(m_cubeMapDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

		ID3D11RenderTargetView* pRTVs[1] = { m_cubeMapRTV[i] };
		pDevContext->OMSetRenderTargets(1, pRTVs, m_cubeMapDSV);

		pDevContext->RSSetViewports(1, &m_cubeMapViewport);

		Mat44 skyViewProj;
		MatrixMult(m_cubeMapMatView[i], m_cubeMapMatProj, &skyViewProj);
		/*m_CBPerView.GetBufferData().mProj = m_cubeMapMatProj;
		m_CBPerView.GetBufferData().mView = m_cubeMapMatView[i];
		m_CBPerView.UpdateBuffer(pDevContext);
		m_CBPerView.SetVS(pDevContext, 1);*/

		Vec3 center;
		//Render(pDevContext, center, skyViewProj, TODO, TODO, TODO);
	}

	pDevContext->OMSetRenderTargets(1, &pOldRTV, pOldDSV);

	SAFE_RELEASE(pOldRTV);
	SAFE_RELEASE(pOldDSV);

	pDevContext->GenerateMips(m_cubeMapSRV);

	pDevContext->RSSetViewports(numViewports, oldViewports);
}

//----------------------------------------------------------------------
void CSkybox::PreRender(ID3D11DeviceContext* pDevContext)
{
	// Set vertex & index buffers, as well as input layout...
	ID3D11Buffer* pVertexBuffers[2] = { m_meshSkybox.GetVertexBufferStream1(), m_meshSkybox.GetVertexBufferStream2() };
	UINT strides[2] = { sizeof( SceMeshVertexStream1 ), sizeof( SceMeshVertexStream2 ) };
	UINT offsets[2] = { 0, 0 };

	pDevContext->IASetVertexBuffers( 0, 2, pVertexBuffers, strides, offsets );
	pDevContext->IASetIndexBuffer( m_meshSkybox.GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0 );
	pDevContext->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );
	pDevContext->IASetInputLayout( m_pInputLayout );
}

//------------------------------------------------------------------------
void CSkybox::SetTextureSphere(ID3D11Device* pDevice, const char* path)
{
	m_texDiffuse.ReleaseTexture();
	m_texDiffuse.LoadTexture(pDevice, path);
}

//------------------------------------------------------------------------
void CSkybox::SetTextureCubemap(ID3D11Device* pDevice, const char* path)
{
	m_texCubemap.ReleaseTexture();
	m_texCubemap.LoadTexture(pDevice, path);
}

//------------------------------------------------------------------------
void CSkybox::DrawSkySphereOnly( ID3D11DeviceContext* pDevContext )
{
	const unsigned int numSubmeshes = m_meshSkybox.GetNumSubmeshes();
	for (unsigned int i = 0; i < numSubmeshes; ++i)
	{
		const CMeshChunk& Submesh = m_meshSkybox.GetSubmesh( i );

		pDevContext->DrawIndexed( Submesh.m_numIndices, 0, 0 );
	}
}

