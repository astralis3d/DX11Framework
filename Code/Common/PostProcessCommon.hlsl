#ifndef __POSTPROCESS_COMMON_HLSL__
#define __POSTPROCESS_COMMON_HLSL__

#include "..//Common//Common.hlsl"


// Select vignette type:
// 0) Unreal Engine 4
// 1) Natural
// 2) Corner
// 3) TW3 (Prototype)
#define VIGNETTE_TYPE	3

cbuffer cbConstantsPP : register (b6)
{
    float2 InputSize0;
    float2 align;
};

Texture2D	InputTexture0	: register (t0);
Texture2D	InputTexture1	: register (t1);
Texture2D	InputTexture2	: register (t2);
Texture2D	InputTexture3	: register (t3);


// Vertex Shader for Post Processing
// http://www.slideshare.net/DevCentralAMD/vertex-shader-tricks-bill-bilodeau

struct VS_OUTPUT_POSTFX
{
	float4 Position									: SV_Position;
	float2 TextureUV								: TEXCOORD0;

#if (VIGNETTE_TYPE == 0)
	noperspective float2 OutExposureScaleVignette	: ATTRIBUTE0;

#elif (VIGNETTE_TYPE == 3)
    float2 Params                                   : ATTRIBUTE1;
#endif

};


// Scale {-1 to 1} of the viewport space to vignette circle space.
// Vignette space is scaled such that regardless of viewport aspect ratio, 
// corners are at the same brightness on a circle.
float2 VignetteSpace(float2 Pos)
{
	float4 ViewportSize = g_Viewport;

	// could be optimized but this computation should be done in the vertex shader (3 or 4 vertices)
	float Scale = sqrt(2.0) / sqrt(1.0 + (ViewportSize.y * ViewportSize.z) * (ViewportSize.y * ViewportSize.z));
	return Pos * float2(1.0, ViewportSize.y * ViewportSize.z) * Scale;
}


VS_OUTPUT_POSTFX QuadVS(in uint id : SV_VertexID)
{
	VS_OUTPUT_POSTFX Output;

	Output.Position.x = float(id / 2) * 4.0 - 1.0;
	Output.Position.y = float(id % 2) * 4.0 - 1.0;
	Output.Position.z = 0.0;
	Output.Position.w = 1.0;

	Output.TextureUV.x = (float)(id / 2) * 2.0;
	Output.TextureUV.y = 1.0 - (float)(id % 2) * 2.0;

#if (VIGNETTE_TYPE == 0)
	Output.OutExposureScaleVignette = VignetteSpace( Output.Position.xy );
#elif (VIGNETTE_TYPE == 3)
    Output.Params = Output.Position.xy * 0.5 + 0.5;
#endif


	return Output;
}


float4 QuadPSRecopy(VS_OUTPUT_POSTFX Input) : SV_Target0
{
	float4 color = InputTexture0.Sample(samplerLinearClamp, Input.TextureUV);
	return color;
}







#endif // __POSTPROCESS_COMMON_HLSL__