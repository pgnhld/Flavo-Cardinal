#ifndef __DEFERREDSHADINGCOMMON_HLSL__
#define __DEFERREDSHADINGCOMMON_HLSL__

#include "Common.hlsl"

struct GBufferData
{
	float3 Albedo;
	float Roughness;
	
	float3 WorldPos;
	float Metalness;
	float3 Normal;
	float AmbientOcclusion;

	float specialEffect;
	float TintIntensity;
};


#endif