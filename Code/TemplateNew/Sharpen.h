#pragma once

class FullscreenPass;


class CSharpenD3D11
{
public:
	CSharpenD3D11();
	~CSharpenD3D11();

	void Initialize(ID3D11Device* pDevice, FullscreenPass* pFullscreenPass);
	void Render(ID3D11DeviceContext* pDevContext, ID3D11ShaderResourceView* pInputs[], uint32 nInputsCount, ID3D11RenderTargetView* pOutput);

	void Release();

private:
	struct SSharpenCB
	{
		SSharpenCB()
		{
			// 2.0, 1.80, 0.025, -0.25
			// -13.3333, 1.33333, 0.0, 1.0
			params1 = Vec4( 2.0, 1.76f, 0.025f, -0.25f );
			params2 = Vec4( -10.0, 0.15285, 0.0f, 1.0f );
		}

		Vec4	params1;
		Vec4	params2;
	};

	PixelShaderPtr					m_sharpenPS;
	D3DConstantBuffer<SSharpenCB>	m_sharpenCB;

	FullscreenPass*					m_pFullscreenPass;
};