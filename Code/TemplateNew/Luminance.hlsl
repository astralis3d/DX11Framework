#ifndef __LUMINANCE_HLSL__
#define __LUMINANCE_HLSL__

#include "..//Common//Common.hlsl"
#include "..//Common//PostProcessCommon.hlsl"

Texture2D TexInput : register(t0);

// Calculate log(luminance) to R16_FLOAT texture
float PSToneMappingLogLuminance( VS_OUTPUT_POSTFX Input ) : SV_TARGET
{
    float3 color = TexInput.SampleLevel( samplerAnisoWrap, Input.TextureUV, 0 ).rgb;

    float luminance = dot( color, LUMINANCE_RGB );
	luminance = clamp( luminance, 1e-5, 100000.0 );
    luminance = log( luminance );

    return luminance;
}

#endif