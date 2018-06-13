#include "../Common/Common.hlsl"
#include "../Common/PostProcessCommon.hlsl"
#include "../Common/Vignette.hlsl"
#include "../Common/ColorCorrection.hlsl"

//----------------------------------------------------------------------
Texture2D		Tex		            : register (t0);	            	// Color
Texture2D		Tex1	            : register (t1);	            	// Depth
Texture3D		TexLUT	            : register (t2);                    // Color correction
Texture2D       TexAdaptedLuminance : register (t3);                    // Adapted luminance   
Texture2D       TexVelocityBuffer   : register (t4);                    // Velocity buffer
Texture2D       TexBloom            : register (t5);

float3 Uncharted2Tonemap(float3 x)
{
	// http://filmicgames.com/archives/75  

    const float A = 0.25;
    const float B = 0.30;
    const float C = 0.10;
    const float D = 0.35;
    const float E = 0.025;
    const float F = 0.40;
    const float W = 11.2;

   return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F)) - E/F;
}

// Applies the Uncharted 2 filmic tone mapping curve
float3 ToneMapFilmicU2(float3 color)
{
    float3 numerator = Uncharted2Tonemap(color);
    numerator = max(numerator, 0);
    numerator *= 1.5;

    float3 denominator = Uncharted2Tonemap(11.2);
    denominator = max(denominator, 1e-5);
	
    return numerator / denominator;
}

// Applies the filmic curve from John Hable's presentation
float3 ToneMapFilmicALU(float3 color)
{
    color = max(0, color - 0.004f);
    color = (color * (6.2f * color + 0.5f)) / (color * (6.2f * color + 1.7f) + 0.06f);

    // result has 1/2.2 baked in
    return pow(color, 2.2f);
}

float3 ToneMapACESFilm(float3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

float3 ToneMappingTest(in float3 HDRColor)
{
	// Step 1: Load adapted luminance and clamp it to user-defined min/max values.
    float fAdaptedLum = TexAdaptedLuminance.Load( int3(0, 0, 0) ).x;

    const float fMinAllowedSceneLuminance = 0.1;
    const float fMaxAllowedSceneLuminance = 3;
    //fAdaptedLum = clamp( fAdaptedLum, fMinAllowedSceneLuminance, fMaxAllowedSceneLuminance );
    fAdaptedLum = max( fAdaptedLum, 1e-4 );

    //float fMiddleGray = 0.38;
    float fMiddleGray = 1.03 - 2.0 / (2.0 + log10(fAdaptedLum + 1));
       
    // Linear Exposure
    float fLumScale = fMiddleGray / fAdaptedLum;
    fLumScale = max( fLumScale, 1e-4 );
   

    float3 f3ExposedColor = fLumScale * HDRColor;
       
    // Step 2 : Apply curve
    //float3 f3TonemappedColor = ToneMapFilmicALU(f3ExposedColor);
    //float3 f3TonemappedColor = ToneMapACESFilm(f3ExposedColor);    
    float3 f3TonemappedColor = ToneMapFilmicU2(2.0f*f3ExposedColor);

    return f3TonemappedColor;
}

float3 ToneMapping_TW3(in float3 HDRColor)
{
	// Step 1: Load adapted luminance and clamp it to user-defined min/max values.
    float fAdaptedLum = TexAdaptedLuminance.Load(int3(0, 0, 0)).x;
		
    const float fMinAllowedSceneLuminance = 0.1;
    const float fMaxAllowedSceneLuminance = 3.5;
    fAdaptedLum = clamp(fAdaptedLum, fMinAllowedSceneLuminance, fMaxAllowedSceneLuminance);
    fAdaptedLum = max(fAdaptedLum, 1e-4);

	// Step 2: Middle gray, white point
    float fWhitePoint = 0.23516 * 11.2;

    float fLumScale = fAdaptedLum / fWhitePoint;
	fLumScale = pow( fLumScale, 0.5 );

    fLumScale = fLumScale * fWhitePoint;

    fLumScale = 0.23516 / fLumScale;

    float3 f3TonemappedColor = ToneMapFilmicU2(fLumScale * HDRColor);

    return f3TonemappedColor;
}


float4 QuadPS(in VS_OUTPUT_POSTFX Input) : SV_TARGET
{    
    float4 ColorBuffer = Tex.Sample(samplerLinearClamp, Input.TextureUV);

#ifdef MOTION_BLUR2
	float Depth = Tex1.Sample(samplerLinearClamp, Input.TextureUV).r;
	float zOverW = Depth.r;

	// Motion Blur
	// GPU Gems 3, chapter 27
	//float4 H = float4(Input.TextureUV.xy, zOverW, 1.f);
	float4 H = float4( Input.TextureUV.x * 2 - 1, (1.0 - Input.TextureUV.y) * 2 - 1, zOverW, 1);

	float4 D = mul(H, g_mtxInvViewProj);


	float4 worldPos = D / D.w;
	//return worldPos;

	float4 currentPos = H;
	float4 previousPos = mul(worldPos, g_mtxViewProjPrevFrame);
	previousPos /= previousPos.w;

	float2 velocity = (currentPos.xy - previousPos.xy) / 2.0;
	
    velocity *= x; // hard-coded velocity scale (curFPS / targetFPS)

	// Max velocity

	float2 texCoords = Input.TextureUV;
	
	for (int i = 1; i < 32; ++i, texCoords += velocity)
	{
		float2 vSampleCoord = Input.TextureUV + lerp(-velocity, +velocity, ((float)i / (float)32));
		//float2 vSampleCoord = texCoords;

		float3 currcolor = ( Tex.Sample(samplerLinearClamp, vSampleCoord).rgb );
		Diffuse.rgb += currcolor.rgb;
	}

	Diffuse.rgb /= (32);

#endif

    /* BLOOM */
    //float3 Bloom = TexBloom.Sample(samplerLinearClamp, Input.TextureUV).rgb;
    //ColorBuffer.rgb += 0.7 * Bloom;
  
    /* TONE MAPPING */
    ColorBuffer.rgb = ToneMappingTest(ColorBuffer.rgb);   
	//ColorBuffer.rgb = ToneMapping_TW3( ColorBuffer.rgb );
    
	/* COLOR CORRECTION */
    ColorBuffer.rgb = ColorCorrection(TexLUT, ColorBuffer.rgb);

    // Prepare for FXAA (after tone mapping!)
    ColorBuffer.a = dot(ColorBuffer.rgb, float3(0.299, 0.587, 0.114)); // compute luma 

    return ColorBuffer;
}