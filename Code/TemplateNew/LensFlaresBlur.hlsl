#ifndef __LENSFLARESBLUR_HLSL__
#define __LENSFLARESBLUR_HLSL__


#include "../Common/Common.hlsl"
#include "../Common/PostProcessCommon.hlsl"

// Calculates the gaussian blur weight for a given distance and sigmas
float CalcGaussianWeight( int sampleDist, float sigma )
{
	float twoSigma2 = 2 * sigma * sigma;
	float g = 1.0f / sqrt( PI * twoSigma2 );
	return (g * exp( -(sampleDist * sampleDist) / (twoSigma2) ));
}


// Performs a gaussian blur in one direction
float4 Blur( in VS_OUTPUT_POSTFX input, float2 texScale, float sigma )
{
	float2 TextureSize = g_Viewport.xy * 0.25;

	float4 color = 0;
	for (int i = -4; i < 4; i++)
	{
		float weight = CalcGaussianWeight( i, sigma );
		float2 texCoord = input.TextureUV;
		texCoord += (i / TextureSize) * texScale;
		float4 sample = InputTexture0.Sample( samplerPointClamp, texCoord );
		color += sample * weight;
	}
	

	return color;

}


float4 LensFlaresBlurV_PS( VS_OUTPUT_POSTFX Input ) : SV_Target
{
	return Blur( Input, float2(0, 1), 1.5 );
}

float4 LensFlaresBlurH_PS( VS_OUTPUT_POSTFX Input ) : SV_Target
{
	return Blur( Input, float2(1, 0), 1.5 );
}



#endif