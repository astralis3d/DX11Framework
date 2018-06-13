#ifndef __SKYBOXVELOCITY_HLSL__
#define __SKYBOXVELOCITY_HLSL__

#include "..//Common//Common.hlsl"

struct VS_INPUT
{
	float3 Position		: POSITION;
	float2 TextureUV	: TEXCOORD;
};

struct VS_OUTPUT
{
	float4 PositionH	: SV_POSITION;
	float4 Position0    : POSITION0;
	float4 PositionPrev : POSITION1;
};

// Vertex Shader
VS_OUTPUT SkyboxVelocityBufferVS( VS_INPUT Input )
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	// Transform the position from object space to homogeneous projection space
	float4 PositionW = mul( float4(Input.Position, 1.0), g_mtxWorld );

	// SV_Position
	Output.PositionH = mul( PositionW, g_mtxViewProj );

	// For velocity buffer
	Output.Position0 = Output.PositionH;
	Output.PositionPrev = mul( PositionW, g_mtxViewProjPrevFrame );

	return Output;
}

float2 SkyboxVelocityBufferPS( VS_OUTPUT Input ) : SV_Target0
{
	// Calculate velocity buffer
	float2 currentPos = Input.Position0.xy / Input.Position0.w;
	currentPos = currentPos * float2(0.5, 0.5) + float2(0.5, 0.5);

	float2 previousPos = Input.PositionPrev.xy / Input.PositionPrev.w;
	previousPos = previousPos * float2(0.5, 0.5) + float2(0.5, 0.5);

	float2 velocity = currentPos - previousPos;

	return velocity;
}

#endif