#ifndef __SHADOWS_HLSL__
#define __SHADOWS_HLSL__

#include "Common.hlsl"


// Input/Output Vertex

// We need only 1 vertex buffer for static meshes
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

// We only need position in light space
struct VS_OUTPUT
{
	float4 PositionH	: SV_POSITION;
};


// Vertex Shader for static meshes
VS_OUTPUT RenderDepthStaticVS( VS_INPUT Input )
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	// Transform the position from object space to homogeneous projection space
	float4 PositionW = mul( float4(Input.Position, 1.0), g_mtxWorld );
	Output.PositionH = mul( PositionW, g_mtxViewProjectionShadow );

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

// Vertex Shader for skinned meshes
VS_OUTPUT RenderDepthSkinnedVS( VS_INPUT_SKINNED Input )
{
	VS_OUTPUT Output = (VS_OUTPUT)0;

	// Transform the skinned position from object space to homogeneous projection space
	float4 PositionW = CalcSkinnedWorldPosition(Input);
	PositionW = mul( PositionW, g_mtxWorld );

	// SV_Position
	Output.PositionH = mul( PositionW, g_mtxViewProjectionShadow );
	return Output;
}

#endif