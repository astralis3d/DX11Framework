/*
#permutation ENABLE_SHARPNESS_PROFILE 0 1
#permutation KERNEL_RADIUS 2 4
*/

/* 
* Copyright (c) 2008-2016, NVIDIA CORPORATION. All rights reserved. 
* 
* NVIDIA CORPORATION and its licensors retain all intellectual property 
* and proprietary rights in and to this software, related documentation 
* and any modifications thereto. Any use, reproduction, disclosure or 
* distribution of this software and related documentation without an express 
* license agreement from NVIDIA CORPORATION is strictly prohibited. 
*/

#include "ConstantBuffers.hlsl"
#include "FullScreenTriangle_VS.hlsl"
#include "Blur_Common.hlsl"

//----------------------------------------------------------------------------------
float2 BlurX_PS( PostProc_VSOut IN ) : SV_TARGET
{
    float CenterDepth;
    float AO = ComputeBlur(IN, float2(g_f2InvFullResolution.x,0), CenterDepth);

    return float2(AO, CenterDepth);
}
