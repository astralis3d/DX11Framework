#include "../Common/Common.hlsl"
#include "../Common/CommonTextures.hlsl"
#include "BRDF.hlsl"

// Textures
Texture2D g_texDiffuse	: register(t1);
Texture2D g_texNormal	: register(t2);
Texture2D g_texSpecular : register(t3);
Texture2D g_texOpacity	: register(t4);
Texture2D g_Shadowmap	: register(t5);

// Input/Output Vertex
struct VS_INPUT
{
	float3 Position		: POSITION;
	float2 TextureUV	: TEXCOORD;
	float3 Normal		: NORMAL;
	float3 Tangent		: TANGENT;
	float3 Bitangent	: BITANGENT;
};

struct VS_OUTPUT
{
	float4 PositionH	: SV_POSITION;
	float3 PositionW	: POSITION;
	float3 Normal		: NORMAL;
	float3 Tangent		: TANGENT;
	float3 Bitangent	: BITANGENT;
	float2 TextureUV	: TEXCOORD0;
	float4 LPositionH	: TEXCOORD1;
};

VS_OUTPUT VS(VS_INPUT Input)
{
	VS_OUTPUT Output;

	Output.PositionH = mul(float4(Input.Position, 1.f), g_mtxWorld);
	Output.PositionW = Output.PositionH.xyz;
	Output.PositionH = mul(Output.PositionH, g_mtxViewProj);

	Output.Normal = mul(Input.Normal, (float3x3) g_mtxWorld);
	Output.Tangent = mul(Input.Tangent, (float3x3) g_mtxWorld);
	Output.Bitangent = mul(Input.Bitangent, (float3x3) g_mtxWorld);

	Output.TextureUV.xy = Input.TextureUV.xy;

	Output.LPositionH = mul(float4(Output.PositionW, 1.f), g_mViewProjShadow);

	return Output;
}


float CalculateShadowFactor(in float4 LightPositionH, in Texture2D ShadowMap, in SamplerComparisonState PCFSampler)
{
	float Shadow = 1.f;

	// Projective texturing
	float2 projected;
	projected.x =  LightPositionH.x / LightPositionH.w * 0.5f + 0.5f;
	projected.y = -LightPositionH.y / LightPositionH.w * 0.5f + 0.5f;

	if (saturate(projected.x) == projected.x && saturate(projected.y) == projected.y)
	{
		const float dx = 1.f / 2048.f;
		const float bias = 0.004f;

		float lightDepthValue = LightPositionH.z / LightPositionH.w; // - bias;
		float shadowMapDepth = ShadowMap.SampleLevel(samplerLinearClamp, projected, 0).r;

		float sum = 0;
		float x, y;
		[unroll] for (y = -1.5f; y <= 1.5f; y++)
		{
			[unroll] for (x = -1.5f; x <= 1.5f; x++)
			{
				sum += g_Shadowmap.SampleCmpLevelZero(PCFSampler, projected.xy + float2(dx*x, dx*y), lightDepthValue);
			}
		}

		Shadow = sum / 16.f;
	}

	return Shadow;
}

//----------------------------------------------------------------------
float4 PS(VS_OUTPUT Input) : SV_TARGET0
{
#ifdef USE_OPACITY_MAPPING
	const float Opacity = g_texOpacity.Sample(samplerAnisoWrap, Input.TextureUV.xy).r;
	if (Opacity < 0.5f)
		discard;
#endif		

#ifdef USE_NORMAL_MAPPING

	float3 bumpMapSample = g_texNormal.Sample(samplerAnisoWrap, Input.TextureUV.xy).xyz;
	bumpMapSample = bumpMapSample * 2.f;
    bumpMapSample -= float3(1, 1, 1);
    
    float3 vBinorm = normalize( cross(Input.Normal, Input.Tangent) );
    float3x3 BTNMatrix = float3x3(vBinorm, Input.Tangent, Input.Normal);


	float3 Normal =  mul(bumpMapSample, BTNMatrix);
	
#else
	float3 Normal = Input.Normal;
#endif

	Normal = normalize(Normal);	

	float4 Albedo = g_texDiffuse.Sample(samplerAnisoWrap, Input.TextureUV.xy);
	
	/*Albedo.rgb = float3(.5, .5, .5);*/

	Albedo.rgb = GammaToLinear(Albedo.rgb);
	

	float3 N = Normal;
	float3 L = -g_lightDir;

	float roughness = 0.4f;


	float NdotL = saturate( dot(Normal, L) );


	float Shadow = CalculateShadowFactor(Input.LPositionH, g_Shadowmap, samplerLinearComparison);

	// Ambient
    float3 Ambient = Albedo * 0.1f; // CalcAmbient(Normal, Albedo.rgb);

	// Specular mapping
	float3 vReflect = normalize(reflect(g_lightDir, Normal));
	float specIntensity = g_texSpecular.Sample(samplerLinearWrap, Input.TextureUV.xy).r;

	float3 V = normalize(g_CameraPos.xyz - Input.PositionW);
	float specular = pow( saturate(dot(vReflect, V)), 32);

	float3 spec = specIntensity * specular;

	float3 H = normalize( V + L );

	float VoH = saturate( dot(V, H) );
	float NoH = saturate( dot(N, H) );
	float NoV = saturate( dot(N, V) );



	//float3 Diffuse = Albedo.rgb;
	//float3 Diffuse = Diffuse_Lambert(Albedo.rgb);
	float3 Diffuse = Diffuse_OrenNayar(Albedo.rgb, roughness, NoV, NdotL, NoH);
		
	float3 finalAttenuation = Shadow;

	float3 finalAmbient = Ambient;
	float3 finalDiffuse = Diffuse * finalAttenuation;
	
	float3 finalColor = finalAmbient + finalDiffuse;


	return float4(finalColor, Albedo.a);
}

float4 PS_ShadowMask(VS_OUTPUT Input) : SV_TARGET
{
	// Non-optimal
	float2 projected;
	projected.x = Input.LPositionH.x / Input.LPositionH.w / 2.f + 0.5f;
	projected.y = -Input.LPositionH.y / Input.LPositionH.w / 2.f + 0.5f;

	if (saturate(projected.x) == projected.x && saturate(projected.y) == projected.y)
	{
		float dx = 1.f / 2048;

		float lightDepthValue = Input.LPositionH.z / Input.LPositionH.w - 0.006f;
		float shadowMapDepth = g_Shadowmap.SampleLevel(samplerLinearClamp, projected, 0).r;

		float sum = 0;
		float x, y;
		for (y = -1.5f; y <= 1.5f; y += 0.5f)
			for (x = -1.5f; x <= 1.5f; x += 0.5f)
			{
				sum += g_Shadowmap.SampleCmpLevelZero(samplerLinearComparison, projected.xy + float2(dx*x, dx*y), lightDepthValue);
			}

		float sFactor = sum / 16.f;

		return sFactor.rrrr;
	}

	return float4(1.f, 1.f, 1.f, 1.f);
}