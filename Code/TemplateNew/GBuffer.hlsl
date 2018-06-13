#ifndef __GBUFFER_HLSL__
#define __GBUFFER_HLSL__

#include "../Common/Common.hlsl"

cbuffer MaterialCB	: register (b5)
{
	float3		g_BaseColor;
	float		g_Roughness;

	float2		padXYZ001;
	float		g_Metallic;
	float		g_MaterialID;
}

Texture2D		g_texAlbedo			: register (t1);
Texture2D		g_texOpacity		: register (t2);
Texture2D		g_texMetallic		: register (t3);
Texture2D		g_texNormalMap		: register (t4);
Texture2D		g_texNormalsFit		: register (t13);

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
    float4 Position0    : POSITION0;
    float4 PositionPrev : POSITION1;
	float3 Normal		: NORMAL;
	float3 Tangent		: TANGENT;
	float3 Bitangent	: BITANGENT;
	float2 TextureUV	: TEXCOORD0;
};

struct PS_OUTPUT
{
	float4 RT0 : SV_Target0;			// RGB: Albedo A: Roughness (What about HDR?)
	float4 RT1 : SV_Target1;			// Normals
	float4 RT2 : SV_Target2;			// R - Metallic, G - material ID
    float2 RT3 : SV_Target3;            // XY velocity buffer
};

float3 normal_encode_xyz(in float3 n)
{
	return n * 0.5 + 0.5;
}

// Vertex Shader for deferred rendering
VS_OUTPUT RenderSceneToGBufferVS( VS_INPUT Input)
{
	VS_OUTPUT Output = (VS_OUTPUT) 0;

	// Transform the position from object space to homogeneous projection space
	float4 PositionW = mul(float4(Input.Position, 1.0), g_mtxWorld);

    // SV_Position
    Output.PositionH = mul(PositionW, g_mtxViewProj);

    // For velocity buffer
    Output.Position0 = Output.PositionH;
    Output.PositionPrev = mul(PositionW, g_mtxViewProjPrevFrame);


	// Normal & Tangent in world space
	Output.Normal =  mul( Input.Normal,  (float3x3) g_mtxWorld );
	Output.Tangent = mul( Input.Tangent, (float3x3) g_mtxWorld );

	Output.Bitangent = normalize( cross( Output.Normal, Output.Tangent ) );

	// Just pass texture uv further
	Output.TextureUV = Input.TextureUV;

	return Output;
}


// Pixel shader for feeding the GBuffer.
// PLEASE NOTE: This shader has many variations (like loading albedo from cbuffer OR texture) and so on.
PS_OUTPUT RenderSceneToGBufferPS( VS_OUTPUT Input, uint bIsFrontFace : SV_IsFrontFace )
{
	PS_OUTPUT Output = (PS_OUTPUT) 0;
	
	// Albedo: Ouput.RT0.rgb
#if (ALBEDO_FROM_TEXTURE == 1)
	
	
	uint2 texDim;
	g_texOpacity.GetDimensions(texDim.x, texDim.y);

	if (texDim.x != 0U)
	{
		float op = g_texOpacity.Sample( samplerAnisoWrap, Input.TextureUV ).r;
		clip(op-0.3);
	}

	float4 Albedo = g_texAlbedo.Sample( samplerAnisoWrap, Input.TextureUV );
	//float4 Albedo = float4(Input.TextureUV.y, 0, 0, 0);

#else
    float3 Albedo = g_BaseColor;	// Albedo from cbuffer value
#endif

	Albedo = (Albedo);


	
	// Roughness: Output.RT0.a
#if (ROUGHNESS_FROM_TEXTURE == 1)
	float Roughness = g_texRoughness.Sample( samplerAnisoWrap, Input.TextureUV ).r;
#else
	float Roughness = g_Roughness;		// Roughness from cbuffer value
#endif

	// Metallic: Output.RT2.r
#if (METALLIC_FROM_TEXTURE == 1)
	float Metallic = g_texMetallic.Sample( samplerAnisoWrap, Input.TextureUV ).r;
#else
	float Metallic = g_Metallic;		// Metallic from cbuffer value
#endif

	// Normal: Output.RT1.rgb (R11G11B10_FLOAT)
#if (USE_NORMAL_MAPPING == 1)
	
	float3 vNorm;

	uint2 texNormalDim;
	g_texNormalMap.GetDimensions(texNormalDim.x, texNormalDim.y);

	// There is normal map
	if (texNormalDim.x != 0U)
	{
		float3 sampledNormal = g_texNormalMap.Sample( samplerAnisoWrap, Input.TextureUV ).xyz;
		float3 biasedNormal = sampledNormal * 2.0 - 1.0;

		float3x3 TBNMatrix = float3x3(Input.Tangent, Input.Bitangent, Input.Normal);
		float3x3 BTNMatrix = float3x3(Input.Bitangent, Input.Tangent, Input.Normal);
		vNorm = normalize( mul( biasedNormal, TBNMatrix ) );

		
		bool bTest = (0 >= bIsFrontFace);
		[branch] if (bTest)
		{
			float NormalDotXferedNormal = dot( Input.Normal, vNorm );
			float3 helper = NormalDotXferedNormal * Input.Normal;
			vNorm = -helper * 2 + vNorm;
			Albedo.rgb = float3(1, 0, 1);
		}
		
	}
	// No normal map
	else
	{
		vNorm = normalize(Input.Normal);
	}

#else
	float3 vNorm = normalize(Input.Normal);
#endif

	CompressUnsignedNormalToNormalsBuffer( g_texNormalsFit, vNorm );
	



    // Calculate velocity buffer
    float2 currentPos = Input.Position0.xy / Input.Position0.w;
    currentPos = currentPos * float2(0.5, 0.5) + float2(0.5, 0.5);

    float2 previousPos = Input.PositionPrev.xy / Input.PositionPrev.w;
    previousPos = previousPos * float2(0.5, 0.5) + float2(0.5, 0.5);
    
    float2 velocity = currentPos - previousPos;
    
	// Feeding GBuffer
	Output.RT0 = float4(Albedo.rgb, Roughness);
	Output.RT1.xyz = normal_encode_xyz(vNorm);
	Output.RT2 = float4(Metallic, g_MaterialID, 0.0, 0.0);
    Output.RT3 = currentPos - previousPos;
	
	return Output;
}


#endif