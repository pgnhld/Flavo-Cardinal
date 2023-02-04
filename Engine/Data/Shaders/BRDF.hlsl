#ifndef __BRDF_HLSL__
#define __BRDF_HLSL__


float3 Diffuse_Lambert( in float3 DiffuseColor )
{
	return DiffuseColor * (1.0 / PI);
}

// [Burley 2012, "Physically-Based Shading at Disney"]
float3 Diffuse_Burley( in float3 DiffuseColor, in float Roughness, in float NoV, in float NoL, in float VoH )
{
	float FD90 = 0.5 + 2 * VoH * VoH * Roughness;
	float FdV = 1 + (FD90 - 1) * Pow5( 1 - NoV );
	float FdL = 1 + (FD90 - 1) * Pow5( 1 - NoL );
	return DiffuseColor * ((1 / PI) * FdV * FdL);
}



// GGX / Trowbridge-Reitz
// [Walter et al. 2007, "Microfacet models for refraction through rough surfaces"]
float D_GGX( float Roughness, float NoH )
{
	float a = Roughness * Roughness;
	float a2 = a * a;
	float d = (NoH * a2 - NoH) * NoH + 1;	// 2 mad
	return a2 / (PI*d*d);					// 4 mul, 1 rcp
}


// [Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"]
float3 F_Schlick( float3 SpecularColor, float VoH )
{
	float Fc = Pow5( 1 - VoH );					// 1 sub, 3 mul
												//return Fc + (1 - Fc) * SpecularColor;		// 1 add, 3 mad

												// Anything less than 2% is physically impossible and is instead considered to be shadowing
	return saturate( 50.0 * SpecularColor.g ) * Fc + (1 - Fc) * SpecularColor;
}


float GeometrySchlickGGX( float NdotV, float roughness )
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;

	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}
float GeometrySmith( float3 N, float3 V, float3 L, float roughness )
{
	float NdotV = max( dot( N, V ), 0.0 );
	float NdotL = max( dot( N, L ), 0.0 );
	float ggx2 = GeometrySchlickGGX( NdotV, roughness );
	float ggx1 = GeometrySchlickGGX( NdotL, roughness );

	return ggx1 * ggx2;
}

// Appoximation of joint Smith term for GGX
// [Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"]
float Vis_SmithJointApprox( float Roughness, float NoV, float NoL )
{
	float a = (Roughness*Roughness);
	float Vis_SmithV = NoL * (NoV * (1 - a) + a);
	float Vis_SmithL = NoV * (NoL * (1 - a) + a);
	// Note: will generate NaNs with Roughness = 0.  MinRoughness is used to prevent this
	return 0.5 * rcp( Vis_SmithV + Vis_SmithL );
}

// Tuned to match behavior of Vis_Smith
// [Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"]
float Vis_Schlick( float Roughness, float NoV, float NoL )
{
	float k = (Roughness*Roughness) * 0.5;
	float Vis_SchlickV = NoV * (1 - k) + k;
	float Vis_SchlickL = NoL * (1 - k) + k;
	return 0.25 / (Vis_SchlickV * Vis_SchlickL);
}




#endif