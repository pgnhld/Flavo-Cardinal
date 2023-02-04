#include "Common.hlsl"
#include "BRDF.hlsl"

Texture2D g_texAlbedo		: register (t0);
Texture2D g_texNormals		: register (t1);
Texture2D g_texShadowMap	: register (t6);

struct VS_OUTPUT
{
	float4 PositionH	: SV_POSITION;
	float3 PositionW	: POSITION;
	float3 Normal		: NORMAL;
	float3 Tangent		: TANGENT;
	float3 Bitangent	: BITANGENT;
	float2 TextureUV	: TEXCOORD0;
};

float4 getNoise( in float2 uv )
{
	float time = g_elapsedTime * 0.25;

	float2 uv0 = (uv / 103.0) + float2( time / 17.0, time / 29.0 );
	float2 uv1 = uv / 107.0 - float2( time / -19.0, time / 31.0 );
	float2 uv2 = uv / float2( 897.0, 983.0 ) + float2( time / 101.0, time / 97.0 );
	float2 uv3 = uv / float2( 991.0, 877.0 ) - float2( time / 109.0, time / -113.0 );

	float4 noise =  g_texNormals.Sample( samplerAnisoWrap, uv0 ) +
					g_texNormals.Sample( samplerAnisoWrap, uv1 ) +
					g_texNormals.Sample( samplerAnisoWrap, uv2 ) +
					g_texNormals.Sample( samplerAnisoWrap, uv3 );

	return noise * 0.5 - 1.0;
}

float4 WaterPS( VS_OUTPUT Input ) : SV_Target0
{
	// Setup texcoords
	float2 TextureUV = Input.TextureUV * uvScale + uvOffset;
	TextureUV += float2(0.0001, -0.0001) * g_elapsedTime;

	// Get bump map from noise
	float3 sampledNormal = getNoise( TextureUV * 100.0 ).rgb;
	
	// Calculate needed vectors
	float3 worldToCamera = normalize( g_cameraPos.xyz - Input.PositionW );
	float3 cameraToWorld = -worldToCamera;

	// calculate reflected color
	float3 normalX = float3(0.0, 1.0, 0.0);
	float3 coordReflect = normalize( reflect( cameraToWorld, normalX) );
	float3 colorReflect = g_texSkybox.Sample( samplerLinearWrap, coordReflect + 0.25*sampledNormal.rbg * float3(1,0,1) ).rgb;
	colorReflect = GammaToLinear(colorReflect);

	// Calculate refracted color
	float3 sampledNormalNormalized = normalize(sampledNormal.rbg);
	float NdotL = dot( float3(0, 1, 0), sampledNormalNormalized );
	NdotL = max(0.15, NdotL);

	float3 colorRefract = colorTint * NdotL;

	// Calculate Fresnel effect
	float fresnel =  saturate( dot( sampledNormalNormalized, worldToCamera ) );

	// Mix refaction and reflections by Fresnel factor
	float3 colorWater = lerp( colorReflect, colorRefract, fresnel );

	// Blinn-Phong specular (simulating the Moon)
	float3 H = normalize( worldToCamera - g_mainLightDir );
	float NdotH = max( 0.0, dot( sampledNormalNormalized, H ) );
	float specular = pow(NdotH, 256);

	colorWater += specular;

	// Shadows
	float shadow = CalculateShadow( g_texShadowMap, Input.PositionW );
	colorWater = lerp( colorWater * 0.3, colorWater, shadow );
	

	return float4(colorWater, 1);
}
