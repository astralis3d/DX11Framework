#include "PCH.h"
#include "Sharpen.h"
#include "D3DFullscreenPass.h"


CSharpenD3D11::CSharpenD3D11()
{

}

CSharpenD3D11::~CSharpenD3D11()
{

}

//------------------------------------------------------------------------
void CSharpenD3D11::Initialize( ID3D11Device* pDevice, FullscreenPass* pFullscreenPass )
{
	m_pFullscreenPass = pFullscreenPass;

	m_sharpenCB.Create( pDevice );
	m_sharpenPS = CompilePSFromFile( pDevice, "Sharpen.hlsl", "SharpenPS" );
}

//------------------------------------------------------------------------
void CSharpenD3D11::Render( ID3D11DeviceContext* pDevContext, ID3D11ShaderResourceView* pInputs[], uint32 nInputsCount, ID3D11RenderTargetView* pOutput )
{
	PIXEvent evt(L"Sharpen");


	ID3D11RenderTargetView* pTarget[1] = { pOutput };
	pDevContext->OMSetRenderTargets(1, pTarget, nullptr);

	m_sharpenCB.UpdateBuffer( pDevContext );
	m_sharpenCB.SetPS( pDevContext, 5 );

	pDevContext->PSSetShaderResources( 0, nInputsCount, pInputs);
	m_sharpenPS->Shader()->Bind(pDevContext);

	m_pFullscreenPass->RenderFullscreenTriangle(pDevContext);
}

//------------------------------------------------------------------------
void CSharpenD3D11::Release()
{
	m_sharpenCB.Release();
}
