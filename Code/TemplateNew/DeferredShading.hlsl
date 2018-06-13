#ifndef __DEFERREDSHADING_HLSL__
#define __DEFERREDSHADING_HLSL__

#include "../Common/Common.hlsl"
#include "DeferredShadingCommon.hlsl"
#include "../Common/PostProcessCommon.hlsl"
#include "../Common/CommonTextures.hlsl"

// GBuffer Render Targets
Texture2D TexDepth				: register (t1);
Texture2D RT0					: register (t2);
Texture2D RT1					: register (t3);
Texture2D RT2					: register (t4);
Texture2D TexHBAOPlus			: register (t5);
Texture2D ShadowMap				: register (t6);

//------------------------------------------------------------------------
//------------------------------------------------------------------------
//------------------------------------------------------------------------


GBufferData DecodeGBuffer(int3 location)
{
	GBufferData Output = (GBufferData) 0;
    

	// Read raw data from GBuffer
	float4 rt0 = RT0.Load( location );
	float4 rt2 = RT2.Load( location );
	
	// And write it to output
	Output.Albedo = ( rt0.xyz );
	Output.Albedo = GammaToLinear( Output.Albedo );

	Output.Roughness = rt0.w;

	Output.Normal = RT1.Load( location ).xyz;
	Output.Normal = normal_decode( Output.Normal );	// don't forget about normalizing!

	Output.Metallic = rt2.x;
	Output.MaterialID = (uint) rt2.y;

	// Determine world position
	float fDepthBufferDepth = TexDepth.Load( location ).x;
	float4 vWSPosition = mul( float4( (float) location.x + 0.5f, (float) location.y + 0.5f, fDepthBufferDepth, 1.0f), g_mtxInvViewProjViewport);
	Output.WorldPos = vWSPosition.xyz / vWSPosition.w;

	// Ambient Occlusion
	Output.AmbientOcclusion = TexHBAOPlus.Load( location ).x;
	

	return Output;
}

static const float RCP_SHADOWMAP = 1.0 / 2048.0;

float2 texOffset(int u, int v)
{
    const float shadowMapSize = 2048;
    return float2(u * RCP_SHADOWMAP, v * RCP_SHADOWMAP );
}

/////////////////////////////////////////////////////////
float4 PSMain( VS_OUTPUT_POSTFX Input ) : SV_TARGET0
{
    int3 location = int3(Input.Position.xy, 0);
	
	GBufferData GBuffer = DecodeGBuffer( location );
	float fDepthBufferDepth = TexDepth.Load( location ).x;
	fDepthBufferDepth = fDepthBufferDepth * g_depthScaleFactors.x + g_depthScaleFactors.y;

	float3 FinalColor = 0;

	if ( fDepthBufferDepth < 1.0 )
	{
		float3 Albedo = GBuffer.Albedo;
		
		float AO = GBuffer.AmbientOcclusion;

		float3 N = GBuffer.Normal;

		float3 L = normalize( -g_lightDir );



		float3 H = normalize( N + L );
		float3 V = normalize( g_CameraPos.xyz - GBuffer.WorldPos );

		float NdotL = max( 1e-5, dot( N, L ) );
	

		float  Roughness = GBuffer.Roughness;
		float  Metallic = GBuffer.Metallic;
		float3 SpecularColor = lerp( 0.04, Albedo.rgb, Metallic );

		// Poor man's ambient cubemap
		float3 ambientUp = Albedo * 0.3;
		float3 ambientDown = ambientUp * 0.5;

		float fAmbientBlend = 0.5 * N.y + 0.5;
		float3 Ambient = ambientUp * fAmbientBlend + ambientDown * (1.0 - fAmbientBlend);
		Ambient = Ambient * AO * float3(0.5, 0.55, 0.7);



		// --------------------------------
		// Shadows
		// Single shadow map
		// --------------------------------
		// 1) Re-homogenize position after interpolation (Todo: Research, why?)
		float4 lightSpacePos = mul( float4(GBuffer.WorldPos, 1.0), g_mViewProjShadow );
		lightSpacePos.xyz /= lightSpacePos.w;

		// Optional
		/*
		if (lightSpacePos.x < -1.0 || lightSpacePos.x > 1.0 ||
		lightSpacePos.y < -1.0 || qlightSpacePos.y > 1.0 ||
		lightSpacePos.z <  0.0 || lightSpacePos.z > 1.0)
		return float4(1, 0, 0, 1);
		*/


		// 2 ) Transform clip space coords to texture space coords :
		//	   [-1;1] -> [0;1]
		lightSpacePos.xy = lightSpacePos.xy * float2(0.5, -0.5) + 0.5;

		// 3) Apply bias
		lightSpacePos.z -= 0.003;

		// 4) PCF 4x4 sampling

		float sum = 0.0;
		
		[unroll] for (float y = -1.5; y <= 1.5; y += 1.0)
		{
			[unroll] for (float x = -1.5; x <= 1.5; x += 1.0)
			{
				sum += ShadowMap.SampleCmpLevelZero( samplerLinearComparison, lightSpacePos.xy + texOffset( x, y ), lightSpacePos.z );
			}
		}

		float shadowFactor = sum / 16.0;

		// Simple specular
		float3 reflLight = -reflect( L, N );
		float NdotH = max( 0.0, dot( N, H ) );

		float3 LightColor = 2.5;// * float3(1.0, 0.85, 0.6);
		float nShininess = 32;
		float specEnergyConservation = (8.0 + nShininess) / (8 * PI);
		float specular = pow( NdotH, nShininess ) * specEnergyConservation * GBuffer.Metallic;

		
		FinalColor = LightColor * Albedo *AO * NdotL  + specular;
		FinalColor += Ambient;
		FinalColor = lerp( Ambient, FinalColor, shadowFactor );

		
		// From "Shadows" sample by Matt Pettineo.
		//FinalColor += NdotL * LightColor * (1 / PI) * Albedo * shadowFactor;
		//FinalColor += float3(0.2f, 0.5f, 1.0f) * 0.1f * Albedo;
	}
	
    return float4( max(FinalColor, 1e-5), 1);
}


#endif