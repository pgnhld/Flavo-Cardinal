#ifndef __BLOOM_BLUR_HLSL__
#define __BLOOM_BLUR_HLSL__

#include "Common.hlsl"
#include "PostProcessCommon.hlsl"

Texture2D texInput : register (t0);

// Calculates the gaussian blur weight for a given distance and sigmas
float CalcGaussianWeight( int sampleDist, float sigma )
{
	float twoSigma2 = 2 * sigma * sigma;
	float g = 1.0f / sqrt( PI * twoSigma2 );
	return (g * exp( -(sampleDist * sampleDist) / (twoSigma2) ));
}

// Performs a gaussian blur in one direction
float3 Blur( in VS_OUTPUT_POSTFX input, float2 texScale, float sigma )
{
	float2 inputSize = float2( floor(g_Viewport.x * 0.50), floor(g_Viewport.y * 0.50) );

	float3 color = float3(0, 0, 0);

	[unroll] for (int i = -4; i < 4; i++)
	{
		float weight = CalcGaussianWeight( i, sigma );
		//float weight = GaussianWeights[i + 4];
		float2 texCoord = input.TextureUV;
		texCoord += (i / inputSize) * texScale;
		float4 sample = texInput.Sample( samplerPointClamp, texCoord );
		color += sample * weight;
	}

	return color;
}


// Horizontal gaussian blur
float3 BloomBlurH( in VS_OUTPUT_POSTFX input ) : SV_Target
{
	return Blur( input, float2(1, 0), bloomBlurSigma );
}

// Vertical gaussian blur
float3 BloomBlurV( in VS_OUTPUT_POSTFX input ) : SV_Target
{
	return Blur( input, float2(0, 1), bloomBlurSigma );
}



#endif