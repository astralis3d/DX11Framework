// Common shader file

#ifndef __INCLUDE_COMMON__
#define __INCLUDE_COMMON__


//------------------------------------------------------------------------
// Samplers
//------------------------------------------------------------------------
SamplerState		  	samplerLinearWrap			: register(s0);
SamplerState		  	samplerLinearClamp			: register(s1);
SamplerState		  	samplerPointWrap			: register(s2);
SamplerState		  	samplerPointClamp			: register(s3);
SamplerState		  	samplerAnisoWrap			: register(s4);
SamplerState		  	samplerAnisoClamp			: register(s5);
SamplerComparisonState	samplerLinearComparison		: register(s6);


//------------------------------------------------------------------------
// Constant Buffers
//------------------------------------------------------------------------

// * per frame
cbuffer cbPerFrameGlobal : register (b0)
{
	float				g_fTime;
	float				g_fTimeDelta;
	float2				pad0000;
}

// * per camera view
cbuffer cbPerView : register (b1)
{
	row_major matrix	g_mtxView;
	row_major matrix	g_mtxProj;
	row_major matrix	g_mtxProjInv;
	row_major matrix	g_mtxViewProj;
	row_major matrix	g_mtxInvViewProj;
	row_major matrix	g_mtxInvViewProjViewport;
	row_major matrix	g_mtxViewProjPrevFrame;
	float4				g_CameraPos;
	float4				g_Viewport;			// Width, Height, 1.f/Width, 1.f/Height
	float				g_zNear;
	float				g_zFar;

	// Near/Far factors
	float2				g_cameraNearFar;

	// For inverted depth: (-1, 0), for default depth: (1, 0)
	float2				g_depthScaleFactors;

	float2				pad000001;
}

cbuffer cbPerObjectStatic : register (b2)
{
	row_major matrix	g_mtxWorld;
}

cbuffer SceneCB			: register (b3)
{
	row_major matrix	g_mViewProjShadow;
	float3				g_lightDir;
	float				pad0002;
}

// * for post processing
cbuffer cbPostProcessing : register (b4)
{
	float4				g_TW3VignetteWeights;
	float4				g_TW3VignetteColor;
	float				g_TW3VignetteOpacity;

	float3 padddd;
}

// Some constants
static const float3 LUMINANCE_RGB = float3(0.2126, 0.7152, 0.0722);

// Common math consts
static const float PI = 3.14159265;
static const float PI_INVERSE = 0.318309886;		// 1.0 / PI


float Square(in float x)
{
	return x*x;
}

float2 Square(in float2 x)
{
	return x*x;
}

float3 Square(in float3 x)
{
	return x*x;
}

float4 Square(in float4 x)
{
	return x*x;
}

float Pow5(in float x)
{
	float x2 = x * x;

	return x2 * x2 * x;
}

float3 RGBToYCoCg(float3 RGB)
{
	float Y =  dot(RGB, float3(1, 2, 1)) * 0.25;
	float Co = dot(RGB, float3(2, 0, -2)) * 0.25 + (0.5 * 256.0 / 255.0);
	float Cg = dot(RGB, float3(-1, 2, -1)) * 0.25 + (0.5 * 256.0 / 255.0);

	float3 YCoCg = float3(Y, Co, Cg);
	return YCoCg;
}

float3 YCoCgToRGB(float3 YCoCg)
{
	float Y = YCoCg.x;
	float Co = YCoCg.y - (0.5 * 256.0 / 255.0);
	float Cg = YCoCg.z - (0.5 * 256.0 / 255.0);

	float R = Y + Co - Cg;
	float G = Y + Cg;
	float B = Y - Co - Cg;

	return float3(R, G, B);
}


// Crytek's BFN
void CompressUnsignedNormalToNormalsBuffer( in Texture2D texNormalsFit, inout float3 vNormal )
{
	float3 vNormalUns = abs( vNormal );

	float maxAbsNormalComponent = max( max( vNormalUns.x, vNormalUns.y ), vNormalUns.z );

	float2 vTexCoord = abs( vNormalUns.y ) < maxAbsNormalComponent ? (abs( vNormalUns.z ) < maxAbsNormalComponent ? abs( vNormalUns.zy ) : abs( vNormalUns.xy )) : abs( vNormalUns.xz );
	vTexCoord = vTexCoord.x < vTexCoord.y ? (vTexCoord.yx) : (vTexCoord.xy);
	vTexCoord.y /= vTexCoord.x;

	// fit normal into the edge of unit cube
	vNormal /= maxAbsNormalComponent;

	// scale the normal to get the best fit
	float fFittingScale = texNormalsFit.SampleLevel( samplerPointClamp, vTexCoord, 0 ).r;
	vNormal *= fFittingScale;
}


// Gamma & Linear Color Spaces
static const float Gamma = 2.2f;
static const float InverseGamma = 1.0f / 2.2f;

float3 GammaToLinear(in float3 clr)
{
	return pow( max(clr, float3(0, 0, 0)), Gamma);
}

float3 LinearToGamma(in float3 clr)
{
	return pow( max(clr, float3(0, 0, 0)), InverseGamma);
}

// Useful function to linearize depth
float LinearizeDepth(in float depth)
{
    float fNear = g_zNear;
    float fFar = g_zFar;

   return (2.0f * fNear) / (fFar + fNear - depth * (fFar - fNear));

}

float LinearizeDepthReversed( in float depth )
{
    float fFar = g_zNear;
    float fNear = g_zFar;

    float dx = ddx(depth);
    float dy = ddy(depth);

    return 1000.0 * abs(dy);

    return (2.0f * fNear) / (fFar + fNear - depth * (fFar - fNear));
}


#endif // __INCLUDE_COMMON__