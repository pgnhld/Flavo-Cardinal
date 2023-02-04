#ifndef __COMMON_HLSL__
#define __COMMON_HLSL__

struct SRenderCylinderLight
{
	float3 posStart;
	float intensity;

	float3 posEnd;
	float attenuation;

	float3 color;
	float radius;
};

// common constant buffers
cbuffer cbPerObject : register(b0)
{
	row_major matrix g_mtxWorld;

	// material
	float2 uvScale;
	float2 uvOffset;

	float3 colorTint;
	float specialEffect;

	float smoothness;
	float3 padding;
}

cbuffer cbPerView : register(b1)
{
	row_major matrix g_mtxView;
	row_major matrix g_mtxProjection;
	row_major matrix g_mtxViewProjection;
	row_major matrix g_mtxInvViewportProjAware;
	row_major matrix g_mtxViewProjectionShadow;
	float4 g_cameraPos;

	// splitscreen
	float4 g_cameraPos1;
	float4 g_cameraPos2;
}

cbuffer cbSkinning : register (b2)
{
	row_major matrix BoneMatrix[64];
}

cbuffer cbPerFrame : register (b3)
{
	float4 g_Viewport;
	float g_elapsedTime;
	float3 perframePad;
}

cbuffer cbLightingTemp : register (b5)
{
	uint g_numCylinderLights;
	float3 g_mainLightDir;

	float3 g_mainLightColor;
	float g_mainLightIntensity;

	SRenderCylinderLight g_cylinderLights[16];
}

cbuffer cbPostprocess : register (b13)
{
	float4 tonemappingCurveABCD;

	float2 tonemappingCurveEF;
	float tonemappingNominatorMultiplier;
	float tonemappingWhitePointScale;

	float tonemappingExposureExponent;
	float eyeAdaptationMinAllowedLuminance;
	float eyeAdaptationMaxAllowedLuminance;
	float bloomBlurSigma;

	float3 vignetteColor;
	float vignetteIntensity;

	float3 vignetteWeights;
	float chromaticAberrationIntensity;

	float2 chromaticAberrationCenter;
	float chromaticAberrationSize;
	float chromaticAberrationRange;

	float chromaticAberrationStart;
	float bloomThreshold;
	float bloomMultiplier;
	float xpadding_postprocess;
}

// Samplers
SamplerState samplerLinearWrap			: register (s0);
SamplerState samplerLinearClamp			: register (s1);
SamplerState samplerPointWrap			: register (s2);
SamplerState samplerPointClamp			: register (s3);
SamplerState samplerAnisoWrap			: register (s4);
SamplerState samplerAnisoClamp			: register (s5);
SamplerComparisonState samplerComparisonLinear	: register (s6);

// Common math consts
static const float PI = 3.14159265;
static const float3 LUMINANCE_RGB = float3(0.2126, 0.7152, 0.0722);
static const float ONE_BY_255 = 1.0 / 255.0;

// Common textures
TextureCube g_texSkybox	: register (t13);

// For Crytek's "Best Fit Normals"
void CompressUnsignedNormalToNormalsBuffer( in Texture2D texNormalsFit, inout float3 vNormal )
{
	float3 vNormalUns = abs( vNormal );

	float maxAbsNormalComponent = max( max( vNormalUns.x, vNormalUns.y ), vNormalUns.z );

	float2 vTexCoord = abs( vNormalUns.y ) < maxAbsNormalComponent ? (abs( vNormalUns.z ) < maxAbsNormalComponent ? abs( vNormalUns.zy ) : abs( vNormalUns.xy )) : abs( vNormalUns.xz );
	vTexCoord = vTexCoord.x < vTexCoord.y ? (vTexCoord.yx) : (vTexCoord.xy);
	vTexCoord.y /= vTexCoord.x;

	// fit normal into the edge of unit cube
	vNormal /= maxAbsNormalComponent;

	// scale the normal to get the best fit
	float fFittingScale = texNormalsFit.SampleLevel( samplerPointClamp, vTexCoord, 0 ).r;
	vNormal *= fFittingScale;
}

// Gamma & Linear Color Spaces
static const float Gamma = 2.2f;
static const float InverseGamma = 1.0f / 2.2f;

float3 GammaToLinear( in float3 clr )
{
	return pow( max( clr, float3(0, 0, 0) ), Gamma );
}

float3 LinearToGamma( in float3 clr )
{
	return pow( max( clr, float3(0, 0, 0) ), InverseGamma );
}

float3 normal_decode( in float3 enc )
{
	return normalize( enc * 2.0 - 1.0 );
}

float3 normal_encode_xyz( in float3 n )
{
	return n * 0.5 + 0.5;
}

float getRGB( in float x )
{
	return x * ONE_BY_255;
}

float3 getRGB( in float3 x )
{
	return x * ONE_BY_255;
}

float3 getRGB( in float r, in float g, in float b)
{
	return float3(r, g, b) * ONE_BY_255.xxx;
}

float3 projection_point_segment(in float3 startPos, in float3 endPos, in float3 pointPos) 
{
	float3 segment = endPos - startPos;
	const float lengthSquared = segment.x * segment.x + segment.y * segment.y + segment.z * segment.z;
	if (lengthSquared == 0.0) return startPos;

	const float t = max(0, min(1, dot(pointPos - startPos, segment) / lengthSquared));
	const float3 projection = startPos + t * (segment);
	return projection;
}

float Pow5(in float x)
{
	float x2 = x*x;

	return x2*x2*x;
}



float2 texOffset( float u, float v )
{
	static const float RCP_SHADOWMAP = 1.0 / 4096.0;
	return float2(u * RCP_SHADOWMAP, v * RCP_SHADOWMAP);
}

float CalculateShadow( in Texture2D shadowMap, in float3 WorldPos )
{
	float4 lightSpacePos = mul( float4(WorldPos, 1.0), g_mtxViewProjectionShadow );
	lightSpacePos.xyz /= lightSpacePos.w;

	// Transform clip space coords to texture space coords :
	// [-1;1] -> [0;1]
	lightSpacePos.xy = lightSpacePos.xy * float2(0.5, -0.5) + float2(0.5, 0.5);

	// apply bias
	//lightSpacePos.z -= 0.0035;

	// PCF filtering
	float sum = 0.0;

	[unroll] for (int x= -2; x < 3; ++x)
	{
		[unroll] for (int y= -2; y < 3; ++y)
		{
			sum += shadowMap.SampleCmpLevelZero( samplerComparisonLinear, lightSpacePos.xy + texOffset( x, y ), lightSpacePos.z );
		}
	}

	float shadowFactor = sum / 25.0;

	return shadowFactor;
}






// Useful function to linearize depth
// TODO: Near/Far
/*
float LinearizeDepth( in float depth )
{
	float fNear = g_zNear;
	float fFar = g_zFar;

	return (2.0f * fNear) / (fFar + fNear - depth * (fFar - fNear));

}
*/



#endif