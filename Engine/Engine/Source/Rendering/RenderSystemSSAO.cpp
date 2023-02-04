#include "Rendering/RenderSystemSSAO.h"
#include "FRendererD3D11.h"
#include "Rendering/FullscreenPass.h"


ft_render::FAmbientOcclusionSSAO::FAmbientOcclusionSSAO() {

}

ft_render::FAmbientOcclusionSSAO::~FAmbientOcclusionSSAO() {

}

bool ft_render::FAmbientOcclusionSSAO::initialize( const uint32 fullscreenWidth, const uint32 fullscreenHeight ) {
	framework::FRendererD3D11& renderer = framework::FRendererD3D11::getInstance();
	ID3D11Device* dev = renderer.getD3D11Device();
	ID3D11DeviceContext* devContext = renderer.getD3D11DeviceContext();

	// create shaders
	ID3DBlob* blobPSSSAO = nullptr;
	if (framework::FRendererD3D11::compileD3DShader( L"..//Data//Shaders//SSAO.hlsl", "PS_SSAO", "ps_5_0", &blobPSSSAO )) {
		dev->CreatePixelShader( (const void*)blobPSSSAO->GetBufferPointer(), blobPSSSAO->GetBufferSize(), nullptr, &psSSAO_ );
	}
	SAFE_RELEASE( blobPSSSAO );

	
	const uint32 ssaoWidth =  fullscreenWidth / 2;
	const uint32 ssaoHeight = fullscreenHeight / 2;
	texSSAO_.initialize(dev, ssaoWidth, ssaoHeight, DXGI_FORMAT_R8_UNORM);

	texSSAONoise_.load(dev, devContext, std::string("..//Data//Textures//Engine//ssaoNoise2.dds"));


	return true;
}

void ft_render::FAmbientOcclusionSSAO::release() {
	texSSAONoise_.cleanup();

	texSSAO_.cleanup();

	SAFE_RELEASE(psSSAO_);
}

void ft_render::FAmbientOcclusionSSAO::renderSSAO( const framework::FRenderTexture2D& texWorldPos, const framework::FRenderTexture2D& texNormals ) {
	ID3D11DeviceContext* devCon = framework::FRendererD3D11::getInstance().getD3D11DeviceContext();

	ID3D11ShaderResourceView* pSSAOInputs[3] = 	{
		texWorldPos.srv_,
		texNormals.srv_,
		texSSAONoise_.getSRV()
	};
	devCon->PSSetShader(psSSAO_, nullptr, 0);
	devCon->PSSetShaderResources( 1, 3, pSSAOInputs );

	ID3D11RenderTargetView* pOutputs[1] = {
		texSSAO_.rtv_
	};
	devCon->OMSetRenderTargets(1, pOutputs, nullptr);	

	FullscreenPass::getInstance().renderFullscreen(texSSAO_.width_, texSSAO_.height_);

	// cleanup
	pOutputs[0] = nullptr;
	devCon->OMSetRenderTargets( 1, pOutputs, nullptr );
}

const framework::FRenderTexture2D& ft_render::FAmbientOcclusionSSAO::getSSAOTexture() const {
	return texSSAO_;
}
