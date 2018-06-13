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

#pragma once

#include <DXUT.h>

//--------------------------------------------------------------------------------------
// Rasterizer states, blend states, depth-stencil states, and sampler states
//--------------------------------------------------------------------------------------
class States
{
public:

	static void Claim();
	static void Destroy();
	static States* Get();

	ID3D11RasterizerState*		pBackfaceCull_RS;
	ID3D11RasterizerState*		pBackfaceCullMSAA_RS;
	ID3D11RasterizerState*		pBackfaceCull_WireFrame_RS;
	ID3D11RasterizerState*		pBackfaceCull_WireFrameMSAA_RS;
	ID3D11RasterizerState*		pNoCull_RS;

	ID3D11DepthStencilState*	pDepthNoStencil_DS;
	ID3D11DepthStencilState*	pNoDepthNoStencil_DS;
	ID3D11DepthStencilState*	pStencilSetBits_DS;

	ID3D11DepthStencilState*	pNoDepthStencilComplex_DS;
	ID3D11DepthStencilState*	pDepthStencilComplex_DS;
	ID3D11DepthStencilState*	pDS_Skybox;

	ID3D11DepthStencilState*	pDS_DepthGreater;

	ID3D11BlendState*			pNoBlend_BS;
	ID3D11BlendState*			pBlend_BS;
	ID3D11BlendState*			pBlendAdditive_BS;
	ID3D11BlendState*			pBlendAlphaToCoverage_BS;
	ID3D11BlendState*			pBS_Blend_NoColorWrite;


	ID3D11SamplerState*			pSamplerPointWrap;
	ID3D11SamplerState*			pSamplerPointClamp;
	ID3D11SamplerState*			pSamplerLinearWrap;
	ID3D11SamplerState*			pSamplerLinearClamp;
	ID3D11SamplerState*			pSamplerAnisoWrap;
	ID3D11SamplerState*			pSamplerAnisoClamp;
	ID3D11SamplerState*			pSamplerComparisonLinear;

	void CreateResources(ID3D11Device *pd3dDevice);
	void ReleaseResources();

private:
	void CreateSamplers(ID3D11Device *pd3dDevice);
	void CreateRasterizerStates(ID3D11Device *pd3dDevice);
	void CreateDepthStencilStates(ID3D11Device *pd3dDevice);
	void CreateBlendStates(ID3D11Device *pd3dDevice);

	States();
	~States();
};



//--------------------------------------------------------------------------------------
// EOF
//--------------------------------------------------------------------------------------