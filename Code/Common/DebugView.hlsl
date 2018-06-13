#include "..//Common//Common.hlsl"
#include "..//Common//PostProcessCommon.hlsl"

Texture2D<float4>	TexInput0	: register (t0);
Texture2D<uint2>	TexInput1	: register (t1);


float4 DebugViewPS(VS_OUTPUT_POSTFX Input) : SV_Target
{
	float4 color = TexInput0.Sample(samplerLinearClamp, Input.TextureUV);
	return color;
}

float4 DebugViewDepthPS(VS_OUTPUT_POSTFX Input) : SV_Target
{
	float Depth = InputTexture0[Input.Position.xy].x;
	Depth = Depth * g_depthScaleFactors.x + g_depthScaleFactors.y;

	float LinearizedDepth = LinearizeDepth( Depth );
	LinearizedDepth = LinearizedDepth * g_depthScaleFactors.x + g_depthScaleFactors.y;

	return LinearizedDepth.xxxx;
}


float4 DebugViewStencilPS(VS_OUTPUT_POSTFX Input) : SV_Target0
{
    int3 location = int3( Input.Position.xy, 0 );
	uint2 value = TexInput1.Load( location ) ;

	float fValue = (float) value.x / 255.0;

	return fValue.xxxx;
};