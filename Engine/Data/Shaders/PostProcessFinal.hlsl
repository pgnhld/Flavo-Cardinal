// Final post process - chromatic aberration, vignette and gamma correction
#ifndef __POSTPROCESSFINAL_HLSL__
#define __POSTPROCESSFINAL_HLSL__

#include "Common.hlsl"
#include "PostProcessCommon.hlsl"

Texture2D texInput	: register (t0);



/* Chromatic Aberration */
void ChromaticAberration( in Texture2D TexColorBuffer, in float2 uv, in float fChromaticAbberationIntensity, inout float3 color )
{
	// User-defined params
	float chromaticAberrationCenterAvoidanceDistance = chromaticAberrationStart;	// chromatic aberration start
	float fA = chromaticAberrationRange;											// chromatic aberration range

	// Calculate vector
	float2 chromaticAberrationOffset = uv - chromaticAberrationCenter;
	chromaticAberrationOffset = chromaticAberrationOffset / chromaticAberrationCenter;

	float chromaticAberrationOffsetLength = length( chromaticAberrationOffset );

	// To avoid applying chromatic aberration in center, subtract small value from
	// just calculated length.
	float chromaticAberrationOffsetLengthFixed = chromaticAberrationOffsetLength - chromaticAberrationCenterAvoidanceDistance;
	float chromaticAberrationTexel = saturate( chromaticAberrationOffsetLengthFixed * fA );

	float fApplyChromaticAberration = (0.0 < chromaticAberrationTexel);
	if (fApplyChromaticAberration)
	{
		chromaticAberrationTexel *= chromaticAberrationTexel;
		chromaticAberrationTexel *= chromaticAberrationSize	;	// chromatic aberration size

		chromaticAberrationOffsetLength = max( chromaticAberrationOffsetLength, 1e-4 );

		float fMultiplier = chromaticAberrationTexel / chromaticAberrationOffsetLength;

		chromaticAberrationOffset *= fMultiplier;
		chromaticAberrationOffset *= g_Viewport.zw;
		chromaticAberrationOffset *= fChromaticAbberationIntensity;

		float2 offsetUV = -chromaticAberrationOffset * 2 + uv;
		color.r = TexColorBuffer.SampleLevel( samplerLinearClamp, offsetUV, 0 ).r;

		offsetUV = uv - chromaticAberrationOffset;
		color.g = TexColorBuffer.SampleLevel( samplerLinearClamp, offsetUV, 0 ).g;
	}
}

// Vignette
float VignetteMask( in float2 uv )
{
	float distanceFromCenter = length( uv - float2(0.5, 0.5) );

	float x = distanceFromCenter * 2.0 - 0.55;
	x = saturate( x * 1.219512 ); // r2.w			// 1.219512 = 100/82

	float x2 = x * x;
	float x3 = x2 * x;
	float x4 = x2 * x2;

	float outX = dot( float4(x4, x3, x2, x), float4(-0.10, -0.105, 1.12, 0.09) );
	outX = min( outX, 0.94 );

	return outX;
}

float3 CalculateVignette( in float3 gammaColor, in float3 vignetteColor, in float3 vignetteWeights, 
							  in float vignetteOpacity, in float2 textureUV )
{
	// For coloring vignette
	float3 vignetteColorGammaSpace = -gammaColor + vignetteColor;

	// Calculate vignette amount based on color in *linear* space and vignette weights
	float vignetteWeight = dot( GammaToLinear(gammaColor), vignetteWeights );

	// we need to keep vignette weight in [0-1] range
	vignetteWeight = saturate( 1.0 - vignetteWeight );

	// Multiply by opacity
	vignetteWeight *= vignetteOpacity;

	// Obtain vignette mask
	float extraMask = VignetteMask( textureUV );

	// Final (inversed) vignette mask
	float finalInvVignetteMask = saturate( vignetteWeight * extraMask );

	// Final composite in gamma space
	float3 color = vignetteColorGammaSpace * finalInvVignetteMask + gammaColor.rgb;
	return color;
}

float3 PSFinalPostProcess( VS_OUTPUT_POSTFX Input ) : SV_Target0
{
	float3 colorLinear = texInput.SampleLevel( samplerLinearClamp, Input.TextureUV, 0).rgb;

	// Apply chromatic aberration
	ChromaticAberration( texInput, Input.TextureUV, chromaticAberrationIntensity, colorLinear );


	float3 colorGamma = LinearToGamma( colorLinear );



	// ** Calculate vignette ***
	float3 colorWithVignetteGamma = CalculateVignette( colorGamma,
													   vignetteColor,
													   vignetteWeights,
													   vignetteIntensity,
													   Input.TextureUV );

	return colorWithVignetteGamma;
}

#endif