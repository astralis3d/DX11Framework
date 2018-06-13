#include "PCH.h"
#include "HullShader.h"

CHullShader::CHullShader()
	: IShader(eShaderType::eShaderType_Hull)
{

}

CHullShader::CHullShader(ID3D11Device* pDevice, const char* file, const char* function, const CD3D11ShaderMacro* pDefines /*= nullptr*/)
	: IShader(eShaderType::eShaderType_Hull)
{
	Initialize(pDevice, file, function, pDefines);
}

CHullShader::~CHullShader()
{
	Release();
}

void CHullShader::Release()
{
	IShader::Release();

	ID3D11HullShader* pShader = (ID3D11HullShader*)GetHandle();
	SAFE_RELEASE(pShader);
}
