#include "PCH.h"
#include "D3DShaders.h"
#include <vector>

std::vector<CompiledShader*>	compiledShaders;

PixelShaderPtr CompilePSFromFile( ID3D11Device* pDevice, const char* file, const char* function, const CD3D11ShaderMacro* pDefines )
{
    CompiledShaderT<CPixelShader>* pCompiledShader = new CompiledShaderT<CPixelShader>();
    CPixelShader* pBlob = new CPixelShader( pDevice, file, function, pDefines );

    pCompiledShader->Init(pBlob);
    compiledShaders.push_back(pCompiledShader);

    return pCompiledShader;
}

//------------------------------------------------------------------------
VertexShaderPtr CompileVSFromFile( ID3D11Device* pDevice, const char* file, const char* function, const CD3D11ShaderMacro* pDefines /* = nullptr */ )
{
    auto pCompiledShader = new CompiledShaderT<CVertexShader>();
    auto pBlob = new CVertexShader( pDevice, file, function, pDefines );

    pCompiledShader->Init( pBlob );
    compiledShaders.push_back( pCompiledShader );

    return pCompiledShader;
}

//------------------------------------------------------------------------
GeometryShaderPtr CompileGSFromFile( ID3D11Device* pDevice, const char* file, const char* function, const CD3D11ShaderMacro* pDefines /* = nullptr */ )
{
    auto pCompiledShader = new CompiledShaderT<CGeometryShader>();
    auto pBlob = new CGeometryShader( pDevice, file, function, pDefines );

    pCompiledShader->Init( pBlob );
    compiledShaders.push_back( pCompiledShader );

    return pCompiledShader;
}

//------------------------------------------------------------------------
HullShaderPtr CompileHSFromFile( ID3D11Device* pDevice, const char* file, const char* function, const CD3D11ShaderMacro* pDefines /* = nullptr */ )
{
    auto pCompiledShader = new CompiledShaderT<CHullShader>();
    auto pBlob = new CHullShader( pDevice, file, function, pDefines );

    pCompiledShader->Init( pBlob );
    compiledShaders.push_back( pCompiledShader );

    return pCompiledShader;
}

//------------------------------------------------------------------------
DomainShaderPtr CompileDSFromFile( ID3D11Device* pDevice, const char* file, const char* function, const CD3D11ShaderMacro* pDefines /* = nullptr */ )
{
    auto pCompiledShader = new CompiledShaderT<CDomainShader>();
    auto pBlob = new CDomainShader( pDevice, file, function, pDefines );

    pCompiledShader->Init( pBlob );
    compiledShaders.push_back( pCompiledShader );

    return pCompiledShader;
}

//------------------------------------------------------------------------
ComputeShaderPtr CompileCSFromFile( ID3D11Device* pDevice, const char* file, const char* function, const CD3D11ShaderMacro* pDefines /* = nullptr */ )
{
    auto pCompiledShader = new CompiledShaderT<CComputeShader>();
    auto pBlob = new CComputeShader( pDevice, file, function, pDefines );

    pCompiledShader->Init( pBlob );
    compiledShaders.push_back( pCompiledShader );

    return pCompiledShader;
}

struct SShaderInfo
{
	uint32 shaderSize;
	uint32 shaderOffset;
};

//------------------------------------------------------------------------
void ShutdownShaders()
{
	// Prototype shaders cooking system

	FILE* pFile = nullptr;
	fopen_s(&pFile, "Shaders.bin", "wb");

	for (size_t i = 0; i < compiledShaders.size(); ++i)
	{
		auto pCompiledShader = compiledShaders[i];

		eShaderType shaderType = pCompiledShader->GetCompiledShaderType();
		switch (shaderType)
		{
			case eShaderType::eShaderType_Pixel:
			{
				auto pCompiledShaderX = static_cast<PixelShaderPtr>(pCompiledShader);
				
				if (pCompiledShaderX->Shader()->GetBlob())
				{
					void* pData = pCompiledShaderX->Shader()->GetBlob()->GetBufferPointer();
					size_t dataSize = pCompiledShaderX->Shader()->GetBlob()->GetBufferSize();

					fwrite( pData, dataSize, 1, pFile );
				}
			}
			break;

			case eShaderType::eShaderType_Vertex:
			{
				auto pCompiledShaderX = static_cast<VertexShaderPtr>(pCompiledShader);

				if (pCompiledShaderX->Shader()->GetBlob())
				{
					void* pData = pCompiledShaderX->Shader()->GetBlob()->GetBufferPointer();
					size_t dataSize = pCompiledShaderX->Shader()->GetBlob()->GetBufferSize();

					fwrite( pData, dataSize, 1, pFile );
				}
			}
			break;


			case eShaderType::eShaderType_Geometry:
			{
				auto pCompiledShaderX = static_cast<GeometryShaderPtr>(pCompiledShader);

				void* pData = pCompiledShaderX->Shader()->GetBlob()->GetBufferPointer();
				size_t dataSize = pCompiledShaderX->Shader()->GetBlob()->GetBufferSize();

				fwrite( pData, dataSize, 1, pFile );
			}
			break;

			case eShaderType::eShaderType_Hull:
			{
				auto pCompiledShaderX = static_cast<HullShaderPtr>(pCompiledShader);

				void* pData = pCompiledShaderX->Shader()->GetBlob()->GetBufferPointer();
				size_t dataSize = pCompiledShaderX->Shader()->GetBlob()->GetBufferSize();

				fwrite( pData, dataSize, 1, pFile );
			}
			break;

			case eShaderType::eShaderType_Domain:
			{
				auto pCompiledShaderX = static_cast<DomainShaderPtr>(pCompiledShader);

				void* pData = pCompiledShaderX->Shader()->GetBlob()->GetBufferPointer();
				size_t dataSize = pCompiledShaderX->Shader()->GetBlob()->GetBufferSize();

				fwrite( pData, dataSize, 1, pFile );
			}
			break;

			case eShaderType::eShaderType_Compute:
			{
				auto pCompiledShaderX = static_cast<ComputeShaderPtr>(pCompiledShader);

				void* pData = pCompiledShaderX->Shader()->GetBlob()->GetBufferPointer();
				size_t dataSize = pCompiledShaderX->Shader()->GetBlob()->GetBufferSize();

				fwrite( pData, dataSize, 1, pFile );
			}
			break;
		}



	}

	fclose(pFile);


    for ( size_t i=0; i < compiledShaders.size(); ++i )
	{
        delete compiledShaders[i];
	}

    compiledShaders.clear();
}
