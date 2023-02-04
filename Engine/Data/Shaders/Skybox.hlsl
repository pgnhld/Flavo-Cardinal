#include "Common.hlsl"

struct VS_INPUT
{
	float3 Position		: POSITION;
};

struct VS_OUTPUT
{
	float4 PositionH	: SV_POSITION;
	float3 Direction	: TEXCOORD0;
	float3 PositionW	: POSITION;
};


VS_OUTPUT SkyboxVS( VS_INPUT Input )
{
	VS_OUTPUT vout = (VS_OUTPUT) 0;

	
	vout.Direction = normalize(Input.Position);
	float4 PositionW = mul( float4(Input.Position, 1.0), g_mtxWorld );
	vout.PositionW = PositionW.xyz;
	vout.PositionH = mul(PositionW, g_mtxViewProjection).xyww;		// Depth = 1.0

	return vout;
}


float4 SkyboxPS( VS_OUTPUT Input ) : SV_Target0
{
	// flip direction for now
	float3 samplingDir = Input.Direction;

	// moon/sun fun
	float3 vCamToWorld = normalize( Input.PositionW - g_cameraPos);
	float cosTheta = saturate( dot(vCamToWorld, -g_mainLightDir) );

	float moonGradient = pow(cosTheta, 254);

	// calc moon uv
	/*
	// angle in radians, repating
	float xRadians = (1 - g_elapsedTime) * 2 * 3.141592;

	// normalize xRadians to [0-1]
	float moonSin;
	float moonCos;
	sincos(xRadians, moonSin, moonCos );

	// V in texture space goes down
	moonSin *= -1.0;

	// scale to half
	moonSin *= 0.25;
	moonCos *= 0.25;

	moonSin += 0.25;
	moonCos += 0.25;	

	float3 moonColor = texMoon.Sample( samplerLinearWrap, float2(moonCos, moonSin) ).rgb;
	moonColor = GammaToLinear(moonColor);
	*/

	float3 moonColor = GammaToLinear( getRGB(192, 224, 240 ) );
	// boost it a bit
	moonColor = lerp( moonColor, GammaToLinear(getRGB(255, 217, 192) ), pow(moonGradient, 0.10) );
	moonColor *= 5;

	


	float4 skyColor = g_texSkybox.SampleLevel( samplerLinearWrap, samplingDir, 0);
	skyColor.rgb = GammaToLinear(skyColor.rgb);

	float3 fogColor = GammaToLinear( getRGB(1, 31, 56) );

	float downDotView = dot( float3(0.0, -1.0, 0.0), vCamToWorld );
	downDotView *= downDotView;
	skyColor.rgb = lerp(skyColor.rgb, fogColor, downDotView );

	skyColor.rgb = lerp( skyColor.rgb, moonColor, moonGradient);


	return skyColor;
}
