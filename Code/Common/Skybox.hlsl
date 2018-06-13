#ifndef __SKYBOXOLD_HLSL__
#define __SKYBOXOLD_HLSL__

#include "..//Common//Common.hlsl"
#include "..//Common//CommonTextures.hlsl"
#include "..//Common//CIEClearSky.hlsl"

TextureCube	TexCubemap	: register (t0);
Texture2D	TexDiffuse	: register (t1);
Texture2D	TexFalloff	: register (t2);
Texture2D	TexClouds	: register (t3);
Texture2D	TexPerlin	: register (t4);
Texture2D	TexNormals	: register (t5);

Texture2D	TexDepth	: register (t10);

cbuffer cbClouds : register (b5)
{
	// MUST HAVE PARAMS:
	// - move speed ( float )
	// - UV scale  ( float2 )
	// - perlin scale (float)
	//  -brightness (float)

	float4 cloudsParams0;		// xy: uv scale, zw - move vector
	float4 cloudsParams1;		// x: move speed, y: perlin scale, z: brightness, w: test alpha
}

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
	float2 TextureUV	: TEXCOORD;
	float3 Normal		: NORMAL;
	float3 Tangent		: TANGENT;
	float3 Bitangent	: BITANGENT;
	float HeightRatio	: CLOUDHEIGHT;
	float FarPlane		: FARPLANE;
};

VS_OUTPUT VS(in VS_INPUT Input)
{
	VS_OUTPUT Output;

	Output.PositionH = mul(float4(Input.Position, 1.0), g_mtxWorld);
	Output.PositionW = Output.PositionH.xyz;
	Output.PositionH = mul(Output.PositionH, g_mtxViewProj).xyzw;
		
	
    Output.TextureUV = Input.TextureUV;
	Output.HeightRatio = pow ( (Input.Position.y * 0.0013), 3);

	Output.Bitangent = normalize( cross( Input.Normal, Input.Tangent ) );
	Output.Normal = normalize(Input.Normal);
	Output.Tangent = normalize(Input.Tangent);

	// far plane
	Output.FarPlane = Output.PositionH.w;
	
	return Output;
}


float4 PS_CIEClearSky(in VS_OUTPUT Input) : SV_Target
{
	/* CIEClearSky Model */	
	
	const float3 PositionW = normalize( Input.PositionW );
	const float3 LightDirection = normalize( -g_lightDir );
	float3 FinalColor = CIEClearSky( PositionW, LightDirection );
	
	float Bias = 1;

	return float4(FinalColor * Bias, 1);


	/* Private Research - Witcher 3 sky model */
	float3 PositionWorld = Input.PositionW;
	float3 vCameraToWorld = PositionWorld - g_CameraPos.xyz;
	float fDistanceCameraToWorld = length( vCameraToWorld );

	float3 vCameraToWorldNorm = vCameraToWorld / fDistanceCameraToWorld;

	float LightDotView = dot ( normalize(g_lightDir), vCameraToWorldNorm );

	float LightDotViewX2 = abs( LightDotView ) * abs( LightDotView );

	float length_param = saturate( 15000 * 0.0002 - 0.3 );
	LightDotViewX2 *= length_param;

	bool bResult = (LightDotView > 0.0);

	float3 fogColorFront = float3( 4.32, 6.117, 8.54);
	float3 fogColorMiddle = float3( 2.5, 3.86, 5.54 );
	float3 fogColorBack = float3( 1.39, 1.22, 1.891 );


	float3 colorFogBackOrFront = (bResult) ? fogColorFront : fogColorBack;
	float3 fogColor = lerp( fogColorMiddle, colorFogBackOrFront, LightDotViewX2 );


	// Sky Stuff

	// params
	float4 skyColor = float4( 0.3636, 0.30663, 0.29765, 1);		// 200
	float4 skyColorHorizon = float4( 0.45364, 0.63371, 0.81951, 0.81851);	 // 201
	float4 sunColorHorizon = float4(1, 4.33, 0.94, 0);	// cb4_v202
	float4 sunBackHorizonColor = float4( 120.00, -0.95, 0.28, 0.05 ); // 203
	float4 sunColorSky = float4( 22.85993, -0.95456, 0.28026, -0.10133 ); //204
	float4 moonColorHorizon = float4( 1, 0.0209, 0, 0);	// 205

	// from line 99
	float3 vWorldToCamera = normalize( g_CameraPos.xyz - PositionWorld );

	float2 r0_xy = saturate( moonColorHorizon.yx );

	float worldToCamDot = length( -vWorldToCamera );
	worldToCamDot = 1.0 / worldToCamDot;

	// line 105
	vWorldToCamera = vWorldToCamera * worldToCamDot - vWorldToCamera;

	// line 106
	float2 sunColorSkyVector = normalize( sunColorSky.yz );	
	//float2 sunColorViewVector = normalize( vWorldToCamera.
	
	// line 113
	float r2_x = saturate( dot( vWorldToCamera, sunColorSky.ywz ) );	// ywz, cause Y-up

	float r0_z = vCameraToWorld.y / fDistanceCameraToWorld;
	r0_z = r0_z * 1000 + g_CameraPos.y;
	r0_z += 710.0;
	r0_z *= sunColorHorizon.z;
	r0_z = r0_z * 0.001 + 0.1;
	r0_z = max( r0_z, 1e-4);
	r0_z = 1.0 / r0_z;
	r0_z = min( r0_z, 1);
	r0_z = pow( r0_z, 2.8 );
	r0_z = -r0_z + 1;



	float3 finalColor = lerp( skyColor.rgb, fogColor.rgb, r0_z );

	//r2_x = pow( r2_x, sunColorSky.x );
	

    return float4(fogColor.rgb, 1);
}


// Texture-based sky
float4 PS(in VS_OUTPUT Input) : SV_Target
{
#ifdef USE_ENVMAP
	float3 ColorDiffuse = g_texEnvironment.SampleLevel(samplerLinearWrap, Input.PositionW * float3(1, -1, 1), 0).rgb;
	ColorDiffuse = GammaToLinear( ColorDiffuse );
	
#else
	float3 ColorDiffuse = TexDiffuse.Sample(samplerLinearWrap, Input.TextureUV).rgb;
	ColorDiffuse += float3(0.03, 0.11, 0.7) * 0.2;
	ColorDiffuse = GammaToLinear(ColorDiffuse);
//	ColorDiffuse = pow( ColorDiffuse, 1.1 );
#endif

	return float4(ColorDiffuse.rgb, 1);
}


float4 PS_Clouds2(in VS_OUTPUT Input) : SV_Target
{	
	
	

	// perlin noise
	float2 uvPerlin = g_fTime * float2(-0.0005, 0.000);
	float2 texcoordsPerlin = Input.TextureUV * float2(0.5, 0.5) + uvPerlin;
	float xNoise = TexPerlin.Sample( samplerAnisoWrap, texcoordsPerlin ).x;
	xNoise = xNoise * 1.0 - 1.0;
	xNoise = saturate( 1 + xNoise );
	xNoise = saturate( xNoise );



	float2 uv = g_fTime * float2(0.12, 0.04);
	float2 texcoords = Input.TextureUV * float2(0.75, 0.75) + uv;
	
	float4 colorClouds = TexClouds.Sample( samplerLinearWrap, texcoords + xNoise * 0.05 ).xxxx;
	colorClouds = pow(colorClouds, 2.2);

	//colorClouds *= xNoise;
	
	float2 normals = TexNormals.Sample( samplerLinearWrap, texcoords ).xy;
	normals = normals * 2.0 - 1.0;

	// calc Z component
	float normals_z = -normals.x * normals.x + 1.0;
	normals_z = -normals.y * normals.y + normals_z;
	normals_z = max( 0, normals_z );

	float3 vNorm = float3(normals.x, normals_z, normals.y);
	//vNorm *= float3(0.5, 1.0, 0.5);
	vNorm = normalize( vNorm );


	float2 lightVector2D = float2(g_lightDir.x, g_lightDir.z);
	lightVector2D = normalize( lightVector2D );



	float3 worldToCameraXYZ = g_CameraPos.xyz - Input.PositionW;
	float worldToCameraDistanceXYZ = length( worldToCameraXYZ );

	float2 worldToCameraXZ = float2(worldToCameraXYZ.x, worldToCameraXYZ.z);
	worldToCameraXZ = normalize( worldToCameraXZ );
	worldToCameraXYZ /= worldToCameraDistanceXYZ;


	float LdotV = saturate( dot( worldToCameraXZ, lightVector2D ) );
	//LdotV = rcp(LdotV);
	//LdotV = 1 / LdotV;
	//LdotV = min(1, LdotV);

	float NdotL = saturate( dot( -g_lightDir, vNorm ) );


	//return float4(LdotV.xxx,1);

	float Depth = TexDepth.Load( uint3(Input.PositionH.xy, 0) ).r;
	Depth = Depth * g_depthScaleFactors.x + g_depthScaleFactors.y;
	Depth = Depth * g_cameraNearFar.x + g_cameraNearFar.y;
	Depth = max( Depth, 0.0001 );

	float FarPlane = 1.0 / Depth;

	float SkyboxFarPlane = Input.FarPlane;
	FarPlane -= SkyboxFarPlane;

	FarPlane = saturate( FarPlane * 0.0001 );	// cb4_v18.x

	float factor = (colorClouds.x);

	float3 colorBase = colorClouds.xyz;
	float3 colorDarken = colorClouds.rgb * NdotL * LdotV;

	float xFactor = dot( vNorm.xz, g_lightDir.xz );
	xFactor=saturate(xFactor);

	float3 colorLerped = lerp( colorBase, colorDarken, xFactor);
	

	//NdotL = lerp(NdotL, 1, LdotV);
	
    return float4(colorLerped, 0);
}




// for small altocumulus
float4 PS_Clouds( in VS_OUTPUT Input ) : SV_Target
{
	// Based on rastertek.com/tertut12.html
	float2 translation = g_fTime * cloudsParams0.zw;
	//translation *= cloudsParams1.x;
	//Input.TextureUV.xy += translation;


	//Input.TextureUV *= cloudsParams0.xy;

	float2 perturbValue = TexPerlin.SampleLevel( samplerAnisoWrap, Input.TextureUV, 0 ).xy;
	perturbValue *= 0.02;	// multiply the perturb value by the perturb scale
	
	// Add the texture coordinates as well as the translation value to get the perturbed texture coordinate sampling location.
	//perturbValue.xy = perturbValue.xy + Input.TextureUV.xy + translation;

	float2 uv = g_fTime * 1 * float2(0.2, 0.01);
	uv = Input.TextureUV.xy + uv;

	//float4 colorClouds = TexClouds.Sample(samplerAnisoWrap, Input.TextureUV + translation);
	float4 colorClouds = TexClouds.Sample( samplerAnisoWrap, uv  );
	
	
	float3 vNormal = colorClouds.xyz - 0.5;
	vNormal *= float3( 0.25f, 1.f, 0.25);
	vNormal = normalize( vNormal * 2 );


	float3x3 TBNMatrix = float3x3(Input.Tangent, Input.Bitangent, Input.Normal);
	vNormal = normalize( mul( vNormal, TBNMatrix ) );

	// r1.xyz, line 15
	float3 vInvSunDir = g_lightDir * float3(1, -1, 1);
	vInvSunDir = normalize( vInvSunDir );

	// r0.x, line 16
	float NormalDotInvSunDir = saturate( dot( vNormal, vInvSunDir) );
	
	// line 17
	float4 cb4_v5 = float4(1, 1, 1, 1);
	float4 cb4_v6 = float4(3, 3, 3, 3);
	float4 cb4_v7 = float4(3, 3, 3, 3);
	float4 cb4_v8 = float4(1.25, 1.25, 1.25, 1.25);
	float4 cb4_v9 = float4(1, 1, 1, 1);
	float4 cb4_v10 = float4(0.5, 0.5, 0.5, 0.5);
	float4 cb4_v11 = float4(1.5, 1.5, 1.5, 1.5);

	float param = cb4_v7.x - cb4_v6.x;
	param = NormalDotInvSunDir * param + cb4_v6.x;
	param = clamp( param, 0.0, 5.0 );
	param -= cb4_v5.x;

	float3 InvSunDir = normalize(g_lightDir);
	
	// line 25
	float3 SkyboxWorldPosition = Input.PositionW;
	float SkyboxFarPlane = Input.FarPlane;

	float3 cameraToWorld = g_CameraPos.xyz - SkyboxWorldPosition;
	float cameraToWorldDistance = length( cameraToWorld );	// r0.z, line 29
	cameraToWorld = normalize( cameraToWorld ); // line 32, r2.xyz

	float ViewDotLight = saturate( dot(cameraToWorld, vInvSunDir) );	// r1.x, line 33

	float r0_y = ViewDotLight * param + cb4_v5.x;
	float r1_y = cb4_v9.x - cb4_v8.x;
	r1_y = ViewDotLight * r1_y + cb4_v8.x;
	
	// line 37
	r1_y = r1_y - r0_y;

	float r1_z = saturate( g_lightDir.y * 4 - 1 );
	r0_y = r1_z * r1_y + r0_y;

	r1_y = cb4_v11.x - cb4_v10.x;
	r1_y = NormalDotInvSunDir  * r1_y + cb4_v10.x;

	r0_y = r0_y * r1_y;
	r0_y = pow(r0_y, 2.2);

	float2 xzSunLight = normalize( g_lightDir.xz );

	// line 49
	float ViewDotLightXZ = dot( xzSunLight, cameraToWorld.xz );

	// line 50
	float3 cb12_v233 = float3(1.374, 1.84, 2.07);
	float3 cb12_v266 = float3(1.6, 0.71, 1.09);

	// r2.xyz
	float3 atmoColor = cb12_v233 - cb12_v266;
	atmoColor = NormalDotInvSunDir * atmoColor + cb12_v266;

	float r0_x = cameraToWorldDistance * 0.00667;
	r0_x = min(r0_x, 1);

	// line 54
	float3 cb12_v267 = float3(1.0832, 1.0832, 1.0832) * 0.2;
	float3 costam = float3(1,1,1);
	float3 atmoColor2 = cb12_v267 - costam;
	atmoColor2 = r0_x * atmoColor2 + costam;
	atmoColor2 = atmoColor2 * atmoColor - atmoColor;

	// line 57, r1.yzw
	float3 atmoColor3 = ViewDotLightXZ * atmoColor2 + atmoColor;
	atmoColor3 *= r0_y;
	

	// line 59
	float3 cb12_v237 = float3(1.4582, 0.865, 0.27);
	atmoColor = -cb12_v237 * atmoColor3 + float3(1, 1.13, 0.55);
	

	// line 60
	atmoColor3 = atmoColor3 * cb12_v237;
	
	atmoColor *= ( clamp(0,1, 1-Input.HeightRatio));

	// Line 62!
	r0_x = cameraToWorldDistance - 0;	// -cb4_v0.x;
	r0_y = cameraToWorldDistance - 0;	// -cb12_v0.x;
	float r0_z = 10;
	r0_x = saturate( r0_x / r0_z );

	// line 66
	float3 foo = r0_x * atmoColor + atmoColor3;
	foo *= 0.61195; // cb2_v2
	

	//float Depth = TexDepth.SampleLevel( samplerPointClamp, Input.TextureUV, 0 ).r;
	float Depth = TexDepth.Load( uint3(Input.PositionH.xy, 0) ).r;
	Depth = Depth * g_depthScaleFactors.x + g_depthScaleFactors.y;
	Depth = Depth * g_cameraNearFar.x + g_cameraNearFar.y;
	Depth = max( Depth, 0.0001 );
	

	float FarPlane = 1.0 / Depth;
	FarPlane -= SkyboxFarPlane;

	FarPlane = saturate( FarPlane * 0.0001 );	// cb4_v18.x
	
	float factor = FarPlane * colorClouds.a;

	
	return float4(foo, factor);

	// line 77
	r0_z = 1;		//cb4_v13.x - cb4_v12.x;
	r0_y = saturate( r0_y / r0_z );


	// line 79, perlin?
	float2 perlinUV = g_fTime * 50 * float2(-0.002, 0.004);
	perlinUV = Input.TextureUV.xy  * float2(1, 0.75) + perlinUV;

	// r0.z
	float perlin = TexPerlin.SampleLevel( samplerAnisoWrap, perlinUV, 0 ).x;

	float r0_w = 1.60 + 0.5;		// cb4_15.x - cb4_14.x;
	perlin = saturate( perlin * r0_w - 0.5 );

	r0_w = Input.TextureUV.x - 1;
	r0_w = saturate( 1 * 2 + r0_w);

	perlin *= r0_w;

	r0_w = r0_y * perlin - 1.0;
	r0_y = r0_y * perlin - r0_w;

	// line 89
	factor = factor * r0_y + r0_w;

	r0_y = 1;
	r0_y = ViewDotLight * r0_y + 1;

	factor = saturate( r0_y * factor );
	factor = factor * 1;

	float4 finalColor = 0;
	finalColor.rgb = foo;
	finalColor.a = factor;

	return finalColor;
}

// The Witcher 3 Clouds - private research
float4 PS_CloudsTW3( in VS_OUTPUT Input ) : SV_Target
{
	/* USER DATA FROM CBUFFER */
	float3 cb4_v4 = float3(0.25, 1.0, 0.25);


	// Initial UV tuning
	float2 uv = float2(175.23, 175.23) * float2(0.002, 0.0);
	uv = Input.TextureUV.xy * 2 + uv;

	// RGB - normal map
	// A - cloud shape
	float4 colorData = TexClouds.Sample( samplerAnisoWrap, uv );
	
	float3 normal = colorData.rgb - 0.5;
	normal = normal * cb4_v4;
	normal = normalize( normal * 2 );

	// Bump mapping "manually"
	float3x3 TBNMatrix = float3x3(Input.Tangent, Input.Bitangent, Input.Normal);
	normal = mul( normal, TBNMatrix );


}




#endif