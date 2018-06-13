#ifndef __POSTPROCESS_VIGNETTE_HLSL__
#define __POSTPROCESS_VIGNETTE_HLSL__

#include "..//Common//PostProcessCommon.hlsl"


// Unreal Engine 4 - Vignette
// (Camera artifact and artistic effect)
// @param VignetteCircleSpacePos from VignetteSpace()
float ComputeUE4VignetteMask(float2 VignetteCircleSpacePos, float Intensity)
{
	// Natural vignetting
	// cosine-fourth law
	VignetteCircleSpacePos *= Intensity;
	float Tan2Angle = dot(VignetteCircleSpacePos, VignetteCircleSpacePos);
	float Cos4Angle = Square(rcp(Tan2Angle + 1));
	return Cos4Angle;
}

// Implementation of natural vignette (cos^4 law)
float ComputeNaturalVignetteMask(in float2 Pos, in float Intensity)
{
	const float aspect = g_Viewport.w / g_Viewport.z;
	const float2 center = float2(0.5, 0.5);

	float2 coord = (Pos - center) * pow(aspect.xx, 1) * 2.0;
	float rf = sqrt(dot(coord, coord)) * Intensity;
	float rf2_1 = rf * rf + 1.0;
	float mask = rcp(rf2_1 * rf2_1);

	return mask;
}

float ComputeCornerVignetteMask(in float2 Pos)
{
	float dist = distance(Pos, float2(0.5, 0.5)) * 0.7;
	float RadiusX = 0.75;
	float RadiusY = 0.25;

	return smoothstep(RadiusX, RadiusY, dist);	
}

float ComputeTW3VignetteMask(in float2 Pos, in float dot3)
{
    dot3 = saturate(-dot3 + 1);

    float2 vignette = Pos - float2(0.5, 0.5);
    float d = length(vignette);
    d = d * 2 - 0.55;

    float d2 = saturate(d * 1.219512); // r2.w
    float d2_sqr = d2 * d2; // r2.z

        // r2.zw * r2.zz
    float2 test = float2(d2_sqr, d2) * d2_sqr;
    
    float4 stuff = float4(test, d2_sqr, d2);
    
    float dist = dot(stuff, float4(-0.100000, -0.105000, 1.120000, 0.090000));
    dist = min(dist, 0.94);
    dist = saturate(dist * dot3);

    return 1.f-dist;
}


float ComputeVignetteMask( in float2 Pos, in float Intensity )
{

#if	(VIGNETTE_TYPE == 0)
	return ComputeUE4VignetteMask( Pos, Intensity );

#elif (VIGNETTE_TYPE == 1)
	return ComputeNaturalVignetteMask( Pos, Intensity );

#elif (VIGNETTE_TYPE == 2)
	return ComputeCornerVignetteMask( Pos );

#elif (VIGNETTE_TYPE == 3)
    return ComputeTW3VignetteMask( Pos, Intensity );

#endif
}


#endif