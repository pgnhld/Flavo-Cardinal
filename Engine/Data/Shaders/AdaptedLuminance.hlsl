#ifndef __ADAPTEDLUMINANCE_HLSL__
#define __ADAPTEDLUMINANCE_HLSL__

#include "Common.hlsl"
#include "PostProcessCommon.hlsl"

Texture2D texCurrentLuminance	: register (t0);
Texture2D texLastLuminance		: register (t1);

float PSAdaptedLuminance( VS_OUTPUT_POSTFX Input ) : SV_Target0
{
	// Get current luminance
	float currentLuminance = texCurrentLuminance.Load( uint3(0, 0, 8) ).x;
	currentLuminance = exp(currentLuminance);

	// Get last luminance
	float lastLuminance = texLastLuminance.SampleLevel( samplerPointClamp, float2(0, 0), 0 ).x;

	// Calculate adapted luminance
	float luminanceDifference = currentLuminance - lastLuminance;
	float adaptedLum = 0.05 * luminanceDifference + lastLuminance;

	return adaptedLum;
}



#endif