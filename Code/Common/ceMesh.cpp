#include "PCH.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


#include <Shlwapi.h>
#include <memory>
#include "ceMesh.h"


//----------------------------------------------------------------------
CceMesh::CceMesh()
	: m_pIndexBuffer(nullptr)
	, m_pVertexBufferStream1(nullptr)
	, m_pVertexBufferStream2(nullptr)
	, m_pDevice(nullptr)
	, m_bUseRelativeTexturePaths(true)
	, m_strFileCurrDirectory()
{
	Clear();
}

//----------------------------------------------------------------------
CceMesh::CceMesh(ID3D11Device* pDevice)
	: m_pIndexBuffer(nullptr)
	, m_pVertexBufferStream1(nullptr)
	, m_pVertexBufferStream2(nullptr)
	, m_pDevice(pDevice)
	, m_bUseRelativeTexturePaths(true)
	, m_strFileCurrDirectory()
{
	Clear();
}

//----------------------------------------------------------------------
CceMesh::~CceMesh()
{
	Clear();
}

//----------------------------------------------------------------------
bool CceMesh::ImportMeshFromFile(const char* pszPath, bool bImportTextures, bool bCreateBuffers)
{
	// Load mesh from existing *.cemesh file.
	// Loading mesh is usually quite a large task, so here is divided to a few phases.

	// 1. Make sure requested file exists. Also get directory of file.
	FILE* pFile = nullptr;
	fopen_s(&pFile, pszPath, "rb");
	if (!pFile)
		return false;

	// 2. Load header. Make sure that 'magic' is valid.
	fread( (void*) &m_meshHeader, sizeof(SceMeshFileHeader), 1, pFile);
	if (strcmp(m_meshHeader.magic, MESH_HEADER_MAGIC) != 0)
	{
		// Incompatible file!
		// @TODO: Log

		return false;
	}

	// 3. Load all needed info from file header
	const unsigned int NUM_VERTICES =	m_nVerticesCnt	=	m_meshHeader.m_nTotalVerts;
	const unsigned int NUM_INDICES =	m_nIndicesCnt	=	m_meshHeader.m_nTotalIndices;
	const unsigned int NUM_SUBMESHES =	m_nSubmeshesCnt	=	m_meshHeader.m_nSubmeshes;
	const unsigned int NUM_MATERIALS =	m_nMaterialCnt	=	m_meshHeader.m_nMaterials;

	//m_vertices.resize(NUM_VERTICES);
	m_verticesStream1.resize(NUM_VERTICES);
	m_verticesStream2.resize(NUM_VERTICES);

	m_indices.resize(NUM_INDICES);
	m_submeshes.resize(NUM_SUBMESHES);
	m_materials.resize(NUM_MATERIALS);

	// 4. Import info about materials
	fread( (void*) &m_materials[0], sizeof(SceMeshMaterial), NUM_MATERIALS, pFile);

	// 5. Import info about submeshes
	fread( (void*) &m_submeshes[0], sizeof(CMeshChunk), NUM_SUBMESHES, pFile);

	// 6. Import vertices
	fread( (void*) &m_verticesStream1[0], sizeof(SceMeshVertexStream1), NUM_VERTICES, pFile);
	fread( (void*) &m_verticesStream2[0], sizeof(SceMeshVertexStream2), NUM_VERTICES, pFile);

	// 7. Import indices
	fread( (void*) &m_indices[0], sizeof(TIndex), NUM_INDICES, pFile);

	// 8. Create buffers & textures
	if (bCreateBuffers)
		CreateBuffers();

	if (bImportTextures)
		CreateTextures();

	// 9. Done!
	fclose(pFile);

	// test
	//char chTest[16];
	//sprintf_s(chTest, "%d\n", m_nSubmeshesCnt);
	//OutputDebugStringA(chTest);

	return true;
}

//----------------------------------------------------------------------
bool CceMesh::ExportMeshToFile(const char* pszPath)
{
	// Export mesh to *.cemesh file.

	// This is also pretty complicated task, so it's also divided in phases

	// 1. Create FILE*. Exit if fails.
	FILE* pFile = 0;
	fopen_s(&pFile, pszPath, "wb");
	if (!pFile)
		return false;

	// 2. Export header
	fwrite( (const void*) &m_meshHeader, sizeof(SceMeshFileHeader), 1, pFile);

	// 3. Export info about materials
	fwrite( (const void*) &m_materials[0], sizeof(SceMeshMaterial), m_materials.size(), pFile);

	// 4. Export info about submeshes
	fwrite( (const void*) &m_submeshes[0], sizeof(CMeshChunk), m_submeshes.size(), pFile);

	// 5. Export vertices
	fwrite( (const void*) &m_verticesStream1[0], sizeof(SceMeshVertexStream1), m_verticesStream1.size(), pFile);
	fwrite( (const void*) &m_verticesStream2[0], sizeof(SceMeshVertexStream2), m_verticesStream1.size(), pFile);

	// 6. Export indices
	fwrite( (const void*) &m_indices[0], sizeof(TIndex), m_indices.size(), pFile);

	// 7. Done!
	fclose(pFile);


	return true;
}

//----------------------------------------------------------------------
bool CceMesh::ImportRawData(const char* pszPath, bool bLeftHanded, bool bImportTextures, bool bCreateBuffers)
{
	// Load raw data - use Assimp, which is truly an extemely great and still developed library!


	
	
	unsigned int flags = (	aiProcess_CalcTangentSpace |
							aiProcess_JoinIdenticalVertices |
							aiProcess_GenSmoothNormals |
							/*aiProcess_ValidateDataStructure | */
							aiProcess_ImproveCacheLocality | 
							aiProcess_FindInstances |
							/*aiProcess_RemoveRedundantMaterials | */
							aiProcess_Triangulate |
							aiProcess_SortByPType |
							aiProcess_OptimizeGraph |
							/* aiProcess_FindInvalidData | */
							aiProcess_SplitLargeMeshes		
							);
	

							
	if (bLeftHanded)
		flags |= aiProcess_ConvertToLeftHanded;

	Assimp::Importer importer;
	auto pScene = importer.ReadFile(pszPath, flags);
	
	if (!pScene)
	{
		DebugPrintf("%s\n", importer.GetErrorString() );
		return false;
	}

	// Info about scene
	const bool HAS_MESHES = pScene->HasMeshes();
	const bool HAS_MATERIALS = pScene->HasMaterials();

	unsigned int NUM_MATERIALS = 0;
	unsigned int NUM_SUBMESHES = 0;
	unsigned int NUM_VERTICES_TOTAL = 0;
	unsigned int NUM_INDICES_TOTAL = 0;

	uint32 iTexturesCounter = 0;

	// Import materials at first
	if (HAS_MATERIALS)
	{
		NUM_MATERIALS = pScene->mNumMaterials;

		//char buffer[256];
		//GetCurrentDirectoryA(256, buffer);
		//OutputDebugStringA(buffer);
				
		for (unsigned int i=0; i < NUM_MATERIALS; ++i)
		{
			aiMaterial* pMaterial = pScene->mMaterials[i];
			SceMeshMaterial mat;

			// For general usage
			aiString str;
			aiColor3D color3d;
			
					
			// ID of material
			mat.m_id = i;

			// Get mat name				
			pMaterial->Get(AI_MATKEY_NAME, str);
			//strcpy_s(mat.m_name, 30, str.C_Str());
			strncpy_s(mat.m_name, str.C_Str(), 31);

			// Ambient color				
			pMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color3d);
			mat.m_clrAmbient.x = color3d.r;
			mat.m_clrAmbient.y = color3d.g;
			mat.m_clrAmbient.z = color3d.b;

			// Diffuse color
			pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color3d);
			mat.m_clrDiffuse.x = color3d.r;
			mat.m_clrDiffuse.y = color3d.g;
			mat.m_clrDiffuse.z = color3d.b;

			// Specular color
			pMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color3d);
			mat.m_clrSpecular.x = color3d.r;
			mat.m_clrSpecular.y = color3d.g;
			mat.m_clrSpecular.z = color3d.b;

			// Emissive color
			pMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color3d);
			mat.m_clrEmissive.x = color3d.r;
			mat.m_clrEmissive.y = color3d.g;
			mat.m_clrEmissive.z = color3d.b;

			// Transparent color
			pMaterial->Get(AI_MATKEY_COLOR_TRANSPARENT, color3d);
			mat.m_clrTransparent.x = color3d.r;
			mat.m_clrTransparent.y = color3d.g;
			mat.m_clrTransparent.z = color3d.b;

			// Wireframe?
			pMaterial->Get(AI_MATKEY_ENABLE_WIREFRAME, mat.m_nWireframe);

			// Twosided?
			pMaterial->Get(AI_MATKEY_TWOSIDED, mat.m_nTwosided);

			// Opacity
			pMaterial->Get(AI_MATKEY_OPACITY, mat.m_fOpacity);

			// Shininess & Shininess Strength
			pMaterial->Get(AI_MATKEY_SHININESS, mat.m_fShininess);
			pMaterial->Get(AI_MATKEY_SHININESS_STRENGTH, mat.m_fShininessStrength);

			// Checking for textures for particular material
			const unsigned int NUM_TEXTURES = 8;

			for (unsigned int j=0; j < NUM_TEXTURES; ++j)
			{
				//char buffer[128];
				aiString texStr;

				if (AI_SUCCESS == pMaterial->Get(AI_MATKEY_TEXTURE_AMBIENT(j), texStr))
				{
					strcpy_s(mat.m_textures[0].path, 128, texStr.C_Str());
					/*sprintf_s(buffer, 128, "Found an Ambient texture (stack: %d) for material #%d (%s)\n", j, i, mat.m_name);
					OutputDebugStringA(buffer);*/

					mat.m_textures[0].m_texType = eTEXTYPE_AMBIENT;
					mat.m_textures[0].bIsValid = true;
					texStr.Clear();						
				}

				if (AI_SUCCESS == pMaterial->Get(AI_MATKEY_TEXTURE_DIFFUSE(j), texStr))
				{
					strcpy_s(mat.m_textures[1].path, 128, texStr.C_Str());
				/*	sprintf_s(buffer, 128, "Found a Diffuse texture (stack: %d) for material #%d (%s)\n", j, i, mat.m_name);
					OutputDebugStringA(buffer);*/

					mat.m_textures[1].m_texType = eTEXTYPE_DIFFUSE;
					mat.m_textures[1].bIsValid = true;
					texStr.Clear();
				}

				if (AI_SUCCESS == pMaterial->Get(AI_MATKEY_TEXTURE_DISPLACEMENT(j), texStr))
				{
					strcpy_s(mat.m_textures[2].path, 128, texStr.C_Str());
					/*sprintf_s(buffer, 128, "Found a Displacement texture (stack: %d) for material #%d (%s)\n", j, i, mat.m_name);
					OutputDebugStringA(buffer);*/

					mat.m_textures[2].m_texType = eTEXTYPE_DISPLACEMENT;
					mat.m_textures[2].bIsValid = true;
					texStr.Clear();
				}

				if (AI_SUCCESS == pMaterial->Get(AI_MATKEY_TEXTURE_EMISSIVE(j), texStr))
				{
					strcpy_s(mat.m_textures[3].path, 128, texStr.C_Str());
				/*	sprintf_s(buffer, 128, "Found a Emissive texture (stack: %d) for material #%d (%s)\n", j, i, mat.m_name);
					OutputDebugStringA(buffer);*/

					mat.m_textures[3].m_texType = eTEXTYPE_EMISSIVE;
					mat.m_textures[3].bIsValid = true;
					texStr.Clear();
				}

				if (AI_SUCCESS == pMaterial->Get(AI_MATKEY_TEXTURE_HEIGHT(j), texStr))
				{
					strcpy_s(mat.m_textures[4].path, 128, texStr.C_Str());
				/*	sprintf_s(buffer, 128, "Found a Height texture (stack: %d) for material #%d (%s)\n", j, i, mat.m_name);
					OutputDebugStringA(buffer);*/

					mat.m_textures[4].m_texType = eTEXTYPE_HEIGHT;
					mat.m_textures[4].bIsValid = true;
					texStr.Clear();
				}

				if (AI_SUCCESS == pMaterial->Get(AI_MATKEY_TEXTURE_LIGHTMAP(j), texStr))
				{
					strcpy_s(mat.m_textures[5].path, 128, texStr.C_Str());
					/*sprintf_s(buffer, 128, "Found a Lightmap texture (stack: %d) for material #%d (%s)\n", j, i, mat.m_name);
					OutputDebugStringA(buffer);*/

					mat.m_textures[5].m_texType = eTEXTYPE_LIGHTMAP;
					mat.m_textures[5].bIsValid = true;
					texStr.Clear();
				}

				if (AI_SUCCESS == pMaterial->Get(AI_MATKEY_TEXTURE_NORMALS(j), texStr))
				{
					strcpy_s(mat.m_textures[6].path, 128, texStr.C_Str());
					/*sprintf_s(buffer, 128, "Found a Normal texture (stack: %d) for material #%d (%s)\n",j,  i, mat.m_name);
					OutputDebugStringA(buffer);*/

					mat.m_textures[6].m_texType = eTEXTYPE_NORMAL;
					mat.m_textures[6].bIsValid = true;
					texStr.Clear();
				}

				if (AI_SUCCESS == pMaterial->Get(AI_MATKEY_TEXTURE_OPACITY(j), texStr))
				{
					strcpy_s(mat.m_textures[7].path, 128, texStr.C_Str());
					/*sprintf_s(buffer, 128, "Found an Opacity texture (stack: %d) for material #%d (%s)\n", j, i, mat.m_name);
					OutputDebugStringA(buffer);*/

					mat.m_textures[7].m_texType = eTEXTYPE_OPACITY;
					mat.m_textures[7].bIsValid = true;
					texStr.Clear();
				}

				if (AI_SUCCESS == pMaterial->Get(AI_MATKEY_TEXTURE_SPECULAR(j), texStr))
				{
					strcpy_s(mat.m_textures[8].path, 128, texStr.C_Str());
					/*sprintf_s(buffer, 128, "Found a Specular texture (stack: %d) for material #%d (%s)\n", j, i, mat.m_name);
					OutputDebugStringA(buffer);*/

					mat.m_textures[8].m_texType = eTEXTYPE_SPECULAR;
					mat.m_textures[8].bIsValid = true;
					texStr.Clear();
				}				
			}

			m_materials.push_back(mat);
		}

	} // if (HAS_MATERIALS)

	if (HAS_MESHES)
	{
		NUM_SUBMESHES = pScene->mNumMeshes;
		TSubmeshes& vSubmeshes = m_submeshes;
		vSubmeshes.resize(NUM_SUBMESHES);

		TVerticesStream1& verticesStream1 = m_verticesStream1;
		TVerticesStream2& verticesStream2 = m_verticesStream2;
		TIndices&	vIndices = m_indices;
		
		for (unsigned int i=0; i < NUM_SUBMESHES; ++i)
		{
			CMeshChunk& submesh = vSubmeshes[i];
			aiMesh* pMesh = pScene->mMeshes[i];

			//unsigned int nNumBones = pMesh->mNumBones;
			
			// For each submesh bbox is defined - for culling
			Vec3 aabbMin(FLT_MAX, FLT_MAX, FLT_MAX);
			Vec3 aabbMax(FLT_MIN, FLT_MIN, FLT_MIN);

			// Info about currently processed submesh

			// calc indices in submesh
			int numIndices = 0;
			for (unsigned int j=0; j < pMesh->mNumFaces; ++j)
			{
				aiFace& f = pMesh->mFaces[j];
				numIndices += f.mNumIndices;
			}

			const unsigned int NUM_VERTICES = pMesh->mNumVertices;
			const unsigned int NUM_INDICES =  numIndices; //pMesh->mNumFaces * 3;		// Always triangulated, so IndexCnt = Faces * 3
			const unsigned int NUM_FACES = pMesh->mNumFaces;
			const bool HAS_BITANGENTS_TANGENTS = pMesh->HasTangentsAndBitangents();
			const bool HAS_NORMALS = pMesh->HasNormals();
			const bool HAS_POSITIONS = pMesh->HasPositions();

			// Update data neccessary for indexed drawing
			submesh.m_baseVertexLocation = NUM_VERTICES_TOTAL;
			submesh.m_startIndexLocation = NUM_INDICES_TOTAL;

			NUM_VERTICES_TOTAL += NUM_VERTICES;
			NUM_INDICES_TOTAL += NUM_INDICES;
			
			// Fill obvious data
			submesh.m_materialID = pMesh->mMaterialIndex;
			submesh.m_numVertices = NUM_VERTICES;
			submesh.m_numIndices = NUM_INDICES;
			strcpy_s(submesh.m_name, 32, pMesh->mName.C_Str());

			// Load vertices
			for (unsigned int j=0; j < NUM_VERTICES; ++j)
			{
				SceMeshVertexStream1	vertexStream1;
				SceMeshVertexStream2	vertexStream2;

				if (HAS_POSITIONS)
				{
					memcpy(&vertexStream1.m_position, &pMesh->mVertices[j], sizeof(Vec3));

					// Assign the first vertex position for bbox;
					if (j == 0)
					{
						memcpy(&submesh.m_min, &pMesh->mVertices[0], sizeof(Vec3));
						memcpy(&submesh.m_max, &pMesh->mVertices[0], sizeof(Vec3));
					}
				}

				if (HAS_NORMALS)
					memcpy(&vertexStream2.m_normal, &pMesh->mNormals[j], sizeof(Vec3));

				if (HAS_BITANGENTS_TANGENTS)
				{
					memcpy(&vertexStream2.m_bitangent, &pMesh->mBitangents[j], sizeof(Vec3));
					memcpy(&vertexStream2.m_tangent, &pMesh->mTangents[j], sizeof(Vec3));
				}

				// Texcoords
				const bool HAS_TEXCOORDS = pMesh->HasTextureCoords(0);
				if (HAS_TEXCOORDS)
				{
					Vec2 tex(pMesh->mTextureCoords[0][j].x, pMesh->mTextureCoords[0][j].y);
					memcpy(&vertexStream1.m_texcoord, &tex, sizeof(Vec2));
				}			

				// Bounding Box

				if  (vertexStream1.m_position.x < aabbMin.x)
					aabbMin.x = vertexStream1.m_position.x;
				if  (vertexStream1.m_position.y < aabbMin.y)
					aabbMin.y = vertexStream1.m_position.y;
				if  (vertexStream1.m_position.z < aabbMin.z)
					aabbMin.z = vertexStream1.m_position.z;

				if  (vertexStream1.m_position.x > aabbMax.x)
					aabbMax.x = vertexStream1.m_position.x;
				if  (vertexStream1.m_position.y > aabbMax.y)
					aabbMax.y = vertexStream1.m_position.y;
				if  (vertexStream1.m_position.z > aabbMax.z)
					aabbMax.z = vertexStream1.m_position.z;

				verticesStream1.push_back(vertexStream1);
				verticesStream2.push_back(vertexStream2);
			}

			// Now set bbox
			memcpy(&submesh.m_min, &aabbMin, sizeof(Vec3));
			memcpy(&submesh.m_max, &aabbMax, sizeof(Vec3));

			//char bfr[128];
			//sprintf_s(bfr, 127, "(%f, %f, %f), (%f, %f, %f)\n", submesh.m_min.x, submesh.m_min.y, submesh.m_min.z, submesh.m_max.x, submesh.m_max.y, submesh.m_max.z);
			//OutputDebugStringA(bfr);

			// Okay, now let's load indices, index type is USHORT (16 bits, 2^16) - to support large meshes (like Sponza)
			// @TODO: In fact, we could consider drawing one large mesh as larger number of smaller submeshes (DXGI_FORMAT_R16_UINT instead of DXGI_FORMAT_R32_UINT) - Assimp can do it. We have to check it out later!
			for (size_t j=0; j < NUM_FACES; ++j)
			{
				aiFace& pFace = pMesh->mFaces[j];
				
				for (size_t k=0; k < pFace.mNumIndices; ++k)
					vIndices.push_back(static_cast<TIndex>( pFace.mIndices[k]) );
			}
		}	
	
	} // if (HAS_MESHES)

	m_meshHeader.m_nTotalIndices = m_nIndicesCnt = NUM_INDICES_TOTAL;
	m_meshHeader.m_nTotalVerts = m_nVerticesCnt = NUM_VERTICES_TOTAL;
	m_meshHeader.m_nMaterials = m_nMaterialCnt = NUM_MATERIALS;
	m_meshHeader.m_nSubmeshes = m_nSubmeshesCnt = NUM_SUBMESHES;
	strcpy_s(m_meshHeader.magic, 16, MESH_HEADER_MAGIC);

	// Find main Bounding Box
	Vec3 bbMin, bbMax;
	bbMin = m_submeshes[0].m_min;
	bbMax = m_submeshes[0].m_max;

	for (unsigned int i=1; i < m_submeshes.size(); ++i)
	{
		const Vec3& smBBMin = m_submeshes[i].m_min;
		const Vec3& smBBMax = m_submeshes[i].m_max;

		// min
		if (smBBMin.x < bbMin.x)
			bbMin.x = smBBMin.x;
		if (smBBMin.y < bbMin.y)
			bbMin.y = smBBMin.y;
		if (smBBMin.z < bbMin.z)
			bbMin.z = smBBMin.z;

		// max
		if (smBBMax.x > bbMax.x)
			bbMax.x = smBBMax.x;
		if (smBBMax.y > bbMax.y)
			bbMax.y = smBBMax.y;
		if (smBBMax.z > bbMax.z)
			bbMax.z = smBBMax.z;
	}

	memcpy(&m_meshHeader.m_min, &bbMin, sizeof(Vec3));
	memcpy(&m_meshHeader.m_max, &bbMax, sizeof(Vec3));

	// Create buffers & textures
	if (bCreateBuffers)
		CreateBuffers();

	if (bImportTextures)
		CreateTextures();

	char chTest[16];
	sprintf_s(chTest, "%d\n", m_nSubmeshesCnt);
	OutputDebugStringA(chTest);

	
	return true;
}

//----------------------------------------------------------------------
void CceMesh::Clear()
{
	m_materials.clear();
	m_indices.clear();
	m_verticesStream1.clear();
	m_verticesStream2.clear();
	m_submeshes.clear();
	m_texturesNew.clear();
	m_texturesMap.clear();


	m_nMaterialCnt = 0;
	m_nIndicesCnt = 0;
	m_nSubmeshesCnt = 0;
	m_nVerticesCnt = 0;

	// Release D3D11 buffers
	SAFE_RELEASE(m_pIndexBuffer);
	SAFE_RELEASE(m_pVertexBufferStream1);
	SAFE_RELEASE(m_pVertexBufferStream2);
}

//----------------------------------------------------------------------
void CceMesh::PrintStats() const
{
	// Stats printer
	// @TODO: Log.

	char buffer[128];
	sprintf_s(buffer, 127, "Total submeshes: %zd\n\n", m_submeshes.size());
	OutputDebugStringA(buffer);

	for (unsigned int i=0; i < m_submeshes.size(); ++i)
	{
		const CMeshChunk& sm = m_submeshes[i];

		sprintf_s(buffer, 127, "SM %d: IndexCnt=%d, StartIndexLoc=%d, BaseVertexLoc=%d\n", i, sm.m_numIndices, sm.m_startIndexLocation, sm.m_baseVertexLocation);
		OutputDebugStringA(buffer);
	}
}

//----------------------------------------------------------------------
void CceMesh::CreateBuffers()
{
	// Release Buffers
	SAFE_RELEASE(m_pVertexBufferStream1);
	SAFE_RELEASE(m_pVertexBufferStream2);
	SAFE_RELEASE(m_pIndexBuffer);

	D3D11_BUFFER_DESC bufDesc;
	bufDesc.CPUAccessFlags = 0;
	bufDesc.MiscFlags = 0;
	bufDesc.StructureByteStride = 0;
	bufDesc.Usage = D3D11_USAGE_IMMUTABLE;
	D3D11_SUBRESOURCE_DATA subrData;

	// Create vertex buffer (stream 1)
	bufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufDesc.ByteWidth = m_nVerticesCnt * sizeof(SceMeshVertexStream1);
	subrData.pSysMem = &m_verticesStream1[0];
	
	m_pDevice->CreateBuffer(&bufDesc, &subrData, &m_pVertexBufferStream1);

	// Create vertex buffer (stream 2)
	bufDesc.ByteWidth = m_nVerticesCnt * sizeof(SceMeshVertexStream2);
	subrData.pSysMem = &m_verticesStream2[0];

	m_pDevice->CreateBuffer(&bufDesc, &subrData, &m_pVertexBufferStream2);

	// Create index buffer
	bufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufDesc.ByteWidth = m_nIndicesCnt * sizeof(TIndex);
	subrData.pSysMem = &m_indices[0];

	m_pDevice->CreateBuffer(&bufDesc, &subrData, &m_pIndexBuffer);
}

//------------------------------------------------------------------------
void CceMesh::Initialize(ID3D11Device* pDevice)
{
	m_pDevice = pDevice;
}

//------------------------------------------------------------------------
ID3D11Buffer* CceMesh::GetVertexBufferStream1() const
{
	return m_pVertexBufferStream1;
}

//------------------------------------------------------------------------
ID3D11Buffer* CceMesh::GetIndexBuffer() const
{
	return m_pIndexBuffer;
}

//------------------------------------------------------------------------
unsigned int CceMesh::GetNumSubmeshes() const
{
	return m_nSubmeshesCnt;
}

//------------------------------------------------------------------------
unsigned int CceMesh::GetNumMaterials() const
{
	return m_nMaterialCnt;
}

//------------------------------------------------------------------------
const CMeshChunk& CceMesh::GetSubmesh(unsigned int idx)
{
	return m_submeshes[idx];
}

//------------------------------------------------------------------------
const SceMeshMaterial& CceMesh::GetMaterial(unsigned int idx)
{
	return m_materials[idx];
}

//----------------------------------------------------------------------
void CceMesh::CreateTextures()
{
	// At first, save current directory
	char chCurrDirectory[MAX_PATH] = {0};
	GetCurrentDirectoryA(MAX_PATH, chCurrDirectory);

	// and set new dir for textures.
	if (m_bUseRelativeTexturePaths)
		SetCurrentDirectoryA( m_strFileCurrDirectory.c_str() );



	// stwórz tutaj mapê
	const uint32 NUM_MATERIALS = m_nMaterialCnt;
	const uint32 MAX_TEXTURE_LAYERS = 9;

	uint32 iTexturesCounter = 0;

	// Fill 2d array which maps index of texture with material/textureslot pair
	m_IndexOfTextureFromMaterialAndTextureSlot.resize(NUM_MATERIALS);
	for (auto& i : m_IndexOfTextureFromMaterialAndTextureSlot)
	{
		i.resize(MAX_TEXTURE_LAYERS);
	}

	for ( uint32 iMat=0; iMat < NUM_MATERIALS; iMat++ )
	{
		auto& material = m_materials[iMat];
		for ( uint32 iTex=0; iTex < MAX_TEXTURE_LAYERS; iTex++ )
		{
			if ( material.m_textures[iTex].bIsValid )
			{
				const std::string strPath( material.m_textures[iTex].path );

				TTexturesMap::const_iterator it = m_texturesMap.find( strPath );
				if (it == m_texturesMap.end())
				{
					m_texturesMap.insert( TTexturesMap::value_type( strPath, iTexturesCounter ) );
					
					m_IndexOfTextureFromMaterialAndTextureSlot[iMat][iTex] = iTexturesCounter++;
				}
				else
				{
					m_IndexOfTextureFromMaterialAndTextureSlot[iMat][iTex] = it->second;
				}
			}
		}
	}

	// Load all textures
	m_texturesNew.clear();
	m_texturesNew.resize( m_texturesMap.size() );

	size_t iCounter = 0;
	for (const auto& it : m_texturesMap)
	{
		DebugPrintf( "Loading texture %d (%d) %s\n", iCounter++, it.second, it.first.c_str() );
		m_texturesNew[ it.second ].LoadTexture( m_pDevice, it.first.c_str() );
	}




	if (m_bUseRelativeTexturePaths)
		SetCurrentDirectoryA(chCurrDirectory);
}

//----------------------------------------------------------------------
const CTexture& CceMesh::GetTexture(unsigned int nMat, unsigned int nTex)
{
	if ( m_materials[nMat].m_textures[nTex].bIsValid )
	{
		const uint32 index = m_IndexOfTextureFromMaterialAndTextureSlot[nMat][nTex];
		return m_texturesNew[index];
	}
	else
	{
		static CTexture nullTexture;
		return nullTexture;
	}
}

//----------------------------------------------------------------------
bool CceMesh::ImportFile(const char* pszPath, bool bLeftHanded /*= true*/, bool bImportTextures, bool bCreateBuffers)
{
	Clear();
	// Is it cemesh?
	// Instead of comparising extension which can be changed, it's better to read it's first 16 bytes ("magic")

	char chMagic[16];
	bool bIsCemesh = false;

	FILE* pFile = nullptr;
	fopen_s(&pFile, pszPath, "rb");

	if (!pFile)
		return false;
	else
	{
		fread( (void*) chMagic, 1, 16, pFile);

		if (!strcmp(chMagic, MESH_HEADER_MAGIC))
			bIsCemesh = true;

		// Determine current directory of mesh
		{
			char chPath[MAX_PATH];

			// Check if the path is relative
			BOOL bIsRelative = !!PathIsRelativeA(pszPath);
			if (!!bIsRelative)
			{
				_fullpath(chPath, pszPath, sizeof(chPath));
				PathRemoveFileSpecA(chPath);
			}
			else
			{
				strcpy_s(chPath, 256, pszPath);
				PathRemoveFileSpecA(chPath);
				strcat_s(chPath, "\\");
			}	

			m_strFileCurrDirectory = std::string(chPath);
		}
	}

	fclose(pFile);

	if (bIsCemesh)
		return ImportMeshFromFile(pszPath, bImportTextures, bCreateBuffers);
	else
		return ImportRawData(pszPath, bLeftHanded, bImportTextures, bCreateBuffers);
}

ID3D11Buffer* CceMesh::GetVertexBufferStream2() const
{
	return m_pVertexBufferStream2;
}
