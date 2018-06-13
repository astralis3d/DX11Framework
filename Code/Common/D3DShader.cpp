#include "PCH.h"
#include "D3DShader.h"
#include <d3dcompiler.h>
#include "ShaderHotReload.h"

// Global (for now) 

IShader::IShader()
	: m_pHandle(nullptr)
	, m_shaderType(eShaderType::eShaderType_Unknown)
	, m_pBlob(nullptr)
	, m_timestamp(0)
{

}

IShader::IShader(const eShaderType& type)
	: m_pHandle(nullptr)
	, m_shaderType(type)
	, m_pBlob(nullptr)
	, m_timestamp(0)
{

}

IShader::~IShader()
{
	Release();

	m_entrypoint.clear();
	m_filename.clear();

	m_defines.clear();
}

//------------------------------------------------------------------------
void IShader::Release()
{
	SAFE_RELEASE(m_pBlob);
}

//------------------------------------------------------------------------
void IShader::Bind(ID3D11DeviceContext* pDevContext)
{
	if (!m_pHandle)
		return;

	switch (m_shaderType)
	{
		case eShaderType::eShaderType_Compute:
			pDevContext->CSSetShader((ID3D11ComputeShader*)m_pHandle, nullptr, 0);	break;

		case eShaderType::eShaderType_Domain:
			pDevContext->DSSetShader((ID3D11DomainShader*)m_pHandle, nullptr, 0);	break;

		case eShaderType::eShaderType_Geometry:
			pDevContext->GSSetShader((ID3D11GeometryShader*)m_pHandle, nullptr, 0);	break;

		case eShaderType::eShaderType_Pixel:
			pDevContext->PSSetShader((ID3D11PixelShader*)m_pHandle, nullptr, 0);	break;

		case eShaderType::eShaderType_Hull:
			pDevContext->HSSetShader((ID3D11HullShader*)m_pHandle, nullptr, 0);		break;

		case eShaderType::eShaderType_Vertex:
			pDevContext->VSSetShader((ID3D11VertexShader*)m_pHandle, nullptr, 0);	break;
	}
}

//------------------------------------------------------------------------
eShaderType IShader::GetShaderType() const
{
	return m_shaderType;
}

//------------------------------------------------------------------------
void* IShader::GetHandle() const
{
	return m_pHandle;
}

//------------------------------------------------------------------------
void IShader::Initialize(ID3D11Device* pDevice, LPCSTR pFilename, const char* pFunction, const CD3D11ShaderMacro* pDefines /*= nullptr*/, bool bHotReload /*=false*/)
{
	// Determine target profile for shader
	std::string shaderTarget;
	switch (m_shaderType)
	{
		case eShaderType::eShaderType_Compute:	shaderTarget = std::string( "cs_5_0" ); break;
		case eShaderType::eShaderType_Domain:	shaderTarget = std::string( "ds_5_0" ); break;
		case eShaderType::eShaderType_Geometry:	shaderTarget = std::string( "gs_5_0" ); break;
		case eShaderType::eShaderType_Hull:		shaderTarget = std::string( "hs_5_0" ); break;
		case eShaderType::eShaderType_Pixel:	shaderTarget = std::string( "ps_5_0" ); break;
		case eShaderType::eShaderType_Vertex:	shaderTarget = std::string( "vs_5_0" ); break;
	}
	

	// Initialize hot-reload system.
	if (bHotReload == false)
	{
		// For shader defintions, get them from pDefines (CD3D11ShaderMacro)
		// and write them to our shader object.
		if (pDefines != nullptr)
		{
			unsigned int nDefines = 0;
			while (pDefines->GetShaderMacro(nDefines).Definition != nullptr)
				nDefines++;

			if (nDefines > 0)
			{
				for (unsigned int i = 0; i < nDefines; ++i)
				{
					m_defines.emplace_back( std::make_pair( pDefines->GetShaderMacro(i).Name, pDefines->GetShaderMacro(i).Definition) );
				}
			}
		}

		m_entrypoint = std::string(pFunction);
		m_filename = std::string(pFilename);

		CShaderHotReload::Get()->AddShader(this);
	}


	// Last modified date
	HANDLE hFile = CreateFileA(pFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		FILETIME lastWriteTime;
		::GetFileTime(hFile, nullptr, nullptr, &lastWriteTime);
		::CloseHandle(hFile);

		SYSTEMTIME fileTime;
		::FileTimeToSystemTime(&lastWriteTime, &fileTime);

		m_timestamp = CalculateShaderTimestamp(fileTime.wSecond, fileTime.wMilliseconds);	
	}
	

	// Shader compilation loop.
	// 
	// Our goal is to get shader blob.
	// It can be obtained either from *.hlsl source file, or from cooked package.

	bool bShaderSuccessfullyCompiled = true;
	HRESULT result;
	ID3DBlob* pBlobMessages = nullptr;

	UINT CompileFlags = 0;
#if defined (_DEBUG) | defined (DEBUG)
	CompileFlags = (D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_PREFER_FLOW_CONTROL);
#else
	CompileFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

	do
	{
		// if ( useShaderCooker )
		// { get blob from cooked file }


		result = D3DCompileFromFile(AnsiToWString(pFilename).c_str(), 
									(pDefines) ? pDefines->GetShaderMacrosPtr() : nullptr,
									D3D_COMPILE_STANDARD_FILE_INCLUDE,
									pFunction,
									shaderTarget.c_str(),
									CompileFlags,
									0,
									&m_pBlob,
									&pBlobMessages);
		
		// Check if file does NOT exist
		// TODO: Which file EXACTLY?
		if (result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) || result == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND))
		{
			char buffer[512];

			sprintf_s(buffer, 511, "File/Path '%s' does not exist", pFilename);

			//MessageBox( 0, L"File does not exist", L"Error", MB_ICONWARNING | MB_OK );
			MessageBoxA(HWND_DESKTOP, buffer, "Error", MB_OK | MB_ICONWARNING);
		}
		

		// Show shader compiler *warnings* in Output Console (non critical)
		if ( SUCCEEDED(result) && pBlobMessages != nullptr )
		{
			DebugPrintf("%s\n", pBlobMessages->GetBufferPointer() );
		}


		// Compilation loop. Developer can fix faults without exiting program
		// or cancel - shader will be set to NULL	
		if ( FAILED(result) )
		{			
			std::string strErr = std::string("Shader compiler errors occured:\n");

			if (pBlobMessages)
			{
				const char* err = (const char*) pBlobMessages->GetBufferPointer();
				strErr += std::string(err);
			}

			strErr += std::string("\n\nRetry compilation?");

			int retCode = MessageBoxA(0, strErr.c_str(), "D3DCompiler messages", MB_RETRYCANCEL | MB_ICONQUESTION);

			if (retCode == IDRETRY)
				continue;
			else
			{
				bShaderSuccessfullyCompiled = false;
				break;
			}
		}		
	}
	while( FAILED(result) );

	SAFE_RELEASE(pBlobMessages);


	// Developer has decided not to fix errors in shader, so set this shader
	// to NULL instead of crashing whole program
	if (!bShaderSuccessfullyCompiled)
	{
		m_pBlob = nullptr;
		m_pHandle = nullptr;

		return;
	}
	else
	{
		void* pShaderBytecode = m_pBlob->GetBufferPointer();
		size_t bytecodeLength = m_pBlob->GetBufferSize();

		// Perform shader stripping in non-debug builds.
	#ifndef _DEBUG
		ID3DBlob* pStrippedBlob = nullptr;

		const UINT stripFlags = D3DCOMPILER_STRIP_DEBUG_INFO | D3DCOMPILER_STRIP_REFLECTION_DATA | D3DCOMPILER_STRIP_TEST_BLOBS;
		result = D3DStripShader(pShaderBytecode, bytecodeLength, stripFlags, &pStrippedBlob);
		if (FAILED(result))
		{
			DebugPrintf("D3DStripShader failed. Result: %08x\n", result);
		}

		// Use stripped blob.
		if (pStrippedBlob)
		{
			pShaderBytecode = pStrippedBlob->GetBufferPointer();
			bytecodeLength = pStrippedBlob->GetBufferSize();

			//DebugPrintf("D3DStripShader: before: %d, after: %d\n", m_pBlob->GetBufferSize(), pStrippedBlob->GetBufferSize() );
		}
	#endif

		switch (m_shaderType)
		{
			case eShaderType::eShaderType_Compute:
				pDevice->CreateComputeShader(pShaderBytecode, bytecodeLength, nullptr, (ID3D11ComputeShader**)&m_pHandle);	break;

			case eShaderType::eShaderType_Domain:
				pDevice->CreateDomainShader(pShaderBytecode, bytecodeLength, nullptr, (ID3D11DomainShader**)&m_pHandle);	break;

			case eShaderType::eShaderType_Geometry:
				pDevice->CreateGeometryShader(pShaderBytecode, bytecodeLength, nullptr, (ID3D11GeometryShader**)&m_pHandle);	break;

			case eShaderType::eShaderType_Hull:
				pDevice->CreateHullShader(pShaderBytecode, bytecodeLength, nullptr, (ID3D11HullShader**)&m_pHandle);	break;

			case eShaderType::eShaderType_Pixel:
				pDevice->CreatePixelShader(pShaderBytecode, bytecodeLength, nullptr, (ID3D11PixelShader**)&m_pHandle);	break;

			case eShaderType::eShaderType_Vertex:
				pDevice->CreateVertexShader(pShaderBytecode, bytecodeLength, nullptr, (ID3D11VertexShader**)&m_pHandle);	break;
		}
	}
}

//------------------------------------------------------------------------
ID3D10Blob* IShader::GetBlob() const
{
	return m_pBlob;
}



//------------------------------------------------------------------------
void CD3D11ShaderMacro::AddMacro(const char* name, const char* definition)
{
	D3D_SHADER_MACRO macro;
	macro.Name = name;
	macro.Definition = definition;

	m_ShaderMacro.push_back(macro);
}

//------------------------------------------------------------------------
void CD3D11ShaderMacro::Finish()
{
	D3D_SHADER_MACRO macro;
	memset(&macro, 0, sizeof(D3D_SHADER_MACRO));

	m_ShaderMacro.push_back(macro);
}

//------------------------------------------------------------------------
void CD3D11ShaderMacro::Reset()
{
	m_ShaderMacro.clear();
}

//------------------------------------------------------------------------
const D3D_SHADER_MACRO* CD3D11ShaderMacro::GetShaderMacrosPtr() const
{
	return &m_ShaderMacro[0];
}

//------------------------------------------------------------------------
const D3D_SHADER_MACRO& CD3D11ShaderMacro::GetShaderMacro(int i) const
{
	return m_ShaderMacro[i];
}
