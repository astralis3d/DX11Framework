#include "../Common/Common.hlsl"

struct VS_INPUT
{
	float3 Position : POSITION;
	float3 Color : COLOR;
};

struct VS_OUTPUT
{
	float4 PositionH : SV_POSITION;
	float4 Color : COLOR;
};

VS_OUTPUT VSSimple(VS_INPUT Input)
{
	VS_OUTPUT Output;

	Output.PositionH = mul(float4(Input.Position, 1.f), g_mtxView);
	Output.PositionH = mul(Output.PositionH, g_mtxProj);
	//Output.PositionH = mul( Output.PositionH, g_mtxProj);

	Output.Color = float4(Input.Color, 1.f);

	return Output;
}

float4 PSSimple(VS_OUTPUT Input) : SV_TARGET
{
	return Input.Color;
}