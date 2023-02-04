#include "FD3D11CommonStates.h"

static framework::rendering::FD3D11States* s_pStates = nullptr;

framework::rendering::FD3D11States::FD3D11States() {
	pBackfaceCull_RS = nullptr;
	pBackfaceCull_WireFrame_RS = nullptr;
	pBackfaceCullMSAA_RS = nullptr;
	pBackfaceCull_WireFrameMSAA_RS = nullptr;
	pNoCull_RS = nullptr;

	pDepthNoStencil_DS = nullptr;
	pNoDepthNoStencil_DS = nullptr;
	pStencilSetBits_DS = nullptr;
	pNoDepthStencilComplex_DS = nullptr;
	pDepthStencilComplex_DS = nullptr;
	pDS_Skybox = nullptr;
	pDS_DepthGreater = nullptr;

	pNoBlend_BS = nullptr;

	pSamplerPointWrap = nullptr;
	pSamplerPointClamp = nullptr;
	pSamplerLinearWrap = nullptr;
	pSamplerLinearClamp = nullptr;
	pSamplerAnisoWrap = nullptr;
	pSamplerAnisoClamp = nullptr;
	pSamplerComparisonLinear = nullptr;
}

framework::rendering::FD3D11States::~FD3D11States() {
	ReleaseResources();
}

void framework::rendering::FD3D11States::Claim() {
	if (nullptr == s_pStates) {
		s_pStates = new framework::rendering::FD3D11States();
	}
}

void framework::rendering::FD3D11States::Destroy() {
	if (s_pStates) {
		delete s_pStates;
		s_pStates = nullptr;
	}
}

framework::rendering::FD3D11States* framework::rendering::FD3D11States::Get() {
	return s_pStates;
}

void framework::rendering::FD3D11States::CreateResources(ID3D11Device *pd3dDevice) {
	CreateSamplers(pd3dDevice);
	CreateDepthStencilStates(pd3dDevice);
	CreateBlendStates(pd3dDevice);
	CreateRasterizerStates(pd3dDevice);
}

void framework::rendering::FD3D11States::ReleaseResources() {
	SAFE_RELEASE(pBackfaceCull_RS);
	SAFE_RELEASE(pBackfaceCull_WireFrame_RS);
	SAFE_RELEASE(pBackfaceCullMSAA_RS);
	SAFE_RELEASE(pBackfaceCull_WireFrameMSAA_RS);
	SAFE_RELEASE(pNoCull_RS);

	SAFE_RELEASE(pDepthNoStencil_DS);
	SAFE_RELEASE(pNoDepthNoStencil_DS);
	SAFE_RELEASE(pStencilSetBits_DS);
	SAFE_RELEASE(pNoDepthStencilComplex_DS);
	SAFE_RELEASE(pDepthStencilComplex_DS);
	SAFE_RELEASE(pDS_Skybox);
	SAFE_RELEASE(pDS_DepthGreater);

	SAFE_RELEASE(pNoBlend_BS);
	SAFE_RELEASE(pBlend_BS);
	SAFE_RELEASE(pBlendAlphaToCoverage_BS);
	SAFE_RELEASE(pBlendAdditive_BS);

	SAFE_RELEASE(pSamplerPointWrap);
	SAFE_RELEASE(pSamplerPointClamp);
	SAFE_RELEASE(pSamplerLinearWrap);
	SAFE_RELEASE(pSamplerLinearClamp);
	SAFE_RELEASE(pSamplerAnisoWrap);
	SAFE_RELEASE(pSamplerAnisoClamp);
	SAFE_RELEASE(pSamplerComparisonLinear);
}

void framework::rendering::FD3D11States::CreateSamplers(ID3D11Device *pd3dDevice) {
	HRESULT hr;

	// Point Clamp
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	memset(&samplerDesc.BorderColor, 0, sizeof(samplerDesc.BorderColor));
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MaxLOD = FLT_MAX;
	hr = pd3dDevice->CreateSamplerState(&samplerDesc, &pSamplerPointClamp);
	if (FAILED(hr)) {
		return;
	}

	// Point Wrap
	samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	hr = (pd3dDevice->CreateSamplerState(&samplerDesc, &pSamplerPointWrap));
	if (FAILED(hr)) {
		return;
	}

	// Linear Clamp
	samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	hr = (pd3dDevice->CreateSamplerState(&samplerDesc, &pSamplerLinearClamp));
	if (FAILED(hr)) {
		return;
	}

	// Linear Wrap
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	hr = (pd3dDevice->CreateSamplerState(&samplerDesc, &pSamplerLinearWrap));
	if (FAILED(hr)) {
		return;
	}

	// Aniso Wrap
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 8;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MipLODBias = -0.4f;
	hr = (pd3dDevice->CreateSamplerState(&samplerDesc, &pSamplerAnisoWrap));
	if (FAILED(hr)) {
		return;
	}

	// Aniso Clamp
	samplerDesc.AddressU = samplerDesc.AddressV = samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	hr = (pd3dDevice->CreateSamplerState(&samplerDesc, &pSamplerAnisoClamp));
	if (FAILED(hr)) {
		return;
	}


	// Linear Comparison
	// Sampler States
	//samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;	// by³o clamp w xyz
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = -0.4f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	memset(&samplerDesc.BorderColor, 0, sizeof(samplerDesc.BorderColor));
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = FLT_MAX;
	hr = (pd3dDevice->CreateSamplerState(&samplerDesc, &pSamplerComparisonLinear));
	if (FAILED(hr)) {
		return;
	}
}

void framework::rendering::FD3D11States::CreateRasterizerStates(ID3D11Device *pd3dDevice) {
	HRESULT hr;

	// Rasterizer state with backface culling
	D3D11_RASTERIZER_DESC rasterizerState;
	rasterizerState.FillMode = D3D11_FILL_SOLID;
	rasterizerState.CullMode = D3D11_CULL_BACK;
	rasterizerState.FrontCounterClockwise = FALSE;
	rasterizerState.DepthBias = FALSE;
	rasterizerState.DepthBiasClamp = 0;
	rasterizerState.SlopeScaledDepthBias = 0;
	rasterizerState.DepthClipEnable = FALSE;
	rasterizerState.ScissorEnable = FALSE;
	// With D3D 11, MultisampleEnable has no effect when rasterizing triangles:
	// MSAA rasterization is implicitely enabled as soon as the render target is MSAA.
	rasterizerState.MultisampleEnable = FALSE;
	rasterizerState.AntialiasedLineEnable = FALSE;
	hr = (pd3dDevice->CreateRasterizerState(&rasterizerState, &pBackfaceCull_RS));
	if (FAILED(hr)) {
		return;
	}

	rasterizerState.MultisampleEnable = TRUE;
	hr = (pd3dDevice->CreateRasterizerState(&rasterizerState, &pBackfaceCullMSAA_RS));
	if (FAILED(hr)) {
		return;
	}


	// Wireframe
	rasterizerState.MultisampleEnable = FALSE;
	rasterizerState.CullMode = D3D11_CULL_NONE;
	rasterizerState.FillMode = D3D11_FILL_WIREFRAME;
	hr = (pd3dDevice->CreateRasterizerState(&rasterizerState, &pBackfaceCull_WireFrame_RS));
	if (FAILED(hr)) {
		return;
	}

	rasterizerState.MultisampleEnable = TRUE;
	hr = (pd3dDevice->CreateRasterizerState(&rasterizerState, &pBackfaceCull_WireFrameMSAA_RS));
	if (FAILED(hr)) {
		return;
	}

	// Rasterizer state with no backface culling (for fullscreen passes)
	rasterizerState.MultisampleEnable = FALSE;
	rasterizerState.CullMode = D3D11_CULL_NONE;
	rasterizerState.FillMode = D3D11_FILL_SOLID;
	hr = (pd3dDevice->CreateRasterizerState(&rasterizerState, &pNoCull_RS));
	if (FAILED(hr)) {
		return;
	}
}

void framework::rendering::FD3D11States::CreateDepthStencilStates(ID3D11Device *pd3dDevice) {
	HRESULT hr;

	D3D11_DEPTH_STENCIL_DESC depthstencilState;
	depthstencilState.DepthEnable = TRUE;
	depthstencilState.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthstencilState.DepthFunc = D3D11_COMPARISON_LESS;
	depthstencilState.StencilEnable = FALSE;
	hr = (pd3dDevice->CreateDepthStencilState(&depthstencilState, &pDepthNoStencil_DS));
	if (FAILED(hr)) {
		return;
	}

	depthstencilState.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = (pd3dDevice->CreateDepthStencilState(&depthstencilState, &pDS_Skybox));
	if (FAILED(hr)) {
		return;
	}

	depthstencilState.DepthFunc = D3D11_COMPARISON_GREATER;
	hr = (pd3dDevice->CreateDepthStencilState(&depthstencilState, &pDS_DepthGreater));
	if (FAILED(hr)) {
		return;
	}



	depthstencilState.DepthFunc = D3D11_COMPARISON_LESS;
	depthstencilState.DepthEnable = FALSE;
	hr = (pd3dDevice->CreateDepthStencilState(&depthstencilState, &pNoDepthNoStencil_DS));
	if (FAILED(hr)) {
		return;
	}

	//------------------------------------------------------------------------
	// For stencil buffer - settings bits
	//------------------------------------------------------------------------

	// Disable depth & enable stencil
	depthstencilState.DepthEnable = FALSE;
	depthstencilState.StencilEnable = TRUE;

	// Read & write all bits
	depthstencilState.StencilReadMask = 0xff;
	depthstencilState.StencilWriteMask = 0xff;

	// Stencil operator for front face
	depthstencilState.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthstencilState.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthstencilState.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthstencilState.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;

	// Stencil operator for back face.
	depthstencilState.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthstencilState.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthstencilState.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthstencilState.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	hr = (pd3dDevice->CreateDepthStencilState(&depthstencilState, &pStencilSetBits_DS));
	if (FAILED(hr)) {
		return;
	}

	depthstencilState.DepthEnable = FALSE;
	depthstencilState.StencilEnable = TRUE;
	depthstencilState.StencilReadMask = 0xff;
	depthstencilState.StencilWriteMask = 0xff;
	depthstencilState.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	depthstencilState.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthstencilState.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthstencilState.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthstencilState.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	depthstencilState.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthstencilState.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthstencilState.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	hr = (pd3dDevice->CreateDepthStencilState(&depthstencilState, &pNoDepthStencilComplex_DS));
	if (FAILED(hr)) {
		return;
	}

	depthstencilState.DepthEnable = TRUE;
	depthstencilState.StencilEnable = TRUE;
	depthstencilState.StencilReadMask = 0xff;
	depthstencilState.StencilWriteMask = 0xff;
	depthstencilState.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	depthstencilState.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthstencilState.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthstencilState.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthstencilState.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
	depthstencilState.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthstencilState.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthstencilState.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	hr = (pd3dDevice->CreateDepthStencilState(&depthstencilState, &pDepthStencilComplex_DS));
	if (FAILED(hr)) {
		return;
	}
}

void framework::rendering::FD3D11States::CreateBlendStates(ID3D11Device *pd3dDevice) {
	HRESULT hr;

	// Blending state with blending disabled
	D3D11_BLEND_DESC blendState;
	blendState.AlphaToCoverageEnable = FALSE;
	blendState.IndependentBlendEnable = FALSE; // new in D3D11
	for (int i = 0; i < 8; ++i) {
		blendState.RenderTarget[i].BlendEnable = FALSE;
		blendState.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}
	hr = (pd3dDevice->CreateBlendState(&blendState, &pNoBlend_BS));
	if (FAILED(hr)) {
		return;
	}

	for (int i = 0; i < 8; ++i) {
		blendState.RenderTarget[i].BlendEnable = TRUE;
		blendState.RenderTarget[i].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendState.RenderTarget[i].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendState.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		blendState.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendState.RenderTarget[i].SrcBlendAlpha = blendState.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
	}
	hr = (pd3dDevice->CreateBlendState(&blendState, &pBlend_BS));
	if (FAILED(hr)) {
		return;
	}

	// Additive blend
	for (int i = 0; i < 8; ++i) {
		blendState.RenderTarget[i].BlendEnable = TRUE;
		blendState.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
		blendState.RenderTarget[i].DestBlend = D3D11_BLEND_ONE;
		blendState.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
		blendState.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendState.RenderTarget[i].SrcBlendAlpha = blendState.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ONE;
	}
	hr = (pd3dDevice->CreateBlendState(&blendState, &pBlendAdditive_BS));
	if (FAILED(hr)) {
		return;
	}

	blendState.AlphaToCoverageEnable = TRUE;
	hr = (pd3dDevice->CreateBlendState(&blendState, &pBlendAlphaToCoverage_BS));
	if (FAILED(hr)) {
		return;
	}
}
