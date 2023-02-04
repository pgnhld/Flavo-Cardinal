#include "Common.hlsl"
#include "BRDF.hlsl"

Texture2D g_texAlbedo	: register (t0);
Texture2D g_texNormals	: register (t1);
Texture2D g_texRoughness : register (t2);
Texture2D g_texMetalness : register (t3);

Texture2D g_texShadowMap : register (t6);


// Vertex Shader is the same as in deferred shading
struct VS_OUTPUT
{
	float4 PositionH	: SV_POSITION;
	float3 PositionW	: POSITION;
	float3 Normal		: NORMAL;
	float3 Tangent		: TANGENT;
	float3 Bitangent	: BITANGENT;
	float2 TextureUV	: TEXCOORD0;
};

float3 mod289( in float3 x )
{
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}

float2 mod289( in float2 x )
{
	return x - floor(x * (1.0 / 289.0)) * 289.0;
}

float3 permute( in float3 x )
{
	return mod289( ((x*34.0) + 1.0)*x );
}

float snoise( in float2 v )
{
	const float4 C = float4( 0.211324865405187,  // (3.0-sqrt(3.0))/6.0
							 0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
							 -0.577350269189626,  // -1.0 + 2.0 * C.x
							 0.024390243902439 ); // 1.0 / 41.0

	// First corner
	float2 i = floor(v + dot(v, C.yy) );
	float2 x0 = v -  i * dot(i, C.xx) ;

	// other corners
	float2 i1;
	i1 = (x0.x > x0.y) ? float2(1.0, 0.0) : float2(0.0, 1.0);
	
	float4 x12 = x0.xyxy + C.xxzz;
	x12.xy -= i1;

	// permutations
	i = mod289(i);	// avoid truncation effects in permutation

	float3 p = permute( permute( i.y + float3(0, i1.y, 1) )
						+ i.x + float3(0, i1.x, 1.0) );

	float3 m = float3(0.5, 0.5, 0.5) - float3( dot(x0,x0),
												   dot(x12.xy, x12.xy), 
												   dot(x12.zw, x12.zw) );

	m = max( m, 0.028 );

	//return dot( m, m) * 130;

	m = m*m;
	m = m*m;
	

	// Gradients: 41 points uniformly over a line, mapped onto a diamond
	// The ring size = 17*17 = 289 is close to a multiple of 17  (41*7 = 287)
	float3 x = 2.0 * frac(p * C.www) - 1.0;
	float3 h = abs(x) - 0.5;
	float3 ox = floor(x + 0.5);
	float3 a0 = x-ox;

	// Normalise gradients implicitly by scaling m
	// Approximation of: m *= inversesqrt( a0*a0 + h*h )
	m *= 1.79284291400159 - 0.85373472095314 * (a0*a0 + h*h);
	//m.x *= rsqrt(a0.x*a0.x + h.x*h.x);
	//m.y *= rsqrt(a0.y*a0.y + h.y*h.y);
	//m.z *= rsqrt(a0.z*a0.z + h.z*h.z);

	// Compute final noise value at P
	float3 g;
	g.x =  a0.x * x0.x + h.x * x0.y;
	g.yz = a0.yz * x12.xz + h.yz * x12.yw;
	//return 130*dot(g,g);

	return 130.0 * dot(m, g);

}

float rand( in float2 co )
{
	return frac( sin( dot( co.xy, float2( 12.9898, 78.233 ) ) ) * 43758.5453 );
}


float4 ForwardPassHologramPS( VS_OUTPUT Input ) : SV_Target0
{
	float2 uv = Input.TextureUV;// * uvScale + g_elapsedTime;

	float4 fragColor = 0;
	fragColor.rgb *= colorTint;

	float time =  g_elapsedTime;
	time = fmod(time, 20);
	time += 22.5;

	// Create large, incidental noise waves
	float noise = max( 0.0, 
					   snoise( float2( time, uv.y * 0.3 ) ) - 0.3 ) * (1.0 / 0.7);

	//if (abs(noise) > 0.001)
//		return float4(1,0,0,1);

	// Offset by smaller, constant noise waves
	float noiseIncr = (snoise( float2(time * 5.0, uv.y * 2.4) ) - 0.5);
	noise = noise + noiseIncr * 0.15;

	// Apply the noise as x displacement for every line
	float xpos = uv.x - noise * noise * 0.25;
	
	fragColor = g_texAlbedo.Sample( samplerLinearWrap, float2(xpos, uv.y) ).rgba;
	
	// Mix in some random interference for lines
	float xNoise = clamp(noise, -3.0, +3.0);
	fragColor = lerp( fragColor, rand( float2( uv.y * time, uv.y * time ) ).xxxx, xNoise.xxxx * 0.15 );

	

	// Shift green/blue channels (using the red channel)
	//fragColor.g = lerp( fragColor.r,g_texAlbedo.Sample( samplerLinearWrap, float2( xpos + noise * 0.05, uv.y )).g, 0.25 );
	//fragColor.b = lerp( fragColor.r,g_texAlbedo.Sample( samplerLinearWrap, float2( xpos - noise * 0.05, uv.y )).b, 0.25 );



	/* Apply an animated line pattern */

	// 1) With randomized line patterns
	/*
	[branch] if (int(uv.y*2500*noise + g_elapsedTime * 20.0) % 16 == 0) {
		fragColor.rgb *= 0.15;
	}
	*/
	
	// 2) With smooth line patterns
	int currLine = int( uv.y * 800 + g_elapsedTime * 20.0 ) % 22;  
	[branch] if (currLine < 8) {
		fragColor.rgb *= 0.85;
	}
	

	return float4(fragColor);
}
