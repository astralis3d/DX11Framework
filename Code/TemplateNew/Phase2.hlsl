#ifndef __PHASE2_HLSL__
#define __PHASE2_HLSL__

#include "..//Common//Common.hlsl"
#include "..//Common//PostProcessCommon.hlsl"

groupshared uint sh_Data[256];

RWStructuredBuffer<uint> g_data : register (u0);
RWTexture2D<float>		TexOutput : register (u1);

cbuffer cbAvgLuminance : register (b5)
{
	float2 quarterSize;
	float2 params;

	float4 viewportFull;
	float4 params2;
}

[numthreads( 64, 1, 1 )]
void TW3_Phase2( int3 threadIndex : SV_GroupThreadID )
{
	int thid = threadIndex.x;
	int3 offseted = int3(thid, thid, thid) + int3(64, 128, 192);
	
	sh_Data[thid] = g_data[thid];
	sh_Data[offseted.x] = g_data[offseted.x];
	sh_Data[offseted.y] = g_data[offseted.y];
	sh_Data[offseted.z] = g_data[offseted.z];

	GroupMemoryBarrierWithGroupSync();

	[branch] 
	if ( threadIndex.x == 0 )
	{
		float fxTotalPixels = quarterSize.y * quarterSize.x;
		uint uTotalPixels = (uint) fxTotalPixels;
		float fTotalPixels = (float) uTotalPixels;

		float2 xy = fTotalPixels * params;
		int2 iXY = (int2) xy;

		iXY.x -= 1;
		iXY.y = max( iXY.y, 0 );
		iXY.y = min( iXY.x, iXY.y );

		int maxc = max( iXY.x, iXY.y );
		iXY.x = min( maxc, iXY.x );

		int r2_x = 0;
		int r2_y = 0;
		int r2_z = 0;

		int2 r1_xy = int2(0,0);
		int r1_z = -1;


		[loop] while (1)
		{
			[flatten] if (r2_x != 0)
				break;

			int r0_z = sh_Data[ r2_z ];
			int r3_x = r0_z + r2_y;

			r0_z = ( iXY.y < r3_x );

			int r3_y = r2_z + 1;

			r1_xy = int2(r2_y, r2_z);

			int r3_z = r2_x;

			r2_x = (r0_z) ? r1_z : r3_z;
			r2_y = (r0_z) ? r1_xy.x : r3_x;
			r2_z = (r0_z) ? r1_xy.y : r3_y;
		}

		float fCostamW = 0;

		[loop] while(1)
		{
			[flatten] if (r1_xy.x != 0)
				break;

			int loaded = sh_Data[ r2_z ];
			int sum  = r2_y + loaded;

			float fLoaded = (float) loaded;
			float fAddress = (float) r2_z;

			fAddress += 0.5;
			fAddress *= 0.011271;
			fAddress = exp(fAddress);
			fAddress -= 1.0;

			float r3_z = fLoaded * fAddress + fCostamW;

			bool result = (iXY.x < sum);

			//int sum2 = sum - 

		}
	}

}


#endif