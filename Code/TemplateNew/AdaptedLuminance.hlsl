#ifndef __ADAPTEDLUMINANCE_HLSL__
#define __ADAPTEDLUMINANCE_HLSL__

#include "..//Common//Common.hlsl"
#include "..//Common//PostProcessCommon.hlsl"

Texture2D TexCurrentLuminance   : register(t0);           // Current luminance with mipmaps
Texture2D TexLastLuminance      : register(t1);           // Last luminance

// The Witcher 3 eye adaptation (currently used)
float PSToneMappingCalcAdaptedLuminance(VS_OUTPUT_POSTFX Input) : SV_TARGET
{
	// Get current luminance.
   	float currentLuminance = TexCurrentLuminance.Load( uint3(0, 0, 8) ).x;
	currentLuminance = exp(currentLuminance);

	// Get last luminance.
	float lastLuminance = TexLastLuminance.SampleLevel( samplerPointClamp, float2(0, 0), 0 ).r;

	// Calculate adapted luminance.
	// TODO: Speeds from cbuffer
	float luminanceDifference = currentLuminance - lastLuminance;
	float adaptedLum = 0.11 * luminanceDifference + lastLuminance;  

    return adaptedLum;
}

// From RedBud Engine (roar11.com)
float PSToneMappingCalcAdaptedLuminance_RedBud( in VS_OUTPUT_POSTFX Input ) : SV_TARGET
{	
    // TODO: Maybe mip from cbuffer?
   	float currentLuminance = TexCurrentLuminance.Load( uint3(0, 0, 9) ).x;
	currentLuminance = exp(currentLuminance);
   
    // Some hacking from RedBud's (roar11.com) assembly
    float beta = currentLuminance + 0.04;
    beta = 0.04 / beta;
       
    float theta = -beta + 1.0;
    theta *= 0.4;
    beta = beta * 0.2 + theta;

	float lastLuminance = TexLastLuminance.SampleLevel( samplerPointClamp, float2(0, 0), 0 ).r;

    // Adapt the luminance using Pattanaik's technique
    float adaptedLum = lastLuminance + (currentLuminance - lastLuminance) * (1.0 - exp(-g_fTimeDelta * beta));
  
    return adaptedLum;
}

#endif
