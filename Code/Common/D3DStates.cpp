//--------------------------------------------------------------------------------------
// This code contains NVIDIA Confidential Information and is disclosed 
// under the Mutual Non-Disclosure Agreement. 
// 
// Notice 
// ALL NVIDIA DESIGN SPECIFICATIONS AND CODE ("MATERIALS") ARE PROVIDED "AS IS" NVIDIA MAKES 
// NO REPRESENTATIONS, WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO 
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ANY IMPLIED WARRANTIES OF NONINFRINGEMENT, 
// MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. 
// 
// NVIDIA Corporation assumes no responsibility for the consequences of use of such 
// information or for any infringement of patents or other rights of third parties that may 
// result from its use. No license is granted by implication or otherwise under any patent 
// or patent rights of NVIDIA Corporation. No third party distribution is allowed unless 
// expressly authorized by NVIDIA.  Details are subject to change without notice. 
// This code supersedes and replaces all information previously supplied. 
// NVIDIA Corporation products are not authorized for use as critical 
// components in life support devices or systems without express written approval of 
// NVIDIA Corporation. 
// 
// Copyright © 2012, NVIDIA Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "PCH.h"
#include "D3DStates.h"

static States* s_pStates = NULL;

States::States()
{
	pBackfaceCull_RS = NULL;
	pBackfaceCull_WireFrame_RS = NULL;
	pBackfaceCullMSAA_RS = NULL;
	pBackfaceCull_WireFrameMSAA_RS = NULL;
	pNoCull_RS = NULL;


	pDepthNoStencil_DS = NULL;
	pNoDepthNoStencil_DS = NULL;
	pStencilSetBits_DS = NULL;
	pNoDepthStencilComplex_DS = NULL;
	pDepthStencilComplex_DS = NULL;
	pDS_Skybox = NULL;
	pDS_DepthGreater = nullptr;

	pNoBlend_BS = NULL;
	
	pSamplerPointWrap = nullptr;
	pSamplerPointClamp = nullptr;
	pSamplerLinearWrap = nullptr;
	pSamplerLinearClamp = nullptr;
	pSamplerAnisoWrap = nullptr;
	pSamplerAnisoClamp = nullptr;
	pSamplerComparisonLinear = nullptr;
}

States::~States()
{
	ReleaseResources();
}

void States::Claim()
{
	if (NULL == s_pStates)
	{
		s_pStates = new States;
	}
}

void States::Destroy()
{
	SAFE_DELETE(s_pStates);
}

States* States::Get()
{
	return s_pStates;
}

void States::CreateResources(ID3D11Device *pd3dDevice)
{
	CreateSamplers(pd3dDevice);
	CreateRasterizerStates(pd3dDevice);
	CreateDepthStencilStates(pd3dDevice);
	CreateBlendStates(pd3dDevice);
}

void States::ReleaseResources()
{
	SAFE_RELEASE(pBackfaceCull_RS);
	SAFE_RELEASE(pBackfaceCull_WireFrame_RS);
	SAFE_RELEASE(pBackfaceCullMSAA_RS);
	SAFE_RELEASE(pBackfaceCull_WireFrameMSAA_RS);
	SAFE_RELEASE(pNoCull_RS);

	SAFE_RELEASE(pDepthNoStencil_DS);
	SAFE_RELEASE(pNoDepthNoStencil_DS);
	SAFE_RELEASE(pStencilSetBits_DS);
	SAFE_RELEASE(pNoDepthStencilComplex_DS);
	SAFE_RELEASE(pDepthStencilComplex_DS);
	SAFE_RELEASE(pDS_Skybox);
	SAFE_RELEASE(pDS_DepthGreater);

	SAFE_RELEASE(pNoBlend_BS);
	SAFE_RELEASE(pBlend_BS);
	SAFE_RELEASE(pBlendAlphaToCoverage_BS);
	SAFE_RELEASE(pBlendAdditive_BS);

	SAFE_RELEASE(pSamplerPointWrap);
	SAFE_RELEASE(pSamplerPointClamp);
	SAFE_RELEASE(pSamplerLinearWrap);
	SAFE_RELEASE(pSamplerLinearClamp);
	SAFE_RELEASE(pSamplerAnisoWrap);
	SAFE_RELEASE(pSamplerAnisoClamp);
	SAFE_RELEASE(pSamplerComparisonLinear);
}

void States::CreateSamplers(ID3D11Device *pd3dDevice)
{
	HRESULT hr;

	// Point Clamp
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	memset(&samplerDesc.BorderColor, 0, sizeof(samplerDesc.BorderColor));
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MaxLOD = FLT_MAX;
	V(pd3dDevice->CreateSamplerState(&samplerDesc, &pSamplerPointClamp));

	// Point Wrap
	samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	V(pd3dDevice->CreateSamplerState(&samplerDesc, &pSamplerPointWrap));

	// Linear Clamp
	samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	V(pd3dDevice->CreateSamplerState(&samplerDesc, &pSamplerLinearClamp));

	// Linear Wrap
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	V(pd3dDevice->CreateSamplerState(&samplerDesc, &pSamplerLinearWrap));


	// Aniso Wrap
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 8;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MipLODBias = -1.0f;
	V(pd3dDevice->CreateSamplerState(&samplerDesc, &pSamplerAnisoWrap));

	// Aniso Clamp
	samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	V(pd3dDevice->CreateSamplerState(&samplerDesc, &pSamplerAnisoClamp));


	// Linear Comparison
	// Sampler States
	//samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;	// by³o clamp w xyz
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	memset(&samplerDesc.BorderColor, 0, sizeof(samplerDesc.BorderColor));
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = 0;
	V(pd3dDevice->CreateSamplerState(&samplerDesc, &pSamplerComparisonLinear));

}

void States::CreateRasterizerStates(ID3D11Device *pd3dDevice)
{
	HRESULT hr;

	// Rasterizer state with backface culling
	D3D11_RASTERIZER_DESC rasterizerState;
	rasterizerState.FillMode = D3D11_FILL_SOLID;
	rasterizerState.CullMode = D3D11_CULL_BACK;
	rasterizerState.FrontCounterClockwise = FALSE;
	rasterizerState.DepthBias = FALSE;
	rasterizerState.DepthBiasClamp = 0;
	rasterizerState.SlopeScaledDepthBias = 0;
	rasterizerState.DepthClipEnable = FALSE;
	rasterizerState.ScissorEnable = FALSE;
	// With D3D 11, MultisampleEnable has no effect when rasterizing triangles:
	// MSAA rasterization is implicitely enabled as soon as the render target is MSAA.
	rasterizerState.MultisampleEnable = FALSE;
	rasterizerState.AntialiasedLineEnable = FALSE;
	V(pd3dDevice->CreateRasterizerState(&rasterizerState, &pBackfaceCull_RS));
	rasterizerState.MultisampleEnable = TRUE;
	V(pd3dDevice->CreateRasterizerState(&rasterizerState, &pBackfaceCullMSAA_RS));
	rasterizerState.MultisampleEnable = FALSE;

	// Wireframe
	rasterizerState.CullMode = D3D11_CULL_NONE;
	rasterizerState.FillMode = D3D11_FILL_WIREFRAME;
	V(pd3dDevice->CreateRasterizerState(&rasterizerState, &pBackfaceCull_WireFrame_RS));
	rasterizerState.MultisampleEnable = TRUE;
	V(pd3dDevice->CreateRasterizerState(&rasterizerState, &pBackfaceCull_WireFrameMSAA_RS));
	rasterizerState.MultisampleEnable = FALSE;

	// Rasterizer state with no backface culling (for fullscreen passes)
	rasterizerState.CullMode = D3D11_CULL_NONE;
	rasterizerState.FillMode = D3D11_FILL_SOLID;
	V(pd3dDevice->CreateRasterizerState(&rasterizerState, &pNoCull_RS));
}

void States::CreateDepthStencilStates(ID3D11Device *pd3dDevice)
{
	HRESULT hr;

	D3D11_DEPTH_STENCIL_DESC depthstencilState;
	depthstencilState.DepthEnable = TRUE;
	depthstencilState.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthstencilState.DepthFunc = D3D11_COMPARISON_LESS;
	depthstencilState.StencilEnable = FALSE;
	V(pd3dDevice->CreateDepthStencilState(&depthstencilState, &pDepthNoStencil_DS));

	depthstencilState.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	V(pd3dDevice->CreateDepthStencilState(&depthstencilState, &pDS_Skybox));

	depthstencilState.DepthFunc = D3D11_COMPARISON_GREATER;
	V(pd3dDevice->CreateDepthStencilState(&depthstencilState, &pDS_DepthGreater));

	

	depthstencilState.DepthFunc = D3D11_COMPARISON_LESS;
	depthstencilState.DepthEnable = FALSE;
	V(pd3dDevice->CreateDepthStencilState(&depthstencilState, &pNoDepthNoStencil_DS));


	//------------------------------------------------------------------------
	// For stencil buffer - settings bits
	//------------------------------------------------------------------------

	// Disable depth & enable stencil
	depthstencilState.DepthEnable = FALSE;
	depthstencilState.StencilEnable = TRUE;

	// Read & write all bits
	depthstencilState.StencilReadMask = 0xff;
	depthstencilState.StencilWriteMask = 0xff;

	// Stencil operator for front face
	depthstencilState.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthstencilState.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthstencilState.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthstencilState.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;

	// Stencil operator for back face.
	depthstencilState.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthstencilState.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthstencilState.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthstencilState.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	V(pd3dDevice->CreateDepthStencilState(&depthstencilState, &pStencilSetBits_DS));


	depthstencilState.DepthEnable = FALSE;
	depthstencilState.StencilEnable = TRUE;
	depthstencilState.StencilReadMask = 0xff;
	depthstencilState.StencilWriteMask = 0xff;
	depthstencilState.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	depthstencilState.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthstencilState.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthstencilState.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthstencilState.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	depthstencilState.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthstencilState.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthstencilState.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	V(pd3dDevice->CreateDepthStencilState(&depthstencilState, &pNoDepthStencilComplex_DS));

	depthstencilState.DepthEnable = TRUE;
	depthstencilState.StencilEnable = TRUE;
	depthstencilState.StencilReadMask = 0xff;
	depthstencilState.StencilWriteMask = 0xff;
	depthstencilState.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	depthstencilState.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthstencilState.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthstencilState.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthstencilState.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	depthstencilState.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthstencilState.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthstencilState.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	V(pd3dDevice->CreateDepthStencilState(&depthstencilState, &pDepthStencilComplex_DS));
}

void States::CreateBlendStates(ID3D11Device *pd3dDevice)
{
	HRESULT hr;

	// Blending state with blending disabled
	D3D11_BLEND_DESC blendState;
	blendState.AlphaToCoverageEnable = FALSE;
	blendState.IndependentBlendEnable = FALSE; // new in D3D11
	for (int i = 0; i < 8; ++i)
	{
		blendState.RenderTarget[i].BlendEnable = FALSE;
		blendState.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}
	V(pd3dDevice->CreateBlendState(&blendState, &pNoBlend_BS));

	for (int i = 0; i < 8; ++i)
	{
		blendState.RenderTarget[i].BlendEnable = TRUE;
		blendState.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendState.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendState.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		blendState.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendState.RenderTarget[i].SrcBlendAlpha = blendState.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
	}
	V(pd3dDevice->CreateBlendState(&blendState, &pBlend_BS));

	// Additive blend
	for (int i = 0; i < 8; ++i)
	{
		blendState.RenderTarget[i].BlendEnable = TRUE;
		blendState.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
		blendState.RenderTarget[i].DestBlend = D3D11_BLEND_ONE;
		blendState.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		blendState.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendState.RenderTarget[i].SrcBlendAlpha = blendState.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
	}
	V( pd3dDevice->CreateBlendState( &blendState, &pBlendAdditive_BS ) );



	blendState.AlphaToCoverageEnable = TRUE;
	V(pd3dDevice->CreateBlendState(&blendState, &pBlendAlphaToCoverage_BS));
}

//--------------------------------------------------------------------------------------
// EOF
//--------------------------------------------------------------------------------------