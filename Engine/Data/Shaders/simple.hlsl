#include "Common.hlsl"

Texture2D txDiffuse : register( t0 );

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float3 Pos : POSITION;
	float2 Tex : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Bitangent : BITANGENT;
};

struct VS_INPUT_SKINNED
{
	float3 Pos : POSITION;
	float2 Tex : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float3 Bitangent : BITANGENT;
	uint4 BlendIndices : BLENDINDICES;
	float4 BlendWeights : BLENDWEIGHTS;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD0;
};


//--------------------------------------------------------------------------------------
// Vertex Shader - simple
//--------------------------------------------------------------------------------------
PS_INPUT VSMain( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    output.Pos = mul( float4(input.Pos, 1.0), World );
    output.Pos = mul( output.Pos, View );
    output.Pos = mul( output.Pos, Projection );
    output.Tex = input.Tex;
    
    return output;
}


//--------------------------------------------------------------------------------------
// Vertex Shader - skinned
//--------------------------------------------------------------------------------------
PS_INPUT VSMainSkinning( VS_INPUT_SKINNED input )
{
	PS_INPUT output = (PS_INPUT)0;

	row_major matrix xform;
	
	float4 posIn = float4(input.Pos, 1.0);
	float4 posX = 0;

	// bone 0
	xform = BoneMatrix[ input.BlendIndices[0] ];
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


	output.Pos = mul( posX, World );
	//output.Pos = mul( posIn, World );
	output.Pos = mul( output.Pos, View );
	output.Pos = mul( output.Pos, Projection );
	output.Tex = input.Tex;

	return output;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float rand_1(float2 uv)
{
   float2 noise = (frac(sin(dot(uv ,float2(12.9898,78.233)*2.0)) * 43758.5453));
   return abs(noise.x + noise.y) * 0.5;
}

float4 PSMain( PS_INPUT input) : SV_Target
{
    //return float4(rand_1(input.Pos.xy), rand_1(input.Pos.xz), rand_1(input.Pos.yz), 1.0);
    return txDiffuse.Sample( samplerAnisoWrap, input.Tex );
}