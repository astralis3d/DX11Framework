#ifndef __CIECLEARSKY_HLSL__
#define __CIECLEARSKY_HLSL__

// CIE Clear Sky Analytical Model
// Adopted from Matt Pettineo's samples

//-------------------------------------------------------------------------------------------------
// Calculates the angle between two vectors
//-------------------------------------------------------------------------------------------------
float AngleBetween(in float3 dir0, in float3 dir1)
{
	return acos(dot(dir0, dir1));
}

//-------------------------------------------------------------------------------------------------
// Uses the CIE Clear Sky model to compute a color for a pixel, given a direction + sun direction
//-------------------------------------------------------------------------------------------------
float3 CIEClearSky(in float3 dir, in float3 sunDir)
{
	float3 skyDir = float3(dir.x, abs(dir.y), dir.z);
	float gamma = AngleBetween(skyDir, sunDir);
	float S = AngleBetween(sunDir, float3(0, 1, 0));
	float theta = AngleBetween(skyDir, float3(0, 1, 0));

	float cosTheta = cos(theta);
	float cosS = cos(S);
	float cosGamma = cos(gamma);

	float num = (0.91f + 10 * exp(-3 * gamma) + 0.45 * cosGamma * cosGamma) * (1 - exp(-0.32f / cosTheta));
	float denom = (0.91f + 10 * exp(-3 * S) + 0.45 * cosS * cosS) * (1 - exp(-0.32f));

	float lum = num / max(denom, 0.0001f);

	// Clear Sky model only calculates luminance, so we'll pick a strong blue color for the sky
	
    const float3 SkyColor = float3(0.2, 0.5, 1.0f) * 1;
	const float3 SunColor = float3(1.0, 0.8f, 0.3f) * 15;
	const float SunWidth = 0.025f;
	
	/*
	const float3 SkyColor = float3(0.2f, 0.5f, 1.0f) * 1;
	const float3 SunColor = float3(1.0f, 0.8f, 0.3f) * 1500;
	const float SunWidth = 0.015f;
	*/

	float3 color = SkyColor;

	// Draw a circle for the sun

	{
		float sunGamma = AngleBetween(dir, sunDir);
		color = lerp(SunColor, SkyColor, saturate(abs(sunGamma) / SunWidth));
	}

	return max(color * lum, 0);
}



#endif