#include "PCH.h"
#include "DomainShader.h"

CDomainShader::CDomainShader()
	: IShader(eShaderType::eShaderType_Domain)
{

}

CDomainShader::CDomainShader(ID3D11Device* pDevice, const char* file, const char* function, const CD3D11ShaderMacro* pDefines /*= nullptr*/)
	: IShader(eShaderType::eShaderType_Domain)
{
	Initialize(pDevice, file, function, pDefines);
}

CDomainShader::~CDomainShader()
{
	Release();
}

void CDomainShader::Release()
{
	IShader::Release();

	ID3D11DomainShader* pShader = (ID3D11DomainShader*)GetHandle();
	SAFE_RELEASE(pShader);
}
