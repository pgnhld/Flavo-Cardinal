#include "Rendering/FullscreenPass.h"
#include "FRendererD3D11.h"

ft_render::FullscreenPass::FullscreenPass()
	: fullscreenVS_(nullptr) {

}

ft_render::FullscreenPass& ft_render::FullscreenPass::getInstance() {
	static FullscreenPass instance;
	return instance;
}


bool ft_render::FullscreenPass::initialize() {
	ID3D11Device* dev = framework::FRendererD3D11::getInstance().getD3D11Device();

	// *** Create shaders ***
	ID3DBlob* blobVS = nullptr;
	bool bSuccess = false;
	if (framework::FRendererD3D11::compileD3DShader(L"..//Data//Shaders//PostProcessCommon.hlsl", "QuadVS", "vs_5_0", &blobVS)) {
		dev->CreateVertexShader(static_cast<const void*>(blobVS->GetBufferPointer()), blobVS->GetBufferSize(), nullptr, &fullscreenVS_);
		bSuccess = true;
	}

	SAFE_RELEASE(blobVS);
	return bSuccess;
}

void ft_render::FullscreenPass::cleanup() {
	SAFE_RELEASE(fullscreenVS_);
}

void ft_render::FullscreenPass::renderFullscreen(const uint32 width, const uint32 height, const uint32 viewportTopLeftX, const uint32 viewportTopLeftY ) {
	ID3D11DeviceContext* dc = framework::FRendererD3D11::getInstance().getD3D11DeviceContext();

	// Save current viewports
	D3D11_VIEWPORT viewports[16];
	UINT nViewports = 0;
	dc->RSGetViewports(&nViewports, viewports);

	// Setup one viewport for fullscreen pass
	D3D11_VIEWPORT vp;
	vp.Width = (float)width;
	vp.Height = (float)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = (float) viewportTopLeftX;
	vp.TopLeftY = (float) viewportTopLeftY;
	dc->RSSetViewports(1, &vp);

	// draw shit
	drawFullscreenTriangle();

	// Restore viewports
	dc->RSSetViewports(nViewports, viewports);
}

void ft_render::FullscreenPass::drawFullscreenTriangle() {
	ID3D11DeviceContext* dc = framework::FRendererD3D11::getInstance().getD3D11DeviceContext();

	dc->IASetInputLayout(nullptr);
	dc->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);

	dc->VSSetShader(fullscreenVS_, nullptr, 0);

	dc->Draw(3, 0);
}
