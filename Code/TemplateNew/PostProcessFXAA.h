#ifndef PostProcessFXAA_h__
#define PostProcessFXAA_h__

#pragma once

#include "PostProcessorBase.h"
#include "D3DShaders.h"
#include "D3DTypes.h"

class PostProcessFXAA : public PostProcessorBase
{
public:
	PostProcessFXAA();
	~PostProcessFXAA();

	virtual void Initialize(ID3D11Device* pDevice);
	virtual void Render(ID3D11DeviceContext* pDevContext, ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output);
	virtual void AfterReset(UINT width, UINT height);

	virtual void Release() override;

private:
	TPixelShader		m_FXAAPixelShader;
};

#endif // PostProcessFXAA_h__
