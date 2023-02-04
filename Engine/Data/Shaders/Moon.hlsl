#include "Common.hlsl"

Texture2D texMoon : register (t0);

struct VS_INPUT
{
	float3 Position		: POSITION;
};

struct VS_OUTPUT
{
	float4 PositionH	: SV_POSITION;
	float3 PositionW	: POSITION;
	float2 Texcoord		: TEXCOORD;
};


VS_OUTPUT MoonVS( VS_INPUT Input )
{
	VS_OUTPUT vout = (VS_OUTPUT) 0;

	Input.Position = -g_mainLightDir;
	
	//float4 PositionW = mul( float4(Input.Position, 1.0), g_mtxWorld );
	float4 PositionW = float4( Input.Position * 220, 1.0 );
	vout.PositionW = PositionW.xyz;

	return vout;
}

[maxvertexcount(4)]
void MoonGS(point VS_OUTPUT Input[1], inout TriangleStream<VS_OUTPUT> OutputStream)
{
	float halfSize =  50;

	float3 planeNormal = Input[0].PositionW- g_cameraPos.xyz;
	//planeNormal.y = 0.0;
	planeNormal = normalize(planeNormal);


	float3 cameraRight = float3(g_mtxView._11, g_mtxView._21, g_mtxView._31);
	float3 cameraUp = float3(g_mtxView._12, g_mtxView._22, g_mtxView._32);

	float3 upVector = normalize( cameraUp );
	float3 rightVector = normalize( cameraRight );

	//upVector *= 0.5;
	//rightVector *= 0.5;
	
	upVector *= halfSize;
	rightVector *= halfSize;

	// Compute the local coordinate system fro the sprite relative
	// to the world space such that the billboard is aligned and faces the eye.
	float3 up = float3(0.0, 1.0, 0.0);
	float3 look = g_cameraPos.xyz - Input[0].PositionW;
	look = normalize(look);
	float3 right = ( cross(up, look) );

	// Compute triangle strip vertices (quad) in world space
	float3 halfWidth =  0.5f * halfSize;
	float3 halfHeight = 0.5f * halfSize;
	
	
	float3 vert[4];
	vert[0] = Input[0].PositionW + halfWidth * right - halfHeight * up;
	vert[1] = Input[0].PositionW + halfWidth * right + halfHeight * up;
	vert[2] = Input[0].PositionW - halfWidth * right - halfHeight * up;
	vert[3] = Input[0].PositionW - halfWidth * right + halfHeight * up;
	
	// Get billboards texture coordinates
	float2 texCoord[4];
	texCoord[0] = float2(0, 1);
	texCoord[1] = float2(0, 0);
	texCoord[2] = float2(1, 1);
	texCoord[3] = float2(1, 0);

	VS_OUTPUT outputVert;
	[unroll] for (int i=0; i < 4; i++)
	{
		outputVert.PositionH = mul( float4(vert[i], 1.0), g_mtxViewProjection );
		outputVert.PositionW = vert[i];
		outputVert.Texcoord = texCoord[i];

		OutputStream.Append(outputVert);
	}
}


float4 MoonPS( VS_OUTPUT Input ) : SV_Target0
{
	float4 color = texMoon.Sample( samplerLinearClamp, Input.Texcoord );
	clip(color.a - 0.25);

	color.rgb *= 4;

	return color;
}
