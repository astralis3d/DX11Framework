#ifndef __DRUNK_HLSL__
#define __DRUNK_HLSL__

#include "../Common/Common.hlsl"
#include "../Common/PostProcessCommon.hlsl"

static const float2 drunkOffsets[8] =
{
	float2(1.0, 0.0),
	float2(-1.0, 0.0),
	float2(0.707, 0.707),
	float2(-0.707, -0.707),
	float2(0.0, 1.0),
	float2(0.0, -1.0),
	float2(-0.707, 0.707),
	float2(0.707, -0.707)
};

// vodka shader
float4 vodka( float2 uv )
{
	float4 cb0_v0 = float4(119, 49, 86119.70, 0.99);
	float4 cb0_v1 = g_Viewport;
	float4 cb3_v0 = float4(1, 1, g_Viewport.zw);
	float4 cb3_v1;
	sincos( g_fTime * 200, cb3_v1.x, cb3_v1.y );

	//return float4(cb3_v1.x, cb3_v1.y, 0, 0);

	// Writing from scratch.
	float fAmount = cb3_v0.y * (-0.1) + 1.0;

	float2 offsetsSmall = cb3_v1.xy * float2(0.05, 0.05);

	// distance from center, r1.z
	float2 centerToTexel = uv - float2(0.5, 0.5);
	float distanceFromCenter = length( centerToTexel );

	float fScaledAmount = min( distanceFromCenter * 10, 1.0 );	// not higher than 1.0
	fScaledAmount *= cb3_v0.y;

	// next, we scale offsets by distance from center. Makes perfect sense :)
	float2 offsets = offsetsSmall * distanceFromCenter;
	offsets = centerToTexel * fAmount - offsets;
	offsets += float2(0.5, 0.5);

	// line 10
	float2 r3_xy = fAmount * centerToTexel;
	r3_xy = offsetsSmall * distanceFromCenter + r3_xy;
	r3_xy += float2(0.5, 0.5);


	// line 14
	float fScaledAmount2 = fScaledAmount * cb3_v0.x;
	fScaledAmount2 *= 5.0;

	// r4.xy.
	float2 xOffsets = fScaledAmount2 * g_Viewport.zw;

	float4 total = 0;
	float4 total2 = 0;
	int i = 0;

	[unroll] for (i = 0; i < 8; i++)
	{
		total += TexColorBuffer.Sample( samplerLinearClamp, xOffsets * drunkOffsets[i] + offsets.xy );
	}
	total /= 16.0;

	[unroll] for (i = 0; i < 8; i++)
	{
		total2 += TexColorBuffer.Sample( samplerLinearClamp, xOffsets * drunkOffsets[i] + r3_xy.xy );
	}
	total2 /= 16.0;



	float4 final = total + total2;

	// some other stuff.

	// 57
	float fAmountY = cb3_v0.y * 8.0;

	// 58
	float2 offsets2 = fAmountY * g_Viewport.zw; // r0.xy

												// 59
	float fExtraAmount = cb3_v1.y * 0.02 + 1.0;

	// 60
	float2 r1_zw = fExtraAmount * centerToTexel;

	// 61
	centerToTexel = centerToTexel * fExtraAmount + float2(0.5, 0.5);

	// 62
	r3_xy = r1_zw * offsets2 + centerToTexel;

	float2 r0_xy = offsets2 * r1_zw;
	r0_xy = r0_xy * 2.0 + centerToTexel;


	float4 a = TexColorBuffer.Sample( samplerLinearClamp, centerToTexel );
	float4 b = TexColorBuffer.Sample( samplerLinearClamp, r0_xy );
	float4 c = TexColorBuffer.Sample( samplerLinearClamp, r3_xy );

	float4 abc = a + b + c;
	abc /= 3.0;


	float4 final_r0 = fScaledAmount * (final - abc);
	float4 finalColor = cb3_v0.y * final_r0 + abc;

	return finalColor;
}




#endif