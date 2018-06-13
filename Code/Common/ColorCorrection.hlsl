#ifndef __COLORCORRECTION_HLSL__
#define __COLORCORRECTION_HLSL__

static const float LUT_SCALE_16  = 0.9375;	// 15.0 / 16.0
static const float LUT_OFFSET_16 = 0.03125;	// 0.5  / 16.0

float3 ColorCorrection(in Texture3D tex3DLUT, in float3 inColor)
{
	//if (inColor.r > 1 || inColor.r < 0 || inColor.g > 1 || inColor.g < 0 || inColor.b <0 || inColor.b > 1)
		//return float4(1,0,0,1);

	float3 lookup = saturate(inColor) * LUT_SCALE_16 + LUT_OFFSET_16;

	return tex3DLUT.SampleLevel(samplerLinearClamp, lookup, 0).rgb;
}


#endif