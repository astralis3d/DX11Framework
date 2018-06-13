#include "PCH.h"
#include "VertexShader.h"

CVertexShader::CVertexShader()
	: IShader(eShaderType::eShaderType_Vertex)
{

}

CVertexShader::CVertexShader(ID3D11Device* pDevice, const char* file, const char* function, const CD3D11ShaderMacro* pDefines /*= nullptr*/)
	: IShader(eShaderType::eShaderType_Vertex)
{
	Initialize(pDevice, file, function, pDefines);
}

CVertexShader::~CVertexShader()
{
	Release();
}

void CVertexShader::Release()
{
	IShader::Release();

	ID3D11VertexShader* pVS = (ID3D11VertexShader*)GetHandle();
	SAFE_RELEASE(pVS);
}

