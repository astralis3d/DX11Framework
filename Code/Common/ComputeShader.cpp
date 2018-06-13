#include "PCH.h"
#include "ComputeShader.h"

CComputeShader::CComputeShader()
	: IShader(eShaderType::eShaderType_Compute)
{

}

CComputeShader::CComputeShader(ID3D11Device* pDevice, const char* file, const char* function, const CD3D11ShaderMacro* pDefines /*= nullptr*/)
	: IShader(eShaderType::eShaderType_Compute)
{
	Initialize(pDevice, file, function, pDefines);
}

CComputeShader::~CComputeShader()
{
	Release();
}

void CComputeShader::Release()
{
	IShader::Release();

	ID3D11ComputeShader* pShader = (ID3D11ComputeShader*)GetHandle();
	SAFE_RELEASE(pShader);
}
