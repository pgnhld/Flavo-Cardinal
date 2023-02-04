#include "Rendering/RenderSystemEyeAdaptation.h"
#include "FRendererD3D11.h"
#include "Rendering/FullscreenPass.h"

ft_render::FEyeAdaptation::FEyeAdaptation() 
	: currentRenderLumTarget_(0) {

}

//------------------------------------------------------------------------
ft_render::FEyeAdaptation::~FEyeAdaptation() {

}

//------------------------------------------------------------------------
bool ft_render::FEyeAdaptation::initialize( const uint32 fullscreenWidth, const uint32 fullscreenHeight, const uint32 luminanceMips /*= 10*/ ) {
	framework::FRendererD3D11& renderer = framework::FRendererD3D11::getInstance();
	ID3D11Device* dev = renderer.getD3D11Device();

	// create shaders
	ID3DBlob* blobPSAdaptedLum = nullptr;
	if (framework::FRendererD3D11::compileD3DShader( L"..//Data//Shaders//AdaptedLuminance.hlsl", "PSAdaptedLuminance", "ps_5_0", &blobPSAdaptedLum )) {
		dev->CreatePixelShader( (const void*)blobPSAdaptedLum->GetBufferPointer(), blobPSAdaptedLum->GetBufferSize(), nullptr, &psAdaptedLuminance_ );
	}
	SAFE_RELEASE( blobPSAdaptedLum );


	texLogLuminance_.initialize(dev, 
								 1 << (luminanceMips-1),
								 1 << (luminanceMips-1),
								 DXGI_FORMAT_R16_FLOAT,
								 false,
								 true,
								 luminanceMips );
	for (uint32 i=0; i < 2; ++i) {
		texAdaptedLuminance_[i].initialize(dev, 1, 1, DXGI_FORMAT_R32_FLOAT, false);
	}

	return true;
}

void ft_render::FEyeAdaptation::release() {
	SAFE_RELEASE(psAdaptedLuminance_);

	texLogLuminance_.cleanup();
	for (uint32 i=0; i < 2; ++i) {
		texAdaptedLuminance_[i].cleanup();
	}
}

void ft_render::FEyeAdaptation::renderAverageLuminanceAndEyeAdaptation( const framework::FRenderTexture2D& inputColor ) {
	renderAverageLuminance(inputColor);
	renderEyeAdaptation();

	currentRenderLumTarget_ = !currentRenderLumTarget_;
}

const framework::FRenderTexture2D& ft_render::FEyeAdaptation::getAdaptedLuminanceTexture() const {
	return texAdaptedLuminance_[!currentRenderLumTarget_];
}

void ft_render::FEyeAdaptation::renderAverageLuminance( const framework::FRenderTexture2D& inputColor ) {
	ID3D11DeviceContext* devCon = framework::FRendererD3D11::getInstance().getD3D11DeviceContext(); 
	
	ID3D11RenderTargetView* rtViews[1] = { texLogLuminance_.rtv_ };
	devCon->OMSetRenderTargets(1, rtViews, nullptr);

	ID3D11ShaderResourceView* pInputTextures[1] = { inputColor.srv_ };
	devCon->PSSetShaderResources(0, 1, pInputTextures);

	FullscreenPass::getInstance().renderFullscreen(texLogLuminance_.width_, texLogLuminance_.height_);

	// generate mipmap to get 1x1 texture
	devCon->GenerateMips(texLogLuminance_.srv_);
}

void ft_render::FEyeAdaptation::renderEyeAdaptation() {
	ID3D11DeviceContext* devCon = framework::FRendererD3D11::getInstance().getD3D11DeviceContext();

	// Output: adapted luminance
	ID3D11RenderTargetView* adaptedLuminanceRTV = texAdaptedLuminance_[currentRenderLumTarget_].rtv_;

	devCon->ClearRenderTargetView(adaptedLuminanceRTV, framework::CLEAR_COLOR);
	devCon->OMSetRenderTargets(1, &adaptedLuminanceRTV, nullptr);

	// Input: previous adapted luminance & current luminance
	ID3D11ShaderResourceView* lastLuminanceSRV = texAdaptedLuminance_[!currentRenderLumTarget_].srv_;
	ID3D11ShaderResourceView* inputsSRV[2] = { texLogLuminance_.srv_, lastLuminanceSRV };
	devCon->PSSetShaderResources(0, 2, inputsSRV);
	devCon->PSSetShader(psAdaptedLuminance_, nullptr, 0);

	FullscreenPass::getInstance().renderFullscreen(1, 1);

}
