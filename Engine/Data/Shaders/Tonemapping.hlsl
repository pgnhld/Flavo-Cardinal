#ifndef __TONEMAPPING_HLSL__
#define __TONEMAPPING_HLSL__

#include "Common.hlsl"
#include "PostProcessCommon.hlsl"

Texture2D texHDRcolor		: register (t0);
Texture2D texLuminance		: register (t1);
Texture2D texBloom			: register (t2);
Texture2D texLensFlares		: register (t3);

// http://filmicgames.com/archives/75
float3 U2Func( float A, float B, float C, float D, float E, float F, float3 x )
{
	return ((x*(A*x + C * B) + D * E) / (x*(A*x + B) + D * F)) - E / F;
}

float3 ToneMapU2Func( float A, float B, float C, float D, float E, float F, float3 color, float numMultiplier )
{
	float3 numerator = U2Func( A, B, C, D, E, F, color );
	numerator = max( numerator, 0 );
	numerator *= numMultiplier;

	float3 denominator = U2Func( A, B, C, D, E, F, 11.2 );
	denominator = max( denominator, 1e-5 );

	return numerator / denominator;
}

float3 PSTonemapping( VS_OUTPUT_POSTFX Input ) : SV_Target0
{
	/*
	float userParamCurveA = 0.98;
	float userParamCurveB = 1.02;
	float userParamCurveC = 1.00;
	float userParamCurveD = 3.00;
	float userParamCurveE = 0.025;
	float userParamCurveF = 0.9;

	float userParamWhitePointScale = 0.48;
	float userParamNumeratorMultiplier = 1.46;
	float userParamLuminanceExponent = 0.7;
	*/

	float userParamCurveA = tonemappingCurveABCD.x;
	float userParamCurveB = tonemappingCurveABCD.y;
	float userParamCurveC = tonemappingCurveABCD.z;
	float userParamCurveD = tonemappingCurveABCD.w;
	float userParamCurveE = tonemappingCurveEF.x;
	float userParamCurveF = tonemappingCurveEF.y;

	float userParamWhitePointScale = tonemappingWhitePointScale;
	float userParamNumeratorMultiplier = tonemappingNominatorMultiplier;
	float userParamLuminanceExponent = tonemappingExposureExponent;

	

	// Get current luminance after eye adaptation
	float avgLuminance = texLuminance.Load( uint3(0, 0, 0) ).x;
	avgLuminance = clamp(avgLuminance, eyeAdaptationMinAllowedLuminance, eyeAdaptationMaxAllowedLuminance);

	avgLuminance = max(avgLuminance, 1e-4);

	// Calculate exposure
	float scaledWhitePoint = userParamWhitePointScale * 11.2;

	float luma = avgLuminance / scaledWhitePoint;
	luma = pow(luma, userParamLuminanceExponent);

	luma = luma * scaledWhitePoint;

	// Exposure is known as middleGrey / avgLogLuminance
	float exposure = userParamWhitePointScale / luma;

	// Get HDR color
	float3 HDRcolor = texHDRcolor.Load( uint3(Input.Position.xy, 0) ).rgb;
	HDRcolor *= exposure;


	float2 uvs = (float2)(Input.Position.xy) / g_Viewport.xy;
	
	// Get bloom
	float3 bloom = texBloom.Sample( samplerLinearWrap, uvs ).rgb;
	HDRcolor += bloomMultiplier * bloom;

	// Get lens flares
	float3 lensFlares = texLensFlares.Sample( samplerLinearWrap, uvs ).rgb;
	//HDRcolor += lensFlares;

	// Calculate final bloom
	float3 tonemappedColor = ToneMapU2Func( userParamCurveA,
											userParamCurveB,
											userParamCurveC,
											userParamCurveD,
											userParamCurveE,
											userParamCurveF,
											HDRcolor,
											userParamNumeratorMultiplier );

	

	return tonemappedColor;
}



#endif