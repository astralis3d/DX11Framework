#ifndef __CHROMATIC_ABERRATION_HLSL__
#define __CHROMATIC_ABERRATION_HLSL__


/* Chromatic Aberration */
void ChromaticAberration( in Texture2D TexColorBuffer, float2 uv, in float fChromaticAbberationIntensity, inout float3 color )
{
	// User-defined params
	float2 chromaticAberrationCenter = float2(0.5, 0.5);
	float chromaticAberrationCenterAvoidanceDistance = 0.20;		// chromatic aberration start
	float fA = 1.25;												// chromatic aberration range

	// Calculate vector
	float2 chromaticAberrationOffset = uv - chromaticAberrationCenter;
	chromaticAberrationOffset = chromaticAberrationOffset / chromaticAberrationCenter;

	float chromaticAberrationOffsetLength = length( chromaticAberrationOffset );

	// To avoid applying chromatic aberration in center, subtract small value from
	// just calculated length.
	float chromaticAberrationOffsetLengthFixed = chromaticAberrationOffsetLength - chromaticAberrationCenterAvoidanceDistance;
	float chromaticAberrationTexel = saturate( chromaticAberrationOffsetLengthFixed * fA );

	float fApplyChromaticAberration = (0.0 < chromaticAberrationTexel);
	if (fApplyChromaticAberration)
	{
		chromaticAberrationTexel *= chromaticAberrationTexel;
		chromaticAberrationTexel *= 0.50;								// chromatic aberration size

		chromaticAberrationOffsetLength = max( chromaticAberrationOffsetLength, 1e-4 );

		float fMultiplier = chromaticAberrationTexel / chromaticAberrationOffsetLength;

		chromaticAberrationOffset *= fMultiplier;
		chromaticAberrationOffset *= g_Viewport.zw;
		chromaticAberrationOffset *= fChromaticAbberationIntensity;

		float2 offsetUV = -chromaticAberrationOffset * 2 + uv;
		color.r = TexColorBuffer.SampleLevel( samplerLinearClamp, offsetUV, 0 ).r;

		offsetUV = uv - chromaticAberrationOffset;
		color.g = TexColorBuffer.SampleLevel( samplerLinearClamp, offsetUV, 0 ).g;
	}
}

#endif