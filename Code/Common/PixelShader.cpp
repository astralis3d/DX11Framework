#include "PCH.h"
#include "PixelShader.h"

CPixelShader::CPixelShader()
	: IShader(eShaderType::eShaderType_Pixel)
{

}

CPixelShader::CPixelShader(ID3D11Device* pDevice, const char* file, const char* function, const CD3D11ShaderMacro* pDefines /*= nullptr*/)
	: IShader(eShaderType::eShaderType_Pixel)
{
	Initialize(pDevice, file, function, pDefines);
}

CPixelShader::~CPixelShader()
{
	Release();
}

void CPixelShader::Release()
{
	IShader::Release();

	ID3D11PixelShader* pPS = (ID3D11PixelShader*)GetHandle();
	SAFE_RELEASE(pPS);
}
