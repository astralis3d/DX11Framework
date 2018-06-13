#ifndef __d3dshaders_h__
#define __d3dshaders_h__

#ifdef _MSC_VER
# pragma once
#endif

#include <memory>
#include "PixelShader.h"
#include "VertexShader.h"
#include "GeometryShader.h"
#include "HullShader.h"
#include "DomainShader.h"
#include "ComputeShader.h"

typedef std::unique_ptr<CPixelShader>		TPixelShader;
typedef std::unique_ptr<CVertexShader>		TVertexShader;
typedef std::unique_ptr<CGeometryShader>	TGeometryShader;
typedef std::unique_ptr<CHullShader>		THullShader;
typedef std::unique_ptr<CDomainShader>		TDomainShader;
typedef std::unique_ptr<CComputeShader>		TComputeShader;

typedef	CPixelShader						PixelShader;
typedef CVertexShader						VertexShader;
typedef CGeometryShader						GeometryShader;
typedef CHullShader							HullShader;
typedef CDomainShader						DomainShader;
typedef CComputeShader						ComputeShader;

// Prototype to simplify creating all this stuff
class CompiledShader
{
public:
	virtual ~CompiledShader() { }

	virtual eShaderType GetCompiledShaderType() const = 0;
};

template <typename T>
class CompiledShaderT : public CompiledShader
{
public:
	CompiledShaderT() { }

	void Init(T* p)
	{
		m_Shader.reset(p);
	}

	T* Shader() const
	{
		return m_Shader.get();
	}

	virtual eShaderType GetCompiledShaderType() const
	{
		if ( typeid(T) == typeid(PixelShader) )
		{
			return eShaderType::eShaderType_Pixel;
		}
		else if (typeid(T) == typeid(VertexShader) )
		{
			return eShaderType::eShaderType_Vertex;
		}
		else if ( typeid(T) == typeid(GeometryShader) )
		{
			return eShaderType::eShaderType_Geometry;
		}
		else if ( typeid(T) == typeid(HullShader) )
		{
			return eShaderType::eShaderType_Hull;
		}
		else if ( typeid(T) == typeid(DomainShader) )
		{
			return eShaderType::eShaderType_Domain;
		}
		else if ( typeid(T) == typeid(ComputeShader) )
		{
			return eShaderType::eShaderType_Compute;
		}
		else
		{
			return eShaderType::eShaderType_Unknown;
		}
	}


private:
	std::unique_ptr<T>		m_Shader;
};

typedef CompiledShaderT<PixelShader>*		PixelShaderPtr;
typedef CompiledShaderT<VertexShader>*		VertexShaderPtr;
typedef CompiledShaderT<GeometryShader>*	GeometryShaderPtr;
typedef CompiledShaderT<HullShader>*		HullShaderPtr;
typedef CompiledShaderT<DomainShader>*		DomainShaderPtr;
typedef CompiledShaderT<ComputeShader>*		ComputeShaderPtr;

PixelShaderPtr		CompilePSFromFile( ID3D11Device* pDevice, const char* file, const char* function, const CD3D11ShaderMacro* pDefines = nullptr );
VertexShaderPtr		CompileVSFromFile( ID3D11Device* pDevice, const char* file, const char* function, const CD3D11ShaderMacro* pDefines = nullptr );
GeometryShaderPtr CompileGSFromFile( ID3D11Device* pDevice, const char* file, const char* function, const CD3D11ShaderMacro* pDefines = nullptr );
HullShaderPtr CompileHSFromFile( ID3D11Device* pDevice, const char* file, const char* function, const CD3D11ShaderMacro* pDefines = nullptr );
DomainShaderPtr CompileDSFromFile( ID3D11Device* pDevice, const char* file, const char* function, const CD3D11ShaderMacro* pDefines = nullptr );
ComputeShaderPtr CompileCSFromFile( ID3D11Device* pDevice, const char* file, const char* function, const CD3D11ShaderMacro* pDefines = nullptr );

void ShutdownShaders();



#endif