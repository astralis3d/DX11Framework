#ifndef __MOTIONBLUR_H__
#define __MOTIONBLUR_H__

#include "../Common/Common.hlsl"
#include "../Common/PostProcessCommon.hlsl"

Texture2D	TexLightBuffer          : register (t0);	   
Texture2D	TexVelocityBuffer       : register (t1);

float4 MotionBlurPS(in VS_OUTPUT_POSTFX Input) : SV_TARGET
{
     // *** Pixel Motion blur ***    
    float2 Velocity = TexVelocityBuffer.Load(int3(Input.Position.xy, 0)).xy;

    float velocityScale = 16.667;
    velocityScale /= g_fTimeDelta;
    velocityScale = min(velocityScale, 2000);
    
    Velocity *= velocityScale;  

    float speed = length(Velocity / g_Viewport.zw);
    uint nSamples = (uint) speed;
    nSamples = max(nSamples, 1);
    nSamples = min(nSamples, 64);

    float fSamples = (float) nSamples;
    
	float fSamplesMinusOne = fSamples - 1.0;

    float4 Diffuse = TexLightBuffer.Load(int3(Input.Position.xy, 0));

	float2 screenTexCoords =( Input.Position.xy + float2(0.5,0.5) ) * g_Viewport.zw;
 
    for (uint i = 1; i < nSamples; ++i)
    {
        float2 offset = Velocity * ( (float(i) / fSamplesMinusOne) - 0.5 );
        offset += Input.Position.xy;

        int3 iPos = int3((int2) offset, 0);

        Diffuse += TexLightBuffer.Load(iPos);
		//Diffuse += TexLightBuffer.SampleLevel( samplerLinearClamp, screenTexCoords + offset, 0);
    }

    Diffuse /= fSamples;

    return Diffuse;
}
#endif