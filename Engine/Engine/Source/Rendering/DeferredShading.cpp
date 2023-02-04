#include "Rendering/DeferredShading.h"
#include "FRendererD3D11.h"

bool ft_render::GBuffer::initialize( const uint32 width, const uint32 height ) {	
	ID3D11Device* device = framework::FRendererD3D11::getInstance().getD3D11Device();
	
	// Depth/Stencil buffer
	const DXGI_FORMAT dsFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	ds_.initialize(device, width, height, dsFormat, true);

	// Render Targets
	rt0_.initialize(device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
	rt1_.initialize(device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
	rt2_.initialize(device, width, height, DXGI_FORMAT_R8G8B8A8_UNORM);
	rt3_.initialize( device, width, height, DXGI_FORMAT_R16G16B16A16_FLOAT);

	return true;
}

void ft_render::GBuffer::cleanup() {
	ds_.cleanup();
	rt0_.cleanup();
	rt1_.cleanup();
	rt2_.cleanup();
	rt3_.cleanup();
}

void ft_render::GBuffer::attach() {
	ID3D11RenderTargetView* GBufferViews[4] = {
		rt0_.rtv_,
		rt1_.rtv_,
		rt2_.rtv_,
		rt3_.rtv_
	};

	ID3D11DeviceContext* dc = framework::FRendererD3D11::getInstance().getD3D11DeviceContext();
	dc->OMSetRenderTargets(4, GBufferViews, ds_.m_DSV);
}

void ft_render::GBuffer::unbind() {
	ID3D11RenderTargetView* nullRTV[5] = {
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	};

	ID3D11DeviceContext* dc = framework::FRendererD3D11::getInstance().getD3D11DeviceContext();
	dc->OMSetRenderTargets( 5, nullRTV, nullptr );
}

void ft_render::GBuffer::clear(bool isReversedDepth, bool clearRTs) {
	const float depthClearValue = isReversedDepth ? 0.0f : 1.0f;

	ID3D11DeviceContext* dc = framework::FRendererD3D11::getInstance().getD3D11DeviceContext();
	dc->ClearDepthStencilView(ds_.m_DSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depthClearValue, 0);

	if (clearRTs) {
		dc->ClearRenderTargetView(rt0_.rtv_, framework::CLEAR_COLOR);
		dc->ClearRenderTargetView(rt1_.rtv_, framework::CLEAR_COLOR);
		dc->ClearRenderTargetView(rt2_.rtv_, framework::CLEAR_COLOR);
		dc->ClearRenderTargetView(rt3_.rtv_, framework::CLEAR_COLOR);
	}
}
