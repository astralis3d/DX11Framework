#ifndef __DEFERREDSHADINGCOMMON_HLSL__
#define __DEFERREDSHADINGCOMMON_HLSL__

struct GBufferData
{
	float3 Albedo;
	float Roughness;

	float3 Normal;
	float Metallic;

	float3 WorldPos;
	uint MaterialID;

	float AmbientOcclusion;
};


float3 normal_decode( in float3 enc )
{
	return normalize( enc * 2.0 - 1.0 );
}


#endif