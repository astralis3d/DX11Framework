#ifndef __BLOOM_HLSL__
#define __BLOOM_HLSL__

#include "../Common/Common.hlsl"
#include "../Common/PostProcessCommon.hlsl"

// Calculates the gaussian blur weight for a given distance and sigmas
float CalcGaussianWeight(int sampleDist, float sigma)
{
    float twoSigma2 = 2 * sigma * sigma;
    float g = 1.0f / sqrt( PI * twoSigma2);
    return (g * exp(-(sampleDist * sampleDist) / (twoSigma2)));
}

static const float GaussianWeights[] = { 0.028532, 0.067234, 0.124009, 0.179044, 0.20236, 0.179044, 0.124009, 0.067234, 0.028532 };

// Performs a gaussian blur in one direction
float4 Blur(in VS_OUTPUT_POSTFX input, float2 texScale, float sigma)
{	

    float4 color = 0;
	int i = -4;
    [unroll] for (; i < 0; i++)
    {
        float weight = CalcGaussianWeight(i, sigma);
		//float weight = GaussianWeights[ i + 4 ];
        float2 texCoord = input.TextureUV;
        texCoord += (i / InputSize0) * texScale;
        float4 sample = InputTexture0.Sample(samplerPointClamp, texCoord);
        color += sample * weight;
    }

	[unroll] for (i = 1; i <= 4; i++)
	{
		float weight = CalcGaussianWeight(i, sigma);
		//float weight = GaussianWeights[i + 4];
		float2 texCoord = input.TextureUV;
		texCoord += (i / InputSize0) * texScale;
		float4 sample = InputTexture0.Sample( samplerPointClamp, texCoord );
		color += sample * weight;
	}
	
		/*
	float4 color = 0;
	for (int i = -4; i < 4; i++)
	{
		float weight = CalcGaussianWeight( i, sigma );
		float2 texCoord = input.TextureUV;
		texCoord += (i / InputSize0) * texScale;
		float4 sample = InputTexture0.Sample( samplerPointClamp, texCoord );
		color += sample * weight;
	}
	*/
	
    return color;
	
}

float4 BlurV_PS( VS_OUTPUT_POSTFX Input ) : SV_Target
{
    return Blur(Input, float2(0, 1), 1.5);
}

float4 BlurH_PS(VS_OUTPUT_POSTFX Input) : SV_Target
{
    return Blur(Input, float2(1, 0), 1.5);
}

float4 BloomThreshold_PS( VS_OUTPUT_POSTFX Input) : SV_Target
{
    float4 color = InputTexture0.SampleLevel(samplerLinearClamp, Input.TextureUV, 0);
    color -= 1.0;

    color = max(color, 0);	

//	color = 0;

    return color;
}


#endif