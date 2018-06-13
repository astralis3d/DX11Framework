#include "..//Common//Common.hlsl"

struct VS_INPUT
{
	float3 Position		: POSITION;
	float2 TextureUV	: TEXCOORD;
};

struct VS_OUTPUT
{
	float4 PositionH	: SV_POSITION;
};

// for Depth PrePass.
// TODO: World transform
VS_OUTPUT DepthVSPrePass(VS_INPUT Input)
{
	VS_OUTPUT Output;

	Output.PositionH = mul(float4(Input.Position, 1.f), g_mtxWorld);
	Output.PositionH = mul(Output.PositionH, g_mtxViewProj);

	return Output;
}

// for Shadow maps
VS_OUTPUT DepthVSShadowMap(VS_INPUT Input)
{
	VS_OUTPUT Output;

	Output.PositionH = mul(float4(Input.Position, 1.f), g_mtxWorld);
	Output.PositionH = mul(Output.PositionH, g_mViewProjShadow);
	/*
	Output.PositionH = mul(Output.PositionH, g_mtxView);
	Output.PositionH = mul(Output.PositionH, g_mtxProj);
	*/

	return Output;
}

void DepthPS(VS_OUTPUT Input)
{
	
}