#ifndef __GBUFFER_HLSL__
#define __GBUFFER_HLSL__

#include "Common.hlsl"

#define USE_NORMAL_MAPPING 1

Texture2D g_texAlbedo	: register (t0);
Texture2D g_texNormals	: register (t1);
Texture2D g_texRoughness : register (t2);
Texture2D g_texMetalness : register (t3);

Texture2D g_texNormalsFit : register (t13);


// Input/Output Vertex
struct VS_INPUT
{
	float3 Position		: POSITION;
	float2 TextureUV	: TEXCOORD;
	float3 Normal		: NORMAL;
	float3 Tangent		: TANGENT;
	float3 Bitangent	: BITANGENT;
};

struct VS_INPUT_SKINNED
{
	float3 Position		: POSITION;
	float2 TextureUV	: TEXCOORD;
	float3 Normal		: NORMAL;
	float3 Tangent		: TANGENT;
	float3 Bitangent	: BITANGENT;
	uint4 BlendIndices	: BLENDINDICES;
	float4 BlendWeights : BLENDWEIGHTS;
};


struct VS_OUTPUT
{
	float4 PositionH	: SV_POSITION;
	float3 PositionW	: POSITION;
	float3 Normal		: NORMAL;
	float3 Tangent		: TANGENT;
	float3 Bitangent	: BITANGENT;
	float2 TextureUV	: TEXCOORD0;
};

struct PS_OUTPUT
{
	float4 RT0 : SV_Target0;		// RGB: Albedo, A - intensity multiplier
	float4 RT1 : SV_Target1;		// Normals
	float4 RT2 : SV_Target2;		// R - roughness, G - metallic, B- material id, A - unused
	float3 RT3 : SV_Target3;		// RGB - world position

};


// Vertex Shader for deferred rendering
VS_OUTPUT RenderSceneToGBufferVS( VS_INPUT Input )
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	// Transform the position from object space to homogeneous projection space
	float4 PositionW = mul( float4(Input.Position, 1.0), g_mtxWorld );

	Output.PositionW = PositionW.xyz;

	// SV_Position
	Output.PositionH = mul( PositionW, g_mtxViewProjection );

	// Normal & Tangent in world space
	Output.Normal = mul( Input.Normal, (float3x3) g_mtxWorld );
	//Output.Normal = Input.Normal;
	Output.Tangent = mul( Input.Tangent, (float3x3) g_mtxWorld );

	Output.Bitangent = normalize( cross( Output.Normal, Output.Tangent ) );



	// Just pass texture uv further
	Output.TextureUV = Input.TextureUV;

	return Output;
}

float4 CalcSkinnedWorldPosition( in VS_INPUT_SKINNED input )
{
	float4 posIn = float4(input.Position, 1.0);
	float4 posX = 0;

	row_major matrix xform;
	// bone 0
	xform = BoneMatrix[input.BlendIndices[0]];
	posX = input.BlendWeights[0] * mul( posIn, xform );

	// bone 1
	xform = BoneMatrix[input.BlendIndices[1]];
	posX += input.BlendWeights[1] * mul( posIn, xform );

	// bone 2
	xform = BoneMatrix[input.BlendIndices[2]];
	posX += input.BlendWeights[2] * mul( posIn, xform );

	// bone 3
	xform = BoneMatrix[input.BlendIndices[3]];
	posX += input.BlendWeights[3] * mul( posIn, xform );

	return posX;
}

// Vertex Shader for deferred rendering
VS_OUTPUT RenderSceneToGBufferSkinnedVS( VS_INPUT_SKINNED Input )
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	// Transform the skinned position from object space to homogeneous projection space
	float4 PositionW = CalcSkinnedWorldPosition(Input);
	PositionW = mul( PositionW, g_mtxWorld );

	// SV_Position
	Output.PositionH = mul( PositionW, g_mtxViewProjection );
	
	Output.PositionW = PositionW.xyz;

	// Normal & Tangent in world space
	Output.Normal = mul( Input.Normal, (float3x3) g_mtxWorld );
	Output.Tangent = mul( Input.Tangent, (float3x3) g_mtxWorld );

	Output.Bitangent = normalize( cross( Output.Normal, Output.Tangent ) );


	// Just pass texture uv further
	Output.TextureUV = Input.TextureUV;

	return Output;
}


// Pixel shader for feeding the GBuffer.
// PLEASE NOTE: This shader might many variations (like loading albedo from cbuffer OR texture) and so on.
PS_OUTPUT RenderSceneToGBufferPS( VS_OUTPUT Input )
{
	PS_OUTPUT Output = (PS_OUTPUT)0;

	float2 TextureUV = Input.TextureUV * uvScale + uvOffset;

	// Albedo: Output.RT0.rgb
	float4 Albedo = g_texAlbedo.Sample( samplerAnisoWrap, TextureUV );
	Albedo.rgb *= colorTint;

	// Tint Intensity: Output.RT0.a
	float TintIntensity = max(colorTint.r, max(colorTint.g, colorTint.b)) / 128.0;

	// Roughness: Output.RT0.a
	float Roughness = g_texRoughness.Sample( samplerAnisoWrap, TextureUV ).r;

	// Metallic: Output.RT2.r
	float Metalness = g_texMetalness.Sample( samplerAnisoWrap, TextureUV ).r;
	//Metalness *= smoothness;

	// Normal: Output.RT1.rgb (R11G11B10_FLOAT)
#if (USE_NORMAL_MAPPING == 1)

	float3 vNorm;

	uint2 texNormalDim;
	g_texNormals.GetDimensions( texNormalDim.x, texNormalDim.y );

	// There is normal map
	if (texNormalDim.x != 0U)
	{
		float3 sampledNormal = g_texNormals.Sample( samplerAnisoWrap, Input.TextureUV * uvScale + uvOffset).xyz;
		//sampledNormal.xy *= smoothness;
		float3 biasedNormal = sampledNormal * 2.0 - 1.0;
		biasedNormal.x *= smoothness;
		biasedNormal.y *= smoothness;

		float3x3 TBNMatrix = float3x3( normalize(Input.Tangent), normalize(Input.Bitangent), normalize(Input.Normal) );
		float3x3 BTNMatrix = float3x3(Input.Bitangent, Input.Tangent, Input.Normal);
		vNorm = normalize( mul( biasedNormal, TBNMatrix ) );
	}
	// No normal map
	else
	{
		vNorm = normalize( Input.Normal );
	}

#else
	float3 vNorm = normalize( Input.Normal );
#endif

	// Perform Crytek's BFN to pack normals within R8G8B8A8_UNORM
	CompressUnsignedNormalToNormalsBuffer( g_texNormalsFit, vNorm );

	// Feeding GBuffer
	Output.RT0 = float4(Albedo.rgb, TintIntensity);
	Output.RT1.xyz = normal_encode_xyz( vNorm );
	Output.RT2 = float4(Roughness, Metalness, specialEffect / 128.0, 0.0);
	Output.RT3 = Input.PositionW;

	return Output;
}

#endif