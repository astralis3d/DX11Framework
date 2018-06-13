// The Witcher 3 Tonemapping implementation.
// Also bloom

#ifndef __TONEMAPPING_HLSL__
#define __TONEMAPPING_HLSL__

#include "..//Common//Common.hlsl"
#include "..//Common//PostProcessCommon.hlsl"
#include "..//Common//ColorCorrection.hlsl"
#include "ChromaticAberration.hlsl"

cbuffer cbToneMapping : register(b5)
{
	// Uncharted 2 A-F params
	float4	curveParamsABCD;
	float2	curveParamsEF;

    float	minAllowedSceneLuminance;
    float	maxAllowedSceneLuminance;

	float	paramWhitePoint;
	float	nominatorMultiplier;
	float	powParam;

	float	padding_unused;
}

Texture2D	TexHDRColor					: register (t0);
Texture2D	TexAvgLuminance				: register (t1);
Texture3D	TexColorCorrectionLUT		: register (t2);
Texture2D	TexDepth					: register (t3);
Texture2D	TexBloom					: register (t4);

Texture2D	TexLensFlares				: register (t5);
Texture2D	TexLensDirt					: register (t6);
Texture2D	TexLensStarburst			: register (t7);


// TODO: Move to different file
// http://filmicgames.com/archives/75
float3 U2Func( float A, float B, float C, float D, float E, float F, float3 x )
{
	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F)) - E/F;
}

float3 ToneMapU2Func( float A, float B, float C, float D, float E, float F, float3 color, float numMultiplier )
{
	float3 numerator = U2Func( A, B, C, D, E, F, color );
	numerator = max( numerator, 0 );
	numerator *= numMultiplier;

	float3 denominator = U2Func( A, B, C, D, E, F, 11.2 );
	denominator = max( denominator, 1e-5 );

	return numerator / denominator;
}

float4 ToneMappingPS( VS_OUTPUT_POSTFX Input) : SV_Target0
{
	// Get average scene luminance
	float avgLuminance = TexAvgLuminance.Load( uint3(0, 0, 0) ).r;
	avgLuminance = clamp( avgLuminance, minAllowedSceneLuminance, maxAllowedSceneLuminance );
	avgLuminance = max( avgLuminance, 1e-4 );

	float scaledWhitePoint = paramWhitePoint * 11.2;

	float luma = avgLuminance / scaledWhitePoint;
	luma = pow( luma, powParam );

	luma = luma * scaledWhitePoint;

	// in general, exposure is known in literature as middleGray/avgLogLuminance
	float exposure = paramWhitePoint / luma;
	
	// HDR color
	float3 HDRColor = TexHDRColor.Load( uint3(Input.Position.xy, 0) ).rgb;

	// Add bloom
	float3 vBloom = TexBloom.SampleLevel( samplerLinearClamp, Input.TextureUV, 0).rgb;
	HDRColor += 0.8*vBloom;

	// Add lens flares
	// (should be applied before any tonemapping)
	float3 vLensFlares = TexLensFlares.Sample( samplerLinearClamp, Input.TextureUV ).rgb;
	float3 vLensDirt = TexLensDirt.Sample( samplerLinearClamp, Input.TextureUV ).rgb;
	float3 vLensStar = TexLensStarburst.Sample( samplerLinearClamp, Input.TextureUV ).rgb;
	vLensStar *= 0.25;
	vLensDirt = vLensDirt * 0.75 + float3(0.0011, 0.0011, 0);
	HDRColor += vLensFlares * (vLensDirt + vLensStar);



	float3 tonemappedColor = ToneMapU2Func( curveParamsABCD.x, curveParamsABCD.y, curveParamsABCD.z, curveParamsABCD.w, curveParamsEF.x, curveParamsEF.y, HDRColor * exposure, nominatorMultiplier );


	return float4(tonemappedColor, 0);
}


#endif