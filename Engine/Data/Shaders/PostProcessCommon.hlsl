#ifndef __POSTPROCESSCOMMON_HLSL__
#define __POSTPROCESSCOMMON_HLSL__


// Vertex Shader for Post Processing
// http://www.slideshare.net/DevCentralAMD/vertex-shader-tricks-bill-bilodeau
struct VS_OUTPUT_POSTFX
{
	float4 Position									: SV_Position;
	float2 TextureUV								: TEXCOORD0;
};


VS_OUTPUT_POSTFX QuadVS( in uint id : SV_VertexID )
{
	VS_OUTPUT_POSTFX Output;

	Output.Position.x = float( id / 2 ) * 4.0 - 1.0;
	Output.Position.y = float( id % 2 ) * 4.0 - 1.0;
	Output.Position.z = 0.0;
	Output.Position.w = 1.0;

	Output.TextureUV.x = (float)(id / 2) * 2.0;
	Output.TextureUV.y = 1.0 - (float)(id % 2) * 2.0;

	return Output;
}



#endif