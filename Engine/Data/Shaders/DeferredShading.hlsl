#ifndef __DEFERREDSHADING_HLSL__
#define __DEFERREDSHADING_HLSL__

#include "Common.hlsl"
#include "DeferredShadingCommon.hlsl"
#include "PostProcessCommon.hlsl"
#include "BRDF.hlsl"

// GBuffer Render Targets
Texture2D TexDepth				: register (t0);
Texture2D RT0					: register (t1);
Texture2D RT1					: register (t2);
Texture2D RT2					: register (t3);
Texture2D RT3					: register (t4);
Texture2D TexAO					: register (t5);
Texture2D ShadowMap				: register (t6);


GBufferData DecodeGBuffer( int3 location )
{
	GBufferData Output = (GBufferData) 0;
	

	// Read raw data from GBuffer
	float4 rt0 = RT0.Load( location );
	float4 rt2 = RT2.Load( location );

	// And write it to output
	Output.Albedo = GammaToLinear( rt0.xyz );
	Output.TintIntensity = rt0.w * 128.0;

	Output.Roughness = rt2.x;
	Output.Metalness = rt2.y;
	Output.specialEffect = rt2.z * 128.0;

	Output.Normal = RT1.Load( location ).xyz;
	Output.Normal = normal_decode( Output.Normal );	// don't forget about normalizing!


	// Determine world position
	
	//float fDepthBufferDepth = TexDepth.Load( location ).x;
	//float4 vWSPosition = mul( float4((float)location.x + 0.5f, (float)location.y + 0.5f, fDepthBufferDepth, 1.0f), g_mtxInvViewportProjAware );
	//Output.WorldPos = vWSPosition.xyz / vWSPosition.w;
	Output.WorldPos = RT3.Load(location);

	return Output;
}



float4 PSMain( VS_OUTPUT_POSTFX Input ) : SV_TARGET0
{
	int3 location = int3(Input.Position.xy, 0);

	GBufferData GBuffer = DecodeGBuffer( location );
	float fDepthBufferDepth = TexDepth.Load( location ).x;
	//fDepthBufferDepth = fDepthBufferDepth * g_depthScaleFactors.x + g_depthScaleFactors.y;

	


	float fColorMultiplier = 1.0;
	fColorMultiplier *= (GBuffer.specialEffect + 1);

	//switch (GBuffer.SpecialEffectID)
	//{
	//	case 1:
	//	{
	//		fColorMultiplier *= 100.0;
	//	}		
	//	break;
	//}


	float3 FinalColor = 0;
	[branch] if (fDepthBufferDepth < 1.0)
	{
		float3 Albedo = GBuffer.Albedo;
		Albedo *= GBuffer.TintIntensity;

		int MiddleScreenX = (int)(g_Viewport.x) / 2;
		float3 cameraPostion = (location.x < MiddleScreenX) ? g_cameraPos1.xyz : g_cameraPos2.xyz;


		float3 N = GBuffer.Normal;
		float3 L = normalize( -g_mainLightDir );
		float3 V = normalize( cameraPostion - GBuffer.WorldPos );
		float3 H = normalize( V + L );
		float  Roughness = GBuffer.Roughness;
		float  Metallic = GBuffer.Metalness;

		float NdotL = saturate( dot( N, L ) );
		float NdotV = abs( dot( N, V ) ) + 1e-5f;	// avoid artifacts
		float LdotV = saturate( dot( L, V ) );
		float NdotH = saturate( dot( N, H ) );
		NdotH = min( 0.98, NdotH );
		float VdotH = saturate( dot( V, H ) );
		float LdotH = saturate( dot( L, H ) );
		
		// as in UE4
		float3 SpecularColor = lerp( 0.04, Albedo.rgb, Metallic );
		float3 DiffuseColor = Albedo * (1.0f - Metallic);
		
		// PBR: Calculating Cook-Torrance BRDF
		float3 F = F_Schlick( SpecularColor, VdotH );
		float D = D_GGX( Roughness, NdotH );

		{
			float3 debugCameraPos = normalize( g_cameraPos.xyz );
			debugCameraPos = debugCameraPos * 0.5 + 0.5;
			debugCameraPos = normalize( g_cameraPos.xyz );

			//return float4(debugCameraPos, 1.0);
		}

		

		float G = GeometrySmith( N, V, L, Roughness );

		float3 Nominator = F * D * G;
		float Denominator = 4.0 * NdotV * NdotL;
		Denominator = max( Denominator, 1e-4 );

		float3 Specular = Nominator / Denominator;
		
		// Poor man's ambient cubemap
		float AO = TexAO.Sample( samplerLinearClamp, Input.TextureUV );

		float3 ambientUp = DiffuseColor * 0.5;
		float3 ambientDown = ambientUp * 0.75;
		float fAmbientBlend = 0.5 * N.y + 0.5;
		float3 Ambient = ambientUp * fAmbientBlend + ambientDown * (1.0 - fAmbientBlend);
	
		DiffuseColor = Diffuse_Lambert(DiffuseColor);
		
	

		FinalColor = DiffuseColor * Albedo*NdotL + Ambient;
		FinalColor *= g_mainLightColor * g_mainLightIntensity;
		FinalColor += Specular * NdotL;

		//cylinder lights
		uint i;
		for (i = 0; i < g_numCylinderLights; i++) {
			float3 vToWorld = projection_point_segment(g_cylinderLights[i].posStart, g_cylinderLights[i].posEnd, GBuffer.WorldPos) - GBuffer.WorldPos;

			float distanceToWorld = length(vToWorld);
			if (distanceToWorld > g_cylinderLights[i].radius)
				continue;

			// falloff
			// -(1/k)*(1-(k+1)/(1+k*x^2))
			float x = distanceToWorld / g_cylinderLights[i].radius;
			float k = g_cylinderLights[i].attenuation;
			float falloff = -(1 / k)*(1 - (k + 1) / (1 + k * x*x));

			float NdotL = saturate(dot(vToWorld, N));
			FinalColor += Albedo * NdotL * g_cylinderLights[i].intensity * g_cylinderLights[i].color * falloff;
		}

		float shadow = CalculateShadow(ShadowMap, GBuffer.WorldPos);
		//return shadow.xxxx;

		FinalColor = lerp( FinalColor * 0.2, FinalColor, shadow);
	}

	FinalColor *= fColorMultiplier;

	return float4(max( FinalColor, 1e-5 ), 1);
}


#endif