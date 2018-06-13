#include "PCH.h"
#include "GeometryShader.h"

CGeometryShader::CGeometryShader()
	: IShader(eShaderType::eShaderType_Geometry)
{

}

CGeometryShader::CGeometryShader(ID3D11Device* pDevice, const char* file, const char* function, const CD3D11ShaderMacro* pDefines /*= nullptr*/)
{

}


CGeometryShader::~CGeometryShader()
{
	Release();
}

void CGeometryShader::Release()
{
	IShader::Release();

	ID3D11GeometryShader* pShader = (ID3D11GeometryShader*)GetHandle();
	SAFE_RELEASE(pShader);
}