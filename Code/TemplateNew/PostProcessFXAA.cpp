#include "PCH.h"
#include "PostProcessFXAA.h"

PostProcessFXAA::PostProcessFXAA()
	: PostProcessorBase()
{

}

PostProcessFXAA::~PostProcessFXAA()
{

}

//------------------------------------------------------------------------
void PostProcessFXAA::Initialize(ID3D11Device* _device)
{
	PostProcessorBase::Initialize(_device);

	m_FXAAPixelShader.reset(new CPixelShader(_device, "FXAA.hlsl", "FxaaPS"));
}

//------------------------------------------------------------------------
void PostProcessFXAA::Render(ID3D11DeviceContext* _devcon, ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output)
{
	PostProcessorBase::Render(_devcon, input, output);

	PostProcess(input, output, (ID3D11PixelShader*)m_FXAAPixelShader->GetHandle(), L"FXAA");
}

//------------------------------------------------------------------------
void PostProcessFXAA::AfterReset(UINT width, UINT height)
{
	PostProcessorBase::AfterReset(width, height);
}

//------------------------------------------------------------------------
void PostProcessFXAA::Release()
{
	PostProcessorBase::Release();

	m_FXAAPixelShader.reset();
}

