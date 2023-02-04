#ifndef __BLOOM_THRESHOLD_HLSL__
#define __BLOOM_THRESHOLD_HLSL__

#include "Common.hlsl"
#include "PostProcessCommon.hlsl"

Texture2D texInput : register (t0);

static const float  MIDDLE_GRAY = 0.72f;
static const float  LUM_WHITE = 1.5f;

float3 PSBloomThreshold( VS_OUTPUT_POSTFX Input ) : SV_Target0
{
	float3 color = texInput.SampleLevel( samplerPointClamp, Input.TextureUV, 0 ).rgb;

	color -= bloomThreshold;
	color = max( color, 0 );

	/*
	color *= MIDDLE_GRAY / (0.03 + 0.001f);
	color *= (1.0f + color / LUM_WHITE);
	color /= (1.0f + color);
	*/

	return color;
}

#endif