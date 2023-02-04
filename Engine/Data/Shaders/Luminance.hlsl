#ifndef __LUMINANCE_HLSL__
#define __LUMINANCE_HLSL__

#include "Common.hlsl"
#include "PostProcessCommon.hlsl"

Texture2D texInput : register (t0);

float PSLogLuminanceSplitscreenLeft( VS_OUTPUT_POSTFX Input ) : SV_Target0
{
	float2 TextureUV = Input.TextureUV;

	// scale + offset
	float scale = 0.5;
	float offset = 0.0;

	TextureUV.x = offset + TextureUV.x * scale;

	float2 position = TextureUV * g_Viewport.xy;
	float3 color = texInput.Load( uint3(position, 0) ).rgb;

	float luminance = dot(color, LUMINANCE_RGB);
	luminance = clamp( luminance, 1e-5, 100000.0 );
	luminance = log(luminance);

	return luminance;
}


float PSLogLuminanceSplitscreenRight( VS_OUTPUT_POSTFX Input ) : SV_Target0
{
	float2 TextureUV = Input.TextureUV;

	// scale + offset
	float scale = 0.5;
	float offset = 0.5;

	TextureUV.x = offset + TextureUV.x * scale;

	float2 position = TextureUV * g_Viewport.xy;
	float3 color = texInput.Load( uint3(position, 0) ).rgb;

	float luminance = dot( color, LUMINANCE_RGB );
	luminance = clamp( luminance, 1e-5, 100000.0 );
	luminance = log( luminance );

	return luminance;
}

float PSLogLuminanceFullscreen( VS_OUTPUT_POSTFX Input ) : SV_Target0
{
	float2 TextureUV = Input.TextureUV;

	float2 position = TextureUV * g_Viewport.xy;
	float3 color = texInput.Load( uint3(position, 0) ).rgb;

	float luminance = dot( color, LUMINANCE_RGB );
	luminance = clamp( luminance, 1e-5, 100000.0 );
	luminance = log( luminance );

	return luminance;
}


#endif