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

float4 ForwardPassPS( VS_OUTPUT Input ) : SV_Target0
{
	float2 TextureUV = Input.TextureUV * uvScale + uvOffset;

	// todo: calculate all
	float3 Albedo = g_texAlbedo.Sample( samplerAnisoWrap, TextureUV ).rgb;
	Albedo *= colorTint;
	Albedo = GammaToLinear(Albedo);

	float3 Normal = 0;

	uint2 texNormalDim;
	g_texNormals.GetDimensions( texNormalDim.x, texNormalDim.y );
	if (texNormalDim.x != 0U)
	{
		float3 sampledNormal = g_texNormals.Sample( samplerAnisoWrap, TextureUV ).rgb;
		float3 biasedNormal = sampledNormal * 2.0 - 1.0;

		float3x3 TBNMatrix = float3x3(normalize( Input.Bitangent ), normalize( Input.Tangent ), normalize( Input.Normal ));


		Normal = mul( biasedNormal, TBNMatrix );
		
	}
	else
	{
		Normal = normalize( Input.Normal );
	}

	// PBR
	float3 N = Normal;
	float3 L = normalize(-g_mainLightDir );
	float3 V = normalize( g_cameraPos.xyz - Input.PositionW );
	float3 H = normalize( V + L );
	float Roughness = g_texRoughness.Sample( samplerLinearWrap, TextureUV ).x;
	float Metallic = g_texMetalness.Sample( samplerLinearWrap, TextureUV ).x;

	float NdotL = max( 1e-4, dot( N, L ) );
	float NdotV = dot( N, V );
	float LdotV = saturate( dot( L, V ) );
	float NdotH = dot( N, H );
	NdotH = min( 0.999, NdotH );
	float VdotH = saturate( dot( V, H ) );
	float LdotH = saturate( dot( L, H ) );
	


	// as in UE4
	float3 SpecularColor = lerp( 0.04, Albedo, Metallic );
	float3 DiffuseColor = Albedo * (1.0 - Metallic);

	// PBR: Calculate Cook-Torrance BRDF
	float3 F = F_Schlick( SpecularColor, VdotH );
	float D =  D_GGX( Roughness, NdotH );
	float G = GeometrySmith( N, V, L, Roughness );

	float3 Nominator = F * D * G;
	float Denominator = 4.0 * NdotV * NdotL;
	Denominator = max( Denominator, 1e-4 );

	float3 Specular = Nominator / Denominator;
	
	// Poor man's ambient cubemap
	float3 ambientUp = DiffuseColor * 0.5;
	float3 ambientDown = ambientUp * 0.75;

	float fAmbientBlend = 0.5 * Normal.y + 0.5;
	float3 Ambient = ambientUp * fAmbientBlend + ambientDown * (1.0 - fAmbientBlend);

	DiffuseColor = Diffuse_Lambert(DiffuseColor);

	float3 FinalColor = 0;
	NdotL = max( 0.4, NdotL );
	FinalColor = DiffuseColor * Albedo * NdotL + Ambient;
	FinalColor *= g_mainLightColor * g_mainLightIntensity;
	FinalColor += Specular * NdotL;

	//cylinder lights
	uint i;
	for (i = 0; i < g_numCylinderLights; i++) {
		float3 vToWorld = projection_point_segment( g_cylinderLights[i].posStart, g_cylinderLights[i].posEnd, Input.PositionW ) - Input.PositionW;

		float distanceToWorld = length( vToWorld );
		if (distanceToWorld > g_cylinderLights[i].radius)
			continue;

		// falloff
		// -(1/k)*(1-(k+1)/(1+k*x^2))
		float x = distanceToWorld / g_cylinderLights[i].radius;
		float k = g_cylinderLights[i].attenuation;
		float falloff = -(1 / k)*(1 - (k + 1) / (1 + k * x*x));

		float NdotL = saturate( dot( vToWorld, Normal ) );
		FinalColor += Albedo * NdotL * g_cylinderLights[i].intensity * g_cylinderLights[i].color * falloff;
	}

	float shadow = CalculateShadow( g_texShadowMap, Input.PositionW );


	//float3 test1 = Albedo;
	//test1 += Specular * NdotL;
	//test1 *= g_mainLightColor * g_mainLightIntensity;
	//FinalColor = test1;

	FinalColor = lerp( FinalColor * 0.2, FinalColor, shadow );

	return float4(FinalColor, 1);
}
