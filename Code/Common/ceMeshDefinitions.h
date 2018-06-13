#ifndef __cemesh_defs_h__
#define __cemesh_defs_h__


#include "mathlib.h"

#define MESH_HEADER_MAGIC		"CEMESH 0.1"

#define ceMESH_TEXTURE_AMBIENT			0
#define ceMESH_TEXTURE_DIFFUSE			1
#define ceMESH_TEXTURE_DISPLACEMENT		2
#define ceMESH_TEXTURE_EMISSIVE			3
#define ceMESH_TEXTURE_HEIGHT			4
#define ceMESH_TEXTURE_LIGHTMAP			5
#define ceMESH_TEXTURE_NORMAL			6
#define ceMESH_TEXTURE_OPACITY			7
#define ceMESH_TEXTURE_SPECULAR			8

struct SceMeshVertexStream1
{
	SceMeshVertexStream1()
	{
		memset(this, 0, sizeof(SceMeshVertexStream1));
	}

	Vec3	m_position;
	Vec2	m_texcoord;
};

struct SceMeshVertexStream2
{
	SceMeshVertexStream2()
	{
		memset(this, 0, sizeof(SceMeshVertexStream2));
	}

	Vec3	m_normal;
	Vec3	m_tangent;
	Vec3	m_bitangent;
};

struct SceMeshVertex
{
	SceMeshVertex()
	{
		// Zero all
		memset(this, 0, sizeof(SceMeshVertex));
	}

	Vec3	m_position;
	Vec3	m_normal;
	Vec3	m_tangent;
	Vec3	m_bitangent;
	Vec2	m_texcoords;
};

struct CMeshChunk
{
	CMeshChunk()
		: m_numVertices(0)
		, m_numIndices(0)
		, m_materialID(0)
	{}

	unsigned int	m_numVertices;	// SVertex
	unsigned int	m_numIndices;	// index count
	char			m_name[32];
	unsigned int	m_materialID;

	// Bounding Box
	Vec3			m_min;
	Vec3			m_max;

	// data for indexed drawing
	unsigned int	m_startIndexLocation;
	unsigned int	m_baseVertexLocation;
};


// Related to textures.
// There are similiar to these ones in assimp.
enum e_ceMeshTextureType
{
	eTEXTYPE_NONE = -1,

	eTEXTYPE_AMBIENT,
	eTEXTYPE_DIFFUSE,
	eTEXTYPE_DISPLACEMENT,
	eTEXTYPE_EMISSIVE,
	eTEXTYPE_HEIGHT,
	eTEXTYPE_LIGHTMAP,
	eTEXTYPE_NORMAL,
	eTEXTYPE_OPACITY,
	eTEXTYPE_SPECULAR
};



struct SceMeshTexture
{
	SceMeshTexture()
		: bIsValid(false)
		, m_texType(eTEXTYPE_NONE)
	{
		memset(path, 0, sizeof(path));
	}

	bool bIsValid;

	e_ceMeshTextureType	m_texType;
	char path[128];	
};

struct SceMeshMaterial
{
	char			m_name[32];
	unsigned int	m_id;

	Vec3			m_clrAmbient;
	Vec3			m_clrDiffuse;
	Vec3			m_clrSpecular;
	Vec3			m_clrEmissive;
	Vec3			m_clrTransparent;

	int				m_nWireframe;
	int				m_nTwosided;
	int				m_nShadingModel;

	float			m_fOpacity;
	float			m_fShininess;
	float			m_fShininessStrength;

	SceMeshTexture	m_textures[9];
};


struct SceMeshFileHeader
{
	char magic[16];

	unsigned int	m_nSubmeshes;
	unsigned int	m_nMaterials;

	unsigned int	m_nTotalVerts;
	unsigned int	m_nTotalIndices;

	// Bounding Box
	Vec3	m_min;
	Vec3	m_max;
};

#endif