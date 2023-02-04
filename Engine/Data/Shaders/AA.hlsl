#ifndef __ANTIALIASING_HLSL__
#define __ANTIALIASING_HLSL__

#include "Common.hlsl"
#include "PostProcessCommon.hlsl"

Texture2D texInput : register (t0);


#define FXAA_PC					1
#define FXAA_HLSL_5				1
#define FXAA_GREEN_AS_LUMA		1
#define FXAA_QUALITY__PRESET	25

#include "FXAA3_11.hlsl"

float4 AntialiasingPS( in VS_OUTPUT_POSTFX Input ) : SV_Target0
{
	FxaaTex InputFXAATex = { samplerAnisoClamp, texInput };

	float4 FXAAColor = FxaaPixelShader(
		Input.TextureUV.xy, // FxaaFloat2 pos,
		FxaaFloat4( 0.0f, 0.0f, 0.0f, 0.0f ), // FxaaFloat4 fxaaConsolePosPos,
		InputFXAATex, // FxaaTex tex,
		InputFXAATex, // FxaaTex fxaaConsole360TexExpBiasNegOne,
		InputFXAATex, // FxaaTex fxaaConsole360TexExpBiasNegTwo,
		g_Viewport.zw, // FxaaFloat2 fxaaQualityRcpFrame,
		FxaaFloat4( 0.0f, 0.0f, 0.0f, 0.0f ), // FxaaFloat4 fxaaConsoleRcpFrameOpt,
		FxaaFloat4( 0.0f, 0.0f, 0.0f, 0.0f ), // FxaaFloat4 fxaaConsoleRcpFrameOpt2,
		FxaaFloat4( 0.0f, 0.0f, 0.0f, 0.0f ), // FxaaFloat4 fxaaConsole360RcpFrameOpt2,
		0.75f, // FxaaFloat fxaaQualitySubpix,
		0.166f, // FxaaFloat fxaaQualityEdgeThreshold,
		0.0833f, // FxaaFloat fxaaQualityEdgeThresholdMin,
		0.0f, // FxaaFloat fxaaConsoleEdgeSharpness,
		0.0f, // FxaaFloat fxaaConsoleEdgeThreshold,
		0.0f, // FxaaFloat fxaaConsoleEdgeThresholdMin,
		FxaaFloat4( 0.0f, 0.0f, 0.0f, 0.0f ) // FxaaFloat fxaaConsole360ConstDir,
	);

	return FXAAColor;
}



#endif