#ifndef __LENSFLARES_HLSL__
#define __LENSFLARES_HLSL__

#include "Common.hlsl"
#include "PostProcessCommon.hlsl"

Texture2D texInput : register (t0);

// User-defined params
static const int	g_nLensFlareNumGhosts = 8;
static const float	g_fLensFlareDistortion = 10;
static const float	g_fLensFlareDispersal = 0.37;
static const float	g_fLensFlareHaloWidth = 0.45;

// Helper function for distortion of lens flares
float3 LensFlareTextureDistorted( in Texture2D Tex, in float2 TextureUV, in float2 Direction, in float3 Distortion )
{
	return float3( Tex.Sample( samplerLinearClamp, TextureUV + Direction * Distortion.r ).r,
				   Tex.Sample( samplerLinearClamp, TextureUV + Direction * Distortion.g ).g,
				   Tex.Sample( samplerLinearClamp, TextureUV + Direction * Distortion.b ).b
				   );

}

float3 LensFlaresThresholdPS( VS_OUTPUT_POSTFX Input ) : SV_Target0
{
	float2 lensUV = -Input.TextureUV + float2(1.0, 1.0);
	float3 color = texInput.SampleLevel( samplerLinearClamp, lensUV, 0 ).rgb;

	color = color * 0.02;
	color = max( color, float3(0, 0, 0) );

	return color;
}

float3 LensFlaresPS( VS_OUTPUT_POSTFX Input ) : SV_Target0
{
	static const float  RcpOfHalfVector2 = 1.0 / length( float2(0.5, 0.5) );



	float2 lensUV = -Input.TextureUV + float2(1.0, 1.0);
	float3 color = texInput.SampleLevel( samplerLinearClamp, lensUV, 0 ).rgb;

	float2 texelSize = g_Viewport.zw * 0.25;

	float3 vDistortion = float3( -texelSize.x * g_fLensFlareDistortion, 0.0, +texelSize.x * g_fLensFlareDistortion );

	// Ghost vector to the center
	float2 vGhost = float2(0.5, 0.5) - lensUV;
	vGhost *= g_fLensFlareDispersal;

	float2 vHalo = normalize( vGhost ) * g_fLensFlareHaloWidth;

	// Sample ghosts

	float3 result = 0;

	[unroll]
	for (int i=0; i < g_nLensFlareNumGhosts; i++)
	{
		float2 offset = frac( lensUV + vGhost * i );

		float weight = length( float2(0.5, 0.5) - offset) * RcpOfHalfVector2;
		weight = pow(1.0 - weight, 10);

		result += LensFlareTextureDistorted( texInput, offset, normalize(vGhost), vDistortion) * weight;
	}

	// Sample halo
	float halo_weight = length( float2(0.5, 0.5) - frac( lensUV + vHalo ) ) * RcpOfHalfVector2;
	halo_weight = pow( 1.0 - halo_weight, 10 );

	result.rgb += LensFlareTextureDistorted( texInput, frac( lensUV + vHalo ), normalize( vGhost ), vDistortion ) * halo_weight;


	return result;
}

#endif