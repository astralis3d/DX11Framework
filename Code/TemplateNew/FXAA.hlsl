#ifndef __FXAA_INCLUDED__
#define __FXAA_INCLUDED__

#include "..//Common//PostProcessCommon.hlsl"

// Input texture for FXAA
Texture2D TexInput0 : register(t0);

// Defines for FXAA
#define USE_FXAA				1

#define FXAA_PC					1
#define FXAA_HLSL_5				1
#define FXAA_QUALITY__PRESET	23

// FXAA shader source
#include "..//Common//FXAA3_11.hlsl"

float4 FxaaPS(VS_OUTPUT_POSTFX Input) : SV_TARGET
{
#if (USE_FXAA == 0)
	return TexInput0.Sample(samplerLinearClamp, Input.TextureUV);
#else

	FxaaTex InputFXAATex = { samplerAnisoClamp, TexInput0 };

	float4 FXAAColor =  FxaaPixelShader(
		Input.TextureUV.xy, // FxaaFloat2 pos,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f), // FxaaFloat4 fxaaConsolePosPos,
		InputFXAATex, // FxaaTex tex,
		InputFXAATex, // FxaaTex fxaaConsole360TexExpBiasNegOne,
		InputFXAATex, // FxaaTex fxaaConsole360TexExpBiasNegTwo,
		g_Viewport.zw, // FxaaFloat2 fxaaQualityRcpFrame,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f), // FxaaFloat4 fxaaConsoleRcpFrameOpt,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f), // FxaaFloat4 fxaaConsoleRcpFrameOpt2,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f), // FxaaFloat4 fxaaConsole360RcpFrameOpt2,
		0.75f, // FxaaFloat fxaaQualitySubpix,
		0.166f, // FxaaFloat fxaaQualityEdgeThreshold,
		0.0833f, // FxaaFloat fxaaQualityEdgeThresholdMin,
		0.0f, // FxaaFloat fxaaConsoleEdgeSharpness,
		0.0f, // FxaaFloat fxaaConsoleEdgeThreshold,
		0.0f, // FxaaFloat fxaaConsoleEdgeThresholdMin,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f) // FxaaFloat fxaaConsole360ConstDir,
		);

	return FXAAColor;

#endif
}


#endif // __FXAA_INCLUDED__