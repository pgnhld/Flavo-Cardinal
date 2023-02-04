#ifndef __SIMPLECOPY_HLSL__
#define __SIMPLECOPY_HLSL__

#include "Common.hlsl"
#include "PostProcessCommon.hlsl"

Texture2D texInput : register (t0);

float4 PSSimpleCopy( VS_OUTPUT_POSTFX Input ) : SV_Target0
{
	float4 color = texInput.SampleLevel( samplerLinearWrap, Input.TextureUV, 0 );
	return color;
}

#endif