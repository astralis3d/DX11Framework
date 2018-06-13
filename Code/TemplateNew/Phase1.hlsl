#ifndef __PHASE1_HLSL__
#define __PHASE1_HLSL__

#include "..//Common//Common.hlsl"
#include "..//Common//PostProcessCommon.hlsl"

groupshared uint sh_Data[256];

Texture2D TexColor : register (t0);
Texture2D TexDepth : register (t1);
RWStructuredBuffer<uint> g_data : register (u0);

cbuffer cbAvgLuminance : register (b5)
{
	float2 quarterSize;
	float2 params;

	float4 viewportFull;
	float4 params2;
}

[numthreads(64, 1, 1)]
void TW3_Phase1( int3 threadIndex : SV_GroupThreadID, int3 groupIndex : SV_GroupID  )
{
	int thid = threadIndex.x;
	int3 offseted = int3(thid, thid, thid) + int3(64, 128, 192);

	sh_Data[thid] = 0;
	sh_Data[offseted.x] = 0;
	sh_Data[offseted.y] = 0;
	sh_Data[offseted.z] = 0;

	GroupMemoryBarrierWithGroupSync();

	int x = (int)params2.z;

	[loop] for (uint i=0; ; i += 64 )
	{
		[flatten] if ( (float) i >= viewportFull.x)
			break;

		// line 16
		uint thid2 = i + threadIndex.x;
		float fThid2 = (float) thid2;
		[branch] if (fThid2 < viewportFull.x)
		{
			int2 colorLocation = int2(thid2, groupIndex.y);
			float3 color = TexColor.Load( int3(colorLocation, 0) );
			float luma = dot(color, LUMINANCE_RGB);
			
			int2 depthLocation = x * colorLocation;
			float depth = TexDepth.Load( int3(depthLocation, 0) );
			
			int result = (depth == params2.z);
			int result2 = (result & asint(params2.y));
			
			// It's often to see params2 = (0.0, 0.0, 4.0, 0.0)
			//result = result & asint(params2.y);
			//float luma2 = lerp(luma, params2.x, result2);
			float luma2 = luma;

			luma2 += 1.0;
			luma2 = log(luma2);
			luma2 *= 128;

			uint uLuma = (uint) luma2;
			uLuma = min(uLuma, 255);

			InterlockedAdd( sh_Data[uLuma], 1 );		// atomic_iadd
		}
	}

	GroupMemoryBarrierWithGroupSync();

	InterlockedAdd( g_data[threadIndex.x],	sh_Data[threadIndex.x] );
	InterlockedAdd( g_data[offseted.x],		sh_Data[offseted.x] );
	InterlockedAdd( g_data[offseted.y],		sh_Data[offseted.y] );
	InterlockedAdd( g_data[offseted.z],		sh_Data[offseted.z] );

}


#endif