#ifndef __ceMesh_h__
#define __ceMesh_h__

#pragma once

#include "D3DTexture.h"
#include "ceMeshDefinitions.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Buffer;
struct ID3D11ShaderResourceView;
class CViewFrustum;

//----------------------------------------------------------------------
// citizengine Mesh File Format
// (c) Mateusz Nagórka 2014
//----------------------------------------------------------------------

/* This is the mesh format for citizengine. It is still in development.
 * Here is short documentation.
 * Work in Progress.
 *
 *
 * 1. Assumptions
 * 2. Pipeline
 * 3. Data Layout
 * 4. Notes
 *
 *
 * 1. Assumptions
 * - In general for Direct3D, but you can change it (left handed / right handed)
 * - Indexed meshes of course
 * - Currently static meshes only
 * - After load, one Vertex & Index Buffer - reduce IASetVertexBuffers / IASetIndexBuffers calls
 * - Support for Frustum Culling & Instancing
 *
 * 2. Pipeline
 * - For assets importing we use assimp 3.1.1 with full range of postprocessing options
 * - You can define a few options however, e.g. left handed mode or generating bitangents/tangents when you import it.
 * - Important: If there is a new version of assimp, we will integrate it quickly to our pipeline.
 *
 * 3. Data Layout
 * - In CEMESH file data are layed out as follows:
 * -	Header - 1*sizeof(SMeshFileHeader) >contains m, s, v, i <
 * -	Materials - m*sizeof(SMeshMaterial)
 * -	Submeshes - s*sizeof(SMeshSubmesh)
 * -	Vertices - v*sizeof(SMehsVertex)
 * -	Indices - i*sizeof(TIndex)
 */

class CceMesh
{
	typedef unsigned int MaterialID;

	typedef unsigned int							TIndex;

	typedef std::vector<SceMeshMaterial>			TMaterials;
	typedef std::vector<CMeshChunk>					TSubmeshes;
	typedef std::vector<SceMeshVertexStream1>		TVerticesStream1;
	typedef std::vector<SceMeshVertexStream2>		TVerticesStream2;
	typedef std::vector<TIndex>						TIndices;

	typedef std::vector<CTexture>					TTextures;
	typedef std::vector<TTextures>					T2DArrTextures;

public:
	CceMesh();
	CceMesh(ID3D11Device* pDevice);
	~CceMesh();

	// Clear all what is inside current mesh
	void Clear();

	void Initialize(ID3D11Device* pDevice);
	
	// Convenient function for importing file.
	// Automatically determines whether it is cemesh or other file
	bool ImportFile(const char* pszPath, bool bLeftHanded = true, bool bImportTextures = true, bool bCreateBuffers = true);

	// If mesh is already loaded, this function saves it to *.cemesh file
	bool ExportMeshToFile(const char* pszPath);

	
	// Print/Log stats
	void PrintStats() const;

	// Get particular submesh
	const CMeshChunk&	GetSubmesh(unsigned int idx);

	// Get particular material
	const SceMeshMaterial&	GetMaterial(unsigned int idx);

	// Get texture
	const CTexture&		GetTexture(unsigned int nMat, unsigned int nTex);

	// Get numbers
	unsigned int		GetNumSubmeshes()	const;
	unsigned int		GetNumMaterials()	const;
	unsigned int		GetNumInstances()	const;
	unsigned int		GetNumInstancesVisible() const;

	ID3D11Buffer*		GetVertexBufferStream1()	const;
	ID3D11Buffer*		GetVertexBufferStream2()	const;
	ID3D11Buffer*		GetIndexBuffer()			const;
	ID3D11Buffer*		GetInstancedBuffer()		const;

	void GetBoundingBox(Vec3& vMin, Vec3& vMax)
	{
		vMin = m_meshHeader.m_min;
		vMax = m_meshHeader.m_max;
	}

private:
	// Load raw data from e.g. *.obj file
	bool ImportRawData(const char* pszPath, bool LeftHanded, bool bImportTextures, bool bCreateBuffers);

	// Load mesh from existing *.cemesh file
	bool ImportMeshFromFile(const char* pszPath, bool bImportTextures, bool bCreateBuffers);

	// Create index and vertex buffers
	void CreateInstanceBuffer();
	void CreateBuffers();
	void CreateTextures();

	void UpdateInstancesData();

	//static CTexture s_texNull;

private:
	SceMeshFileHeader		m_meshHeader;

	typedef std::map<std::string, uint32> TTexturesMap;
	TTexturesMap	m_texturesMap;

	// List of textures
	TTextures				m_texturesNew;

	std::vector<std::vector<uint32>> m_IndexOfTextureFromMaterialAndTextureSlot;


	// Index and Vertex buffers.
	ID3D11Buffer*			m_pIndexBuffer;
	ID3D11Buffer*			m_pVertexBufferStream1;
	ID3D11Buffer*			m_pVertexBufferStream2;

	// Resources
	unsigned int			m_nMaterialCnt;
	TMaterials				m_materials;

	unsigned int			m_nIndicesCnt;
	TIndices				m_indices;

	unsigned int			m_nVerticesCnt;
	TVerticesStream1		m_verticesStream1;
	TVerticesStream2		m_verticesStream2;

	unsigned int			m_nSubmeshesCnt;
	TSubmeshes				m_submeshes;

	bool					m_bUseRelativeTexturePaths;
	std::string				m_strFileCurrDirectory;

	ID3D11Device*			m_pDevice;
};


#endif