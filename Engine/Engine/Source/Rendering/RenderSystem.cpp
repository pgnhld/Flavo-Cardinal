#include "Rendering/RenderSystem.h"
#include "Rendering/Camera.h"
#include "Rendering/SkinnedMeshRenderer.h"
#include "Rendering/PointLight.h"
#include "Rendering/DirectionalLight.h"
#include "Rendering/CylinderLight.h"
#include "Rendering/StaticMeshRenderer.h"
#include "Physics/Transform.h"
#include "EntityManager.h"
#include "DxtkString.h"
#include "imgui.h"
#include "FWindow.h"
#include "EngineEvent.h"
#include "FTime.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "FInput.h"
#include "Logger.h"
#include "imgui_impl_dx11.h"
#include "SceneManager.h"
#include "FlavoRootsGame/Player.h"
#include "FlavoRootsGame/SceneSpecificData.h"

// Prefer discrete GPU on switchable GPU systems
extern "C"
{
	__declspec(dllexport) DWORD		NvOptimusEnablement = 1;
	__declspec(dllexport) int		AmdPowerXpressRequestHighPerformance = 1;
}


ft_render::RenderSystem::RenderSystem() {
	subscribe<EventPostSceneLoaded>(this, &RenderSystem::onPostSceneLoaded);

	framework::FRendererD3D11& renderer = framework::FRendererD3D11::getInstance();
	const uint32 Width = renderer.getRendererWidth();
	const uint32 Height = renderer.getRendererHeight();

	gBuffer_.initialize(Width, Height);

	FullscreenPass::getInstance().initialize();


	eyeAdaptation_[0].initialize(Width, Height, 9);

	// setup viewports
	const float halfWidth = static_cast<float>(Width) / 2.0f;

	if (FWindow::getInstance().isEditorWindow()) {
		renderViewports_[0].TopLeftX = 0.f;
		renderViewports_[0].TopLeftY = 0.f;
		renderViewports_[0].MinDepth = 0.f;
		renderViewports_[0].MaxDepth = 1.f;
		renderViewports_[0].Width = static_cast<float>(Width);
		renderViewports_[0].Height = static_cast<float>(Height);

		numPlayersViewports = 1;
	} else {
		renderViewports_[0].TopLeftX = 0.f;
		renderViewports_[0].TopLeftY = 0.f;
		renderViewports_[0].MinDepth = 0.f;
		renderViewports_[0].MaxDepth = 1.f;
		renderViewports_[0].Width = halfWidth;
		renderViewports_[0].Height = static_cast<float>(Height);

		renderViewports_[1].TopLeftX = halfWidth;
		renderViewports_[1].TopLeftY = 0.f;
		renderViewports_[1].MinDepth = 0.f;
		renderViewports_[1].MaxDepth = 1.f;
		renderViewports_[1].Width = halfWidth;
		renderViewports_[1].Height = static_cast<float>(Height);

		numPlayersViewports = 2;
		eyeAdaptation_[1].initialize(Width, Height, 9);
	}

	ID3D11Device* dev = renderer.getD3D11Device();
	ID3D11DeviceContext* dc = renderer.getD3D11DeviceContext();

	// This "normals fitting" texture is loaded just once, no need to load it
	// using ResourceManager.
	texNormalsFitting_.load(dev, dc, "..//Data//Textures//Engine//normalsFit.dds");

	// *** Create shaders ***
	if (FWindow::getInstance().isEditorWindow()) {

		ID3DBlob* blobPSLogLumFullscreen = nullptr;
		if (FRendererD3D11::compileD3DShader(L"..//Data//Shaders//Luminance.hlsl", "PSLogLuminanceFullscreen", "ps_5_0", &blobPSLogLumFullscreen)) {
			dev->CreatePixelShader((const void*)blobPSLogLumFullscreen->GetBufferPointer(), blobPSLogLumFullscreen->GetBufferSize(), nullptr, &pixelShaderFinalPass_);
		}
		SAFE_RELEASE(blobPSLogLumFullscreen);

	} else {

		ID3DBlob* blobPSLogLumSplitLeft = nullptr;
		ID3DBlob* blobPSLogLumSplitRight = nullptr;

		if (FRendererD3D11::compileD3DShader(L"..//Data//Shaders//Luminance.hlsl", "PSLogLuminanceSplitscreenLeft", "ps_5_0", &blobPSLogLumSplitLeft)) {
			dev->CreatePixelShader((const void*)blobPSLogLumSplitLeft->GetBufferPointer(), blobPSLogLumSplitLeft->GetBufferSize(), nullptr, &pixelShaderLogLuminance_Split[0]);
		}
		SAFE_RELEASE(blobPSLogLumSplitLeft);


		if (FRendererD3D11::compileD3DShader(L"..//Data//Shaders//Luminance.hlsl", "PSLogLuminanceSplitscreenRight", "ps_5_0", &blobPSLogLumSplitRight)) {
			dev->CreatePixelShader((const void*)blobPSLogLumSplitRight->GetBufferPointer(), blobPSLogLumSplitRight->GetBufferSize(), nullptr, &pixelShaderLogLuminance_Split[1]);
		}
		SAFE_RELEASE(blobPSLogLumSplitRight);
	}



	ID3DBlob* blobVSGBuffer = nullptr;
	if (FRendererD3D11::compileD3DShader(L"..//Data//Shaders//GBuffer.hlsl", "RenderSceneToGBufferVS", "vs_5_0", &blobVSGBuffer)) {
		dev->CreateVertexShader((const void*)blobVSGBuffer->GetBufferPointer(), blobVSGBuffer->GetBufferSize(), nullptr, &gbufferFeedingVS_);
	}

	ID3DBlob* blobVSGBufferSkinned = nullptr;
	if (FRendererD3D11::compileD3DShader(L"..//Data//Shaders//GBuffer.hlsl", "RenderSceneToGBufferSkinnedVS", "vs_5_0", &blobVSGBufferSkinned)) {
		dev->CreateVertexShader((const void*)blobVSGBufferSkinned->GetBufferPointer(), blobVSGBufferSkinned->GetBufferSize(), nullptr, &gbufferFeedingSkinnedVS_);
	}

	ID3DBlob* blobPSGBuffer = nullptr;
	if (FRendererD3D11::compileD3DShader(L"..//Data//Shaders//GBuffer.hlsl", "RenderSceneToGBufferPS", "ps_5_0", &blobPSGBuffer)) {
		dev->CreatePixelShader((const void*)blobPSGBuffer->GetBufferPointer(), blobPSGBuffer->GetBufferSize(), nullptr, &gbufferFeedingPS_);
	}

	ID3DBlob* blobPSDeferred = nullptr;
	if (FRendererD3D11::compileD3DShader(L"..//Data//Shaders//DeferredShading.hlsl", "PSMain", "ps_5_0", &blobPSDeferred)) {
		dev->CreatePixelShader((const void*)blobPSDeferred->GetBufferPointer(), blobPSDeferred->GetBufferSize(), nullptr, &deferredShadingSimplePS_);
	}

	ID3DBlob* blobPSForward = nullptr;
	if (FRendererD3D11::compileD3DShader(L"..//Data//Shaders//ForwardPass.hlsl", "ForwardPassPS", "ps_5_0", &blobPSForward)) {
		dev->CreatePixelShader((const void*)blobPSForward->GetBufferPointer(), blobPSForward->GetBufferSize(), nullptr, &forwardPassSimplePS_);
	}

	ID3DBlob* blobVSDepthStatic = nullptr;
	if (FRendererD3D11::compileD3DShader(L"..//Data//Shaders//Shadows.hlsl", "RenderDepthStaticVS", "vs_5_0", &blobVSDepthStatic)) {
		dev->CreateVertexShader((const void*)blobVSDepthStatic->GetBufferPointer(), blobVSDepthStatic->GetBufferSize(), nullptr, &depthOnlyStaticMeshesVS_);
	}

	ID3DBlob* blobVSDepthSkinned = nullptr;
	if (FRendererD3D11::compileD3DShader(L"..//Data//Shaders//Shadows.hlsl", "RenderDepthSkinnedVS", "vs_5_0", &blobVSDepthSkinned)) {
		dev->CreateVertexShader((const void*)blobVSDepthSkinned->GetBufferPointer(), blobVSDepthSkinned->GetBufferSize(), nullptr, &depthOnlySkinnedMeshesVS_);
	}

	ID3DBlob* blobPSSimpleCopy = nullptr;
	if (FRendererD3D11::compileD3DShader(L"..//Data//Shaders//SimpleCopy.hlsl", "PSSimpleCopy", "ps_5_0", &blobPSSimpleCopy)) {
		dev->CreatePixelShader((const void*)blobPSSimpleCopy->GetBufferPointer(), blobPSSimpleCopy->GetBufferSize(), nullptr, &pixelShaderSimpleCopy_);
	}

	ID3DBlob* blobPSTonemapping = nullptr;
	if (FRendererD3D11::compileD3DShader(L"..//Data//Shaders//Tonemapping.hlsl", "PSTonemapping", "ps_5_0", &blobPSTonemapping)) {
		dev->CreatePixelShader((const void*)blobPSTonemapping->GetBufferPointer(), blobPSTonemapping->GetBufferSize(), nullptr, &pixelShaderTonemapping_);
	}

	ID3DBlob* blobPSFinalPostProcessPass = nullptr;
	if (FRendererD3D11::compileD3DShader(L"..//Data//Shaders//PostProcessFinal.hlsl", "PSFinalPostProcess", "ps_5_0", &blobPSFinalPostProcessPass)) {
		dev->CreatePixelShader((const void*)blobPSFinalPostProcessPass->GetBufferPointer(), blobPSFinalPostProcessPass->GetBufferSize(), nullptr, &pixelShaderFinalPass_);
	}

	ID3DBlob* blobPSBloomTreshold = nullptr;
	if (FRendererD3D11::compileD3DShader(L"..//Data//Shaders//BloomThreshold.hlsl", "PSBloomThreshold", "ps_5_0", &blobPSBloomTreshold)) {
		dev->CreatePixelShader((const void*)blobPSBloomTreshold->GetBufferPointer(), blobPSBloomTreshold->GetBufferSize(), nullptr, &bloomThresholdPS);
	}

	ID3DBlob* blobPSGaussianBlurH = nullptr;
	if (FRendererD3D11::compileD3DShader(L"..//Data//Shaders//BloomBlur.hlsl", "BloomBlurH", "ps_5_0", &blobPSGaussianBlurH)) {
		dev->CreatePixelShader((const void*)blobPSGaussianBlurH->GetBufferPointer(), blobPSGaussianBlurH->GetBufferSize(), nullptr, &bloomGaussianBlurHPS);
	}

	ID3DBlob* blobPSGaussianBlurV = nullptr;
	if (FRendererD3D11::compileD3DShader(L"..//Data//Shaders//BloomBlur.hlsl", "BloomBlurV", "ps_5_0", &blobPSGaussianBlurV)) {
		dev->CreatePixelShader((const void*)blobPSGaussianBlurV->GetBufferPointer(), blobPSGaussianBlurV->GetBufferSize(), nullptr, &bloomGaussianBlurVPS);
	}

	ID3DBlob* blobPSForwardHologram = nullptr;
	if (FRendererD3D11::compileD3DShader(L"..//Data//Shaders//ForwardPassHologram.hlsl", "ForwardPassHologramPS", "ps_5_0", &blobPSForwardHologram)) {
		dev->CreatePixelShader((const void*)blobPSForwardHologram->GetBufferPointer(), blobPSForwardHologram->GetBufferSize(), nullptr, &forwardPassHologramPS_);
	}

	ID3DBlob* blobPSWater = nullptr;
	if (FRendererD3D11::compileD3DShader(L"..//Data//Shaders//Water.hlsl", "WaterPS", "ps_5_0", &blobPSWater)) {
		dev->CreatePixelShader((const void*)blobPSWater->GetBufferPointer(), blobPSWater->GetBufferSize(), nullptr, &waterPS_);
	}
	ID3DBlob* blobPSFullscreenGlitch = nullptr;
	if (FRendererD3D11::compileD3DShader(L"..//Data//Shaders//FullscreenGlitch.hlsl", "FullscreenGlitchPS", "ps_5_0", &blobPSFullscreenGlitch)) {
		dev->CreatePixelShader((const void*)blobPSFullscreenGlitch->GetBufferPointer(), blobPSFullscreenGlitch->GetBufferSize(), nullptr, &fullscreenGlitchPS_);
	}

	ID3DBlob* blobPSLensFlares = nullptr;
	if (FRendererD3D11::compileD3DShader(L"..//Data//Shaders//LensFlares.hlsl", "LensFlaresPS", "ps_5_0", &blobPSLensFlares)) {
		dev->CreatePixelShader((const void*)blobPSLensFlares->GetBufferPointer(), blobPSLensFlares->GetBufferSize(), nullptr, &lensFlaresPS);
	}

	ID3DBlob* blobPSLensFlaresThreshold = nullptr;
	if (FRendererD3D11::compileD3DShader(L"..//Data//Shaders//LensFlares.hlsl", "LensFlaresThresholdPS", "ps_5_0", &blobPSLensFlaresThreshold)) {
		dev->CreatePixelShader((const void*)blobPSLensFlaresThreshold->GetBufferPointer(), blobPSLensFlaresThreshold->GetBufferSize(), nullptr, &lensFlaresThresholdPS);
	}

	ID3DBlob* blobPSAntialiasing = nullptr;
	if (FRendererD3D11::compileD3DShader(L"..//Data//Shaders//AA.hlsl", "AntialiasingPS", "ps_5_0", &blobPSAntialiasing)) {
		dev->CreatePixelShader((const void*)blobPSAntialiasing->GetBufferPointer(), blobPSAntialiasing->GetBufferSize(), nullptr, &fxaaPS);
	}

	//ao_.initialize(Width, Height);

	// Input layout for non-skinned geometry
	const D3D11_INPUT_ELEMENT_DESC inputLayoutStaticGeo[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,		0, 12,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,		1, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT,		1, 12,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BITANGENT",	0, DXGI_FORMAT_R32G32B32_FLOAT,		1, 24,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE(inputLayoutStaticGeo);
	dev->CreateInputLayout(inputLayoutStaticGeo, numElements, (const void*)blobVSGBuffer->GetBufferPointer(),
		blobVSGBuffer->GetBufferSize(), &inputLayoutStaticGeometry_);


	const D3D11_INPUT_ELEMENT_DESC inputLayoutSkinnedGeo[] =
	{
		{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,			0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD",		0, DXGI_FORMAT_R32G32_FLOAT,			0, 12,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",			0, DXGI_FORMAT_R32G32B32_FLOAT,			1, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT",		0, DXGI_FORMAT_R32G32B32_FLOAT,			1, 12,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BITANGENT",		0, DXGI_FORMAT_R32G32B32_FLOAT,			1, 24,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES",	0, DXGI_FORMAT_R32G32B32A32_UINT,		2, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHTS",	0, DXGI_FORMAT_R32G32B32A32_FLOAT,		2, 16,	D3D11_INPUT_PER_VERTEX_DATA, 0 }

	};
	numElements = ARRAYSIZE(inputLayoutSkinnedGeo);
	dev->CreateInputLayout(inputLayoutSkinnedGeo, numElements, (const void*)blobVSGBufferSkinned->GetBufferPointer(),
		blobVSGBufferSkinned->GetBufferSize(), &inputLayoutSkinnedGeometry_);


	SAFE_RELEASE(blobVSGBuffer);
	SAFE_RELEASE(blobVSGBufferSkinned);
	SAFE_RELEASE(blobPSGBuffer);
	SAFE_RELEASE(blobPSDeferred);
	SAFE_RELEASE(blobPSForward);
	SAFE_RELEASE(blobVSDepthStatic);
	SAFE_RELEASE(blobVSDepthSkinned);
	SAFE_RELEASE(blobPSSimpleCopy);
	SAFE_RELEASE(blobPSTonemapping);
	SAFE_RELEASE(blobPSFinalPostProcessPass);
	SAFE_RELEASE(blobPSForwardHologram);
	SAFE_RELEASE(blobPSWater);
	SAFE_RELEASE(blobPSFullscreenGlitch);
	SAFE_RELEASE(blobPSLensFlares);
	SAFE_RELEASE(blobPSLensFlaresThreshold);
	SAFE_RELEASE(blobPSAntialiasing);

	SAFE_RELEASE(blobPSBloomTreshold);
	SAFE_RELEASE(blobPSGaussianBlurH);
	SAFE_RELEASE(blobPSGaussianBlurV);

	// Render states
	{
		D3D11_BLEND_DESC blendDesc = {};
		blendDesc.AlphaToCoverageEnable = FALSE;
		blendDesc.IndependentBlendEnable = FALSE;

		D3D11_RENDER_TARGET_BLEND_DESC& rtbd = blendDesc.RenderTarget[0];
		rtbd.BlendEnable = TRUE;
		rtbd.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		// destination: current RGB value in render target
		// source: RGB value that pixel shader outputs
		rtbd.BlendOp = D3D11_BLEND_OP_ADD;
		rtbd.DestBlend = D3D11_BLEND_INV_BLEND_FACTOR;
		rtbd.SrcBlend = D3D11_BLEND_BLEND_FACTOR;

		rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
		rtbd.DestBlendAlpha = D3D11_BLEND_ONE;
		rtbd.SrcBlendAlpha = D3D11_BLEND_ZERO;
		dev->CreateBlendState(&blendDesc, &blendStateForwardPassTransparency_);

		rtbd.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		rtbd.SrcBlend = D3D11_BLEND_SRC_ALPHA;
		dev->CreateBlendState(&blendDesc, &blendStateForwardPassTransparencyFromAlpha_);
	}

	{
		D3D11_RASTERIZER_DESC rsDesc = { };
		rsDesc.AntialiasedLineEnable = FALSE;
		rsDesc.CullMode = D3D11_CULL_BACK;
		rsDesc.FillMode = D3D11_FILL_SOLID;
		rsDesc.DepthBias = 0;
		rsDesc.SlopeScaledDepthBias = 3.00f;
		rsDesc.DepthBiasClamp = 0.0f;
		rsDesc.ScissorEnable = FALSE;
		rsDesc.MultisampleEnable = FALSE;
		rsDesc.DepthClipEnable = TRUE;
		rsDesc.FrontCounterClockwise = FALSE;

		rasterizerStateShadows = nullptr;
		dev->CreateRasterizerState(&rsDesc, &rasterizerStateShadows);
	}

	{
		D3D11_DEPTH_STENCIL_DESC dsDesc = { };
		dsDesc.DepthEnable = TRUE;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;		// disable writing to depth buffer
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
		dsDesc.StencilEnable = FALSE;
		dev->CreateDepthStencilState(&dsDesc, &depthStencilState_NoDepthWrite);

		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		dev->CreateDepthStencilState(&dsDesc, &depthStencilState_Skybox);
	}


	// Create D3D textures and buffers
	hdrColorBuffer_.initialize(dev, Width, Height, DXGI_FORMAT_R16G16B16A16_FLOAT, false);
	colorBufferAfterTonemapping_.initialize(dev, Width, Height, DXGI_FORMAT_R11G11B10_FLOAT, false);
	cbufferLighting_.Create(dev);
	cbufferPerFrame_.Create(dev);
	cbufferPostprocess_.Create(dev);

	bloomThresholdFullOutput.initialize(dev, Width, Height, DXGI_FORMAT_R11G11B10_FLOAT);

	bloomColorBuffer_half[0].initialize(dev, Width / 2, Height / 2, DXGI_FORMAT_R11G11B10_FLOAT);
	bloomColorBuffer_half[1].initialize(dev, Width / 2, Height / 2, DXGI_FORMAT_R11G11B10_FLOAT);

	lensFlaresTex[0].initialize(dev, Width / 4, Height / 4, DXGI_FORMAT_R11G11B10_FLOAT);
	lensFlaresTex[1].initialize(dev, Width / 4, Height / 4, DXGI_FORMAT_R11G11B10_FLOAT);

	// Shadow map, 32bit float
	shadowMap_.initialize(dev, shadowMapResolution_, shadowMapResolution_, DXGI_FORMAT_D32_FLOAT, true);

	skybox_.initialize();
}

ft_render::RenderSystem::~RenderSystem() {
	unsubscribe<EventPostSceneLoaded>();

	gBuffer_.cleanup();
	FullscreenPass::getInstance().cleanup();

	eyeAdaptation_[0].release();
	eyeAdaptation_[1].release();

	lensFlaresTex[0].cleanup();
	lensFlaresTex[1].cleanup();

	skybox_.release();

	bloomThresholdFullOutput.cleanup();

	bloomColorBuffer_half[0].cleanup();
	bloomColorBuffer_half[1].cleanup();

	SAFE_RELEASE(gbufferFeedingVS_);
	SAFE_RELEASE(gbufferFeedingSkinnedVS_);
	SAFE_RELEASE(gbufferFeedingPS_);
	SAFE_RELEASE(deferredShadingSimplePS_);
	SAFE_RELEASE(forwardPassSimplePS_);

	SAFE_RELEASE(inputLayoutStaticGeometry_);
	SAFE_RELEASE(inputLayoutSkinnedGeometry_);

	SAFE_RELEASE(depthStencilState_NoDepthWrite);
	SAFE_RELEASE(depthStencilState_Skybox);

	SAFE_RELEASE(depthOnlyStaticMeshesVS_);
	SAFE_RELEASE(depthOnlySkinnedMeshesVS_);
	SAFE_RELEASE(blendStateForwardPassTransparency_);
	SAFE_RELEASE(rasterizerStateShadows);
	SAFE_RELEASE(blendStateForwardPassTransparencyFromAlpha_);

	SAFE_RELEASE(pixelShaderSimpleCopy_);
	SAFE_RELEASE(pixelShaderTonemapping_);
	SAFE_RELEASE(pixelShaderFinalPass_);

	SAFE_RELEASE(pixelShaderLogLuminance_Split[0]);
	SAFE_RELEASE(pixelShaderLogLuminance_Split[1]);
	SAFE_RELEASE(pixelShaderLogLuminance_Full);
	SAFE_RELEASE(forwardPassHologramPS_);
	SAFE_RELEASE(waterPS_);
	SAFE_RELEASE(fullscreenGlitchPS_);
	SAFE_RELEASE(lensFlaresPS);
	SAFE_RELEASE(lensFlaresThresholdPS);
	SAFE_RELEASE(fxaaPS);

	SAFE_RELEASE(bloomThresholdPS);
	SAFE_RELEASE(bloomGaussianBlurVPS);
	SAFE_RELEASE(bloomGaussianBlurHPS);

	ao_.release();

	texNormalsFitting_.cleanup();
	hdrColorBuffer_.cleanup();
	colorBufferAfterTonemapping_.cleanup();
	shadowMap_.cleanup();

	cbufferLighting_.Release();
	cbufferPerFrame_.Release();
	cbufferPostprocess_.Release();
}

void ft_render::RenderSystem::update(eecs::EntityManager& entities, double deltaTime) {
	// Update per-frame stats

	//OutputDebugString( std::to_string(deltaTime).c_str() );
	//OutputDebugString( "\n" );

	if (deltaTime < 2.0) {
		elapsedTime += static_cast<float>(deltaTime);
		numFrames++;
	}

	framework::FRendererD3D11& renderer = framework::FRendererD3D11::getInstance();
	const uint32 Width = renderer.getRendererWidth();
	const uint32 Height = renderer.getRendererHeight();
	ID3D11DeviceContext* pDevCon = renderer.getD3D11DeviceContext();




	renderer.clearBackbuffer();
	renderer.setAllSamplers();

	// Update per-frame constant buffer
	{
		cbufferPerFrame_.GetBufferData().fElapsedTime = elapsedTime;
		cbufferPerFrame_.GetBufferData().viewportSize.x = (float)Width;
		cbufferPerFrame_.GetBufferData().viewportSize.y = (float)Height;
		cbufferPerFrame_.GetBufferData().viewportSize.z = 1.0f / (float)Width;
		cbufferPerFrame_.GetBufferData().viewportSize.w = 1.0f / (float)Height;

		cbufferPerFrame_.UpdateBuffer(pDevCon);
		cbufferPerFrame_.SetPS(pDevCon, 3);
	}

	pDevCon->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	pDevCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Update scene specific data
	if (requestSceneSpecificDataUpdate_) {

		eecs::Entity ssd = entities.getEntityWithComponents<ft_game::SceneSpecificData>();
		if (ssd.isValid()) {
			ft_game::SceneSpecificData* pSSD = ssd.getComponent<ft_game::SceneSpecificData>().get();

			auto& cbData = cbufferPostprocess_.GetBufferData();

			cbData.tonemappingCurveABCD.x = pSSD->postprocess.tonemappingCurveA_ShoulderStrength_;
			cbData.tonemappingCurveABCD.y = pSSD->postprocess.tonemappingCurveB_LinearStrength_;
			cbData.tonemappingCurveABCD.z = pSSD->postprocess.tonemappingCurveC_LinearAngle_;
			cbData.tonemappingCurveABCD.w = pSSD->postprocess.tonemappingCurveD_ToeStrength_;
			cbData.tonemappingCurveEF.x = pSSD->postprocess.tonemappingCurveE_ToeNumerator_;
			cbData.tonemappingCurveEF.y = pSSD->postprocess.tonemappingCurveF_ToeDenominator_;
			cbData.tonemappingExposureExponent = pSSD->postprocess.tonemappingExposureExponent_;
			cbData.tonemappingNominatorMultiplier = pSSD->postprocess.tonemappingNominatorMultiplier_;
			cbData.tonemappingWhitePointScale = pSSD->postprocess.tonemappingWhitePointScale_;

			cbData.eyeAdaptationMinAllowedLuminance = pSSD->postprocess.eyeAdaptationMinAllowedLuminance_;
			cbData.eyeAdaptationMaxAllowedLuminance = pSSD->postprocess.eyeAdaptationMaxAllowedLuminance_;

			cbData.vignetteIntensity = pSSD->postprocess.vignetteIntensity_;
			cbData.vignetteColor = Vector3( (const float*) &pSSD->postprocess.vignetteColor_ );
			cbData.vignetteWeights = Vector3( (const float*) &pSSD->postprocess.vignetteWeights_ );

			cbData.chromaticAberrationCenter = Vector2( (const float*) &pSSD->postprocess.chromaticAberrationCenter_ );
			cbData.chromaticAberrationIntensity = pSSD->postprocess.chromaticAberrationIntensity_;
			cbData.chromaticAberrationRange = pSSD->postprocess.chromaticAberrationRange_;
			cbData.chromaticAberrationSize = pSSD->postprocess.chromaticAberrationSize_;
			cbData.chromaticAberrationStart = pSSD->postprocess.chromaticAberrationStart_;

			cbData.bloomBlurSigma = pSSD->postprocess.bloomBlurSigma_;
			cbData.bloomThreshold = pSSD->postprocess.bloomThreshold_;
			cbData.bloomMultiplier = pSSD->postprocess.bloomMultiplier_;

			cbufferPostprocess_.UpdateBuffer(pDevCon);
			cbufferPostprocess_.SetPS(pDevCon, 13);		

			requestSceneSpecificDataUpdate_ = false;


			if (FWindow::getInstance().isEditorWindow()) {
				requestSceneSpecificDataUpdate_ = true;
			}
		}
	}

	// prepare rendering here (set shaders, vertex layout)
	std::vector<eecs::Entity> e = entities.getEntitiesWithComponents<ft_render::Camera, ft_engine::Transform>();
	const uint32 numCameras = (uint32)e.size();

	std::vector<eecs::Entity> meshes = entities.getEntitiesWithComponents<ft_render::StaticMeshRenderer, ft_engine::Transform>();
	const uint32 staticMeshesCount = (uint32)meshes.size();

	std::vector<eecs::Entity> meshesSkinned = entities.getEntitiesWithComponents<ft_render::SkinnedMeshRenderer, ft_engine::Transform>();
	const uint32 skinnedMeshesCount = (uint32)meshesSkinned.size();

	// update animation times
	for (uint32 iSkinnedMesh = 0; iSkinnedMesh < skinnedMeshesCount; iSkinnedMesh++) {
		ft_render::SkinnedMeshRenderer* skinnedmesh = meshesSkinned[iSkinnedMesh].getComponent<ft_render::SkinnedMeshRenderer>().get();

		skinnedmesh->animationSliceTime_ += (float)deltaTime;
	}


	setupLighting(entities);

	if (isDirLightPresent_) {
		renderShadows(meshes, staticMeshesCount, meshesSkinned, skinnedMeshesCount);
	}

	prepareDeferredShadingPass();

	const bool bEditor = FWindow::getInstance().isEditorWindow();
	for (uint32 i = 0; i < numCameras; i++) {
		Camera* cam = e[i].getComponent<Camera>().get();
		ft_engine::Transform* xform = e[i].getComponent<ft_engine::Transform>().get();

		if (bEditor) {
			if (!cam->bEnabled)
				continue;
			moveCamera(cam, xform);
			invoke<EventTransformUpdate>();
			renderer.setRenderViewport(renderViewports_[0]);
		} else {
			// render to proper viewport
			renderer.setRenderViewport(renderViewports_[i]);
		}


		// *** for non-skinned geometry
		//renderer.renderSimple((float)deltaTime);
		pDevCon->VSSetShader(gbufferFeedingVS_, nullptr, 0);
		pDevCon->PSSetShader(gbufferFeedingPS_, nullptr, 0);
		pDevCon->IASetInputLayout(inputLayoutStaticGeometry_);

		// calculate player view/proj matrix
		const Matrix matProj = DirectX::XMMatrixPerspectiveFovLH(DEG2RAD(cam->fovDegrees),
			cam->aspectRatio,
			cam->nearPlane,
			cam->farPlane);

		const Matrix matView = DirectX::XMMatrixLookAtLH(xform->getWorldPosition(),
			xform->getWorldPosition() - xform->getWorldForward(),
			xform->getWorldUp());

		// Update players world positions
		eyePosPlayers_[i] = xform->getWorldPosition();

		DirectX::BoundingFrustum boundingFrustrum = DirectX::BoundingFrustum(matProj);
		Matrix inverseViewMatrix = matView.Invert();
		boundingFrustrum.Transform(boundingFrustrum, inverseViewMatrix);

		renderer.updateViewConstantBuffer(xform->getWorldPosition(), matView, matProj);

		//used to verify which meshes to render
		ft_engine::Player* cameraPlayer = e[i].getComponent<ft_engine::Player>().get();
		for (uint32 meshIndex = 0; meshIndex < staticMeshesCount; meshIndex++) {
			StaticMeshRenderer* staticmesh = meshes[meshIndex].getComponent<ft_render::StaticMeshRenderer>().get();
			if (meshes[meshIndex].hasComponent<ft_engine::Player>()) {
				ft_engine::Player* meshPlayer = meshes[meshIndex].getComponent<ft_engine::Player>().get();
				if (meshPlayer->bLocal == cameraPlayer->bLocal) {
					if (!staticmesh->bEnabledOwn)
						continue;
				} else {
					if (!staticmesh->bEnabledOther)
						continue;
				}
			} else {
				if (!staticmesh->bEnabledOwn)
					continue;
			}

			if (staticmesh->getMaterial().colorTint.A() < 0.9999f) {
				continue;
			}

			ft_engine::Transform* currMeshTransform = meshes[meshIndex].getComponent<ft_engine::Transform>().get();
			DirectX::BoundingBox transformedBoundingBox = currMeshTransform->getTransformedBoundingBox();
			if (!checkIntersection(boundingFrustrum, transformedBoundingBox)) {
				continue;
			}

			const Matrix matWorld = currMeshTransform->getWorldTransform();
			const FMaterial& meshMaterial = staticmesh->material_;
			renderer.updateStaticObjectConstantBuffer(matWorld, meshMaterial.uvTiling, meshMaterial.uvOffset, meshMaterial.colorTint.ToVector3(), meshMaterial.specialEffect, meshMaterial.smoothness);
			staticmesh->render();
		}

		// *** for skinned geometry
		pDevCon->PSSetShader(gbufferFeedingPS_, nullptr, 0);
		pDevCon->IASetInputLayout(inputLayoutSkinnedGeometry_);
		for (uint32 skinnedmeshIndex = 0; skinnedmeshIndex < skinnedMeshesCount; skinnedmeshIndex++) {
			SkinnedMeshRenderer* skinnedmesh = meshesSkinned[skinnedmeshIndex].getComponent<SkinnedMeshRenderer>().get();
			if (meshesSkinned[skinnedmeshIndex].hasComponent<ft_engine::Player>()) {
				ft_engine::Player* meshPlayer = meshesSkinned[skinnedmeshIndex].getComponent<ft_engine::Player>().get();
				if (meshPlayer->bLocal == cameraPlayer->bLocal) {
					if (!skinnedmesh->bEnabledOwn)
						continue;
				} else {
					if (!skinnedmesh->bEnabledOther)
						continue;
				}
			} else {
				if (!skinnedmesh->bEnabledOwn)
					continue;
			}

			ft_engine::Transform* xform = meshesSkinned[skinnedmeshIndex].getComponent<ft_engine::Transform>().get();
			DirectX::SimpleMath::Matrix matWorld = xform->getWorldTransform();
			if (!checkIntersection(boundingFrustrum, xform->getTransformedBoundingBox())) {
				continue;
			}

			Matrix44 matWorld2;
			memcpy(&matWorld2, &matWorld, 16 * sizeof(float));
			const FMaterial& meshMaterial = skinnedmesh->material_;
			renderer.updateStaticObjectConstantBuffer(matWorld2, meshMaterial.uvTiling, meshMaterial.uvOffset, meshMaterial.colorTint.ToVector3(), meshMaterial.specialEffect, meshMaterial.smoothness);

			if (skinnedmesh->bAnimationEnabled) {
				std::vector<Matrix> bones;
				skinnedmesh->boneTransform(bones);

				const uint32 numBones = bones.size();
				for (uint32 ij = 0; ij < numBones; ij++) {
					renderer.cbufferSkinning_.GetBufferData().bones[ij] = bones[ij];
				}

				pDevCon->VSSetShader(gbufferFeedingSkinnedVS_, nullptr, 0);
				pDevCon->IASetInputLayout(inputLayoutSkinnedGeometry_);

				renderer.cbufferSkinning_.SetVS(pDevCon, 2);
				renderer.cbufferSkinning_.UpdateBuffer(pDevCon);

				skinnedmesh->render();
			} else {
				pDevCon->VSSetShader(gbufferFeedingVS_, nullptr, 0);
				pDevCon->IASetInputLayout(inputLayoutStaticGeometry_);

				skinnedmesh->renderNoSkinning();
			}

		}
	}

	// *** Deferred Shading ***
	// Output: HDR buffer
	gBuffer_.unbind();

	//ao_.renderSSAO( gBuffer_.rt3_, gBuffer_.rt1_ );

	renderer.updateViewConstantBufferMultiCamera(eyePosPlayers_[0], eyePosPlayers_[1]);

	ID3D11ShaderResourceView* GBufferSRV[7] = {
		gBuffer_.ds_.m_SRV,
		gBuffer_.rt0_.srv_,
		gBuffer_.rt1_.srv_,
		gBuffer_.rt2_.srv_,
		gBuffer_.rt3_.srv_,
		ao_.getSSAOTexture().srv_,
		shadowMap_.m_SRV
	};
	pDevCon->PSSetShaderResources(0, 7, GBufferSRV);

	pDevCon->OMSetRenderTargets(1, &hdrColorBuffer_.rtv_, nullptr);
	pDevCon->PSSetShader(deferredShadingSimplePS_, nullptr, 0);
	FullscreenPass::getInstance().renderFullscreen(Width, Height);


	/*

	pDevCon->OMSetRenderTargets(1, &backbuffer, nullptr);
	pDevCon->PSSetShader(deferredShadingSimplePS_, nullptr, 0);
	FullscreenPass::getInstance().renderFullscreen(Width, Height);
	*/

	// unbind all textures but keep shadow map
	for (uint32 i = 0; i < 6; i++) { GBufferSRV[i] = nullptr; }
	pDevCon->PSSetShaderResources(0, 6, GBufferSRV);


	// *** FORWARD PASS *** //
	pDevCon->OMSetRenderTargets(1, &hdrColorBuffer_.rtv_, gBuffer_.ds_.m_DSV);





	// Render skybox for two viewports
	pDevCon->OMSetDepthStencilState(depthStencilState_Skybox, 0);
	for (uint32 iCamera = 0; iCamera < numCameras; iCamera++) {
		Camera* cam = e[iCamera].getComponent<Camera>().get();
		ft_engine::Transform* cameraForm = e[iCamera].getComponent<ft_engine::Transform>().get();

		if (bEditor) {
			if (!cam->bEnabled)
				continue;
			renderer.setRenderViewport(renderViewports_[0]);
		} else {
			// render to proper viewport
			renderer.setRenderViewport(renderViewports_[iCamera]);
		}

		// calculate player view/proj matrix (Possible optimization)
		const Matrix matProj = DirectX::XMMatrixPerspectiveFovLH(DEG2RAD(cam->fovDegrees),
			cam->aspectRatio,
			cam->nearPlane,
			cam->farPlane);

		const Matrix matView = DirectX::XMMatrixLookAtLH(cameraForm->getWorldPosition(),
			cameraForm->getWorldPosition() - cameraForm->getWorldForward(),
			cameraForm->getWorldUp());
		renderer.updateViewConstantBuffer(cameraForm->getWorldPosition(), matView, matProj);


		Matrix matWorld;
		matWorld.Translation(cameraForm->getWorldPosition());

		FMaterial matDummy;
		renderer.updateStaticObjectConstantBuffer(matWorld, matDummy.uvTiling, matDummy.uvOffset, matDummy.colorTint.ToVector3(), matDummy.specialEffect, matDummy.smoothness);


		skybox_.renderSkybox();
	}


	pDevCon->VSSetShader(gbufferFeedingVS_, nullptr, 0);
	pDevCon->IASetInputLayout(inputLayoutStaticGeometry_);

	// Transparent objects
	for (uint32 iCamera = 0; iCamera < numCameras; iCamera++) {
		Camera* cam = e[iCamera].getComponent<Camera>().get();
		ft_engine::Player* cameraPlayer = e[iCamera].getComponent<ft_engine::Player>().get();
		ft_engine::Transform* cameraForm = e[iCamera].getComponent<ft_engine::Transform>().get();

		if (bEditor) {
			if (!cam->bEnabled)
				continue;
			renderer.setRenderViewport(renderViewports_[0]);
		} else {
			// render to proper viewport
			renderer.setRenderViewport(renderViewports_[iCamera]);
		}

		// calculate player view/proj matrix
		const Matrix matProj = DirectX::XMMatrixPerspectiveFovLH(DEG2RAD(cam->fovDegrees),
			cam->aspectRatio,
			cam->nearPlane,
			cam->farPlane);

		const Matrix matView = DirectX::XMMatrixLookAtLH(cameraForm->getWorldPosition(),
			cameraForm->getWorldPosition() - cameraForm->getWorldForward(),
			cameraForm->getWorldUp());

		DirectX::BoundingFrustum boundingFrustrum = DirectX::BoundingFrustum(matProj);
		Matrix inverseViewMatrix = matView.Invert();
		boundingFrustrum.Transform(boundingFrustrum, inverseViewMatrix);

		renderer.updateViewConstantBuffer(cameraForm->getWorldPosition(), matView, matProj);


		pDevCon->OMSetDepthStencilState(depthStencilState_NoDepthWrite, 0);
		for (uint32 meshIndex = 0; meshIndex < staticMeshesCount; meshIndex++) {
			StaticMeshRenderer* staticmesh = meshes[meshIndex].getComponent<ft_render::StaticMeshRenderer>().get();
			if (meshes[meshIndex].hasComponent<ft_engine::Player>()) {
				ft_engine::Player* meshPlayer = meshes[meshIndex].getComponent<ft_engine::Player>().get();
				if (meshPlayer->bLocal == cameraPlayer->bLocal) {
					if (!staticmesh->bEnabledOwn)
						continue;
				} else {
					if (!staticmesh->bEnabledOther)
						continue;
				}
			} else {
				if (!staticmesh->bEnabledOwn)
					continue;
			}

			const float Alpha = staticmesh->getMaterial().colorTint.A();
			if (Alpha < 0.9999f) {
				ft_engine::Transform* xform = meshes[meshIndex].getComponent<ft_engine::Transform>().get();
				Matrix matWorld = xform->getWorldTransform();
				DirectX::BoundingBox boundingBox = xform->getTransformedBoundingBox();

				if (!checkIntersection(boundingFrustrum, boundingBox)) {
					continue;
				}

				// Set proper pixel shader
				float blendFactor[4] = { Alpha, Alpha, Alpha, Alpha };
				pDevCon->PSSetShader(forwardPassSimplePS_, nullptr, 0);
				pDevCon->OMSetBlendState(blendStateForwardPassTransparency_, blendFactor, 0xffffffff);

				const FMaterial& meshMaterial = staticmesh->material_;
				renderer.updateStaticObjectConstantBuffer(matWorld, meshMaterial.uvTiling, meshMaterial.uvOffset, meshMaterial.colorTint.ToVector3(), meshMaterial.specialEffect, meshMaterial.smoothness);
				staticmesh->render();
			}
		}
	}

		// After forward pass we have HDR buffer in "hdrColorBuffer_" render texture.
	{
		// Don't use blend state for now
		pDevCon->OMSetBlendState(nullptr, nullptr, 0xffffffff);
		pDevCon->OMSetDepthStencilState(nullptr, 0);

		// Clear shadowMap
		ID3D11ShaderResourceView* pNullSRV = nullptr;
		pDevCon->PSSetShaderResources(6, 1, &pNullSRV);
	}




	//------------------------------------------------------------------------
	// Postprocessing
	//------------------------------------------------------------------------

	//// 1) bloom
	//{
	//	// Render threshold to fullscreen at first.
	//	ID3D11RenderTargetView* pOutputs[1] = { bloomThresholdFullOutput.rtv_ };
	//	pDevCon->OMSetRenderTargets(1, pOutputs, nullptr);

	//	ID3D11ShaderResourceView* pInputs[1] = { hdrColorBuffer_.srv_ };
	//	pDevCon->PSSetShaderResources(0, 1, pInputs);
	//	pDevCon->PSSetShader(bloomThresholdPS, nullptr, 0);

	//	FullscreenPass::getInstance().renderFullscreen(bloomThresholdFullOutput.width_, bloomThresholdFullOutput.height_);


	//	// Perform downsampling.
	//	// Downsample fullscreen bloom to 1/2 x 1/2
	//	{
	//		pDevCon->PSSetShader(pixelShaderSimpleCopy_, nullptr, 0);

	//		pOutputs[0] = bloomColorBuffer_half[1].rtv_;
	//		pDevCon->OMSetRenderTargets(1, pOutputs, nullptr);

	//		pInputs[0] = bloomThresholdFullOutput.srv_;
	//		pDevCon->PSSetShaderResources(0, 1, pInputs);

	//		FullscreenPass::getInstance().renderFullscreen(bloomColorBuffer_half[1].width_, bloomColorBuffer_half[1].height_);
	//	}


	//	// Perform lens flares pass at first - downsample to 1/4 x 1/4 and calculate flares
	//	{
	//		pDevCon->PSSetShader(lensFlaresThresholdPS, nullptr, 0);

	//		pOutputs[0] = lensFlaresTex[1].rtv_;
	//		pDevCon->OMSetRenderTargets(1, pOutputs, nullptr);

	//		pInputs[0] = bloomColorBuffer_half[1].srv_;
	//		pDevCon->PSSetShaderResources(0, 1, pInputs);

	//		FullscreenPass::getInstance().renderFullscreen(lensFlaresTex[1].width_, lensFlaresTex[1].height_);
	//	}

	//	// Perform gaussian blur for bloom
	//	for (uint32 iBlurPass = 0; iBlurPass < 4; ++iBlurPass) {

	//		// Horizontal pass
	//		pDevCon->PSSetShader(bloomGaussianBlurHPS, nullptr, 0);
	//		{
	//			pOutputs[0] = bloomColorBuffer_half[0].rtv_;
	//			pDevCon->OMSetRenderTargets(1, pOutputs, nullptr);

	//			pInputs[0] = bloomColorBuffer_half[1].srv_;
	//			pDevCon->PSSetShaderResources(0, 1, pInputs);

	//			FullscreenPass::getInstance().renderFullscreen(bloomColorBuffer_half[0].width_, bloomColorBuffer_half[0].height_);


	//			// reset now.
	//			pInputs[0] = nullptr;
	//			pDevCon->PSSetShaderResources(0, 1, pInputs);

	//			pOutputs[0] = nullptr;
	//			pDevCon->OMSetRenderTargets(1, pOutputs, nullptr);
	//		}

	//		// Vertical pass
	//		pDevCon->PSSetShader(bloomGaussianBlurVPS, nullptr, 0);
	//		{
	//			pOutputs[0] = bloomColorBuffer_half[1].rtv_;
	//			pDevCon->OMSetRenderTargets(1, pOutputs, nullptr);

	//			pInputs[0] = bloomColorBuffer_half[0].srv_;
	//			pDevCon->PSSetShaderResources(0, 1, pInputs);

	//			FullscreenPass::getInstance().renderFullscreen(bloomColorBuffer_half[1].width_, bloomColorBuffer_half[1].height_);


	//			// reset now.
	//			pInputs[0] = nullptr;
	//			pDevCon->PSSetShaderResources(0, 1, pInputs);

	//			pOutputs[0] = nullptr;
	//			pDevCon->OMSetRenderTargets(1, pOutputs, nullptr);
	//		}
	//	}
	//}

	//// 1) average luminance & eye adaptation
	//if (FWindow::getInstance().isEditorWindow()) {
	//	// do for one window
	//	pDevCon->PSSetShader(pixelShaderLogLuminance_Full, nullptr, 0);
	//	eyeAdaptation_[0].renderAverageLuminanceAndEyeAdaptation(hdrColorBuffer_);

	//} else {
	//	// for two players
	//	for (uint32 iPlayer = 0; iPlayer < numPlayersViewports; ++iPlayer) {

	//		pDevCon->PSSetShader(pixelShaderLogLuminance_Split[iPlayer], nullptr, 0);
	//		eyeAdaptation_[iPlayer].renderAverageLuminanceAndEyeAdaptation(hdrColorBuffer_);
	//	}
	//}

	//// 2) Tonemapping
	//{
	//	pDevCon->OMSetRenderTargets(1, &colorBufferAfterTonemapping_.rtv_, nullptr);
	//	pDevCon->PSSetShader(pixelShaderTonemapping_, nullptr, 0);

	//	for (uint32 iPlayer = 0; iPlayer < numPlayersViewports; ++iPlayer) {
	//		ID3D11ShaderResourceView* pTonemappingInputs[4];
	//		pTonemappingInputs[0] = hdrColorBuffer_.srv_;
	//		pTonemappingInputs[1] = eyeAdaptation_[iPlayer].getAdaptedLuminanceTexture().srv_;
	//		pTonemappingInputs[2] = bloomColorBuffer_half[1].srv_;
	//		pTonemappingInputs[3] = lensFlaresTex[1].srv_;
	//		pDevCon->PSSetShaderResources(0, 4, pTonemappingInputs);


	//		//FullscreenPass::getInstance().renderFullscreen( Width, Height );
	//		FullscreenPass::getInstance().renderFullscreen(renderViewports_[iPlayer].Width, renderViewports_[iPlayer].Height, renderViewports_[iPlayer].TopLeftX, renderViewports_[iPlayer].TopLeftY);
	//	}

	//	ID3D11ShaderResourceView* pNullSRV[4] = { nullptr, nullptr, nullptr, nullptr };
	//	pDevCon->PSSetShaderResources(0, 4, pNullSRV);

	//}

	//// 3) AA
	//{
	//	ID3D11RenderTargetView* aaOutput[1] = { hdrColorBuffer_.rtv_ };
	//	pDevCon->OMSetRenderTargets(1, aaOutput, nullptr);

	//	ID3D11ShaderResourceView* aaInputs[1] = { colorBufferAfterTonemapping_.srv_ };
	//	pDevCon->PSSetShaderResources(0, 1, aaInputs);
	//	pDevCon->PSSetShader(fxaaPS, nullptr, 0);

	//	FullscreenPass::getInstance().renderFullscreen(hdrColorBuffer_.width_, hdrColorBuffer_.height_);


	//	// set to null
	//	aaInputs[0] = nullptr;
	//	pDevCon->PSSetShaderResources(0, 1, aaInputs);

	//	aaOutput[0] = nullptr;
	//	pDevCon->OMSetRenderTargets(1, aaOutput, nullptr);
	//}

	// 4) Final postprocess pass to backbuffer, back to gamma space
	auto backbuffer = renderer.getBackbuffer();
	pDevCon->OMSetRenderTargets(1, &backbuffer, nullptr);

	pDevCon->PSSetShader(pixelShaderFinalPass_, nullptr, 0);
	pDevCon->PSSetShaderResources(0, 1, &hdrColorBuffer_.srv_);

	FullscreenPass::getInstance().renderFullscreen(Width, Height);

	// imGui stuff
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// Present to swapchain
	framework::FRendererD3D11::getInstance().swapchainPresent();
}

void ft_render::RenderSystem::onPostSceneLoaded(const EventPostSceneLoaded& event) {
	requestSceneSpecificDataUpdate_ = true;
}

void ft_render::RenderSystem::prepareDeferredShadingPass() {
	framework::FRendererD3D11& renderer = framework::FRendererD3D11::getInstance();
	ID3D11DeviceContext* pDevCon = renderer.getD3D11DeviceContext();

	ID3D11ShaderResourceView* NullSRV[5] = {
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	};
	pDevCon->PSSetShaderResources(0, 5, NullSRV);

	gBuffer_.attach();

	bool clearGBufferRenderTargets = false;
#ifdef _DEBUG
	clearGBufferRenderTargets = true;
#endif

	gBuffer_.clear(false, clearGBufferRenderTargets);

	// default rasterizer state
	pDevCon->RSSetState(nullptr);

	// Attach "Normals Fitting" texture to slot PS13
	ID3D11ShaderResourceView* srv[] = { texNormalsFitting_.getSRV() };
	pDevCon->PSSetShaderResources(13, 1, srv);
}

void ft_render::RenderSystem::setupLighting(eecs::EntityManager& entities) {
	auto& data = cbufferLighting_.GetBufferData();

	/* cylinder lights */
	std::vector<eecs::Entity> lightEntities = entities.getEntitiesWithComponents<ft_render::CylinderLight, ft_engine::Transform>();
	std::vector<eecs::Entity> pointLights = entities.getEntitiesWithComponents<ft_render::PointLight, ft_engine::Transform>();
	data.numCylinderLights = lightEntities.size() + pointLights.size();

	size_t lightCounter = 0;
	for (; lightCounter < lightEntities.size(); ++lightCounter) {
		ft_render::CylinderLight* cl = lightEntities[lightCounter].getComponent<ft_render::CylinderLight>().get();
		ft_engine::Transform* cylinderTransform = lightEntities[lightCounter].getComponent<ft_engine::Transform>().get();

		data.cylinderLights[lightCounter].attenuation = cl->attenuation;

		data.cylinderLights[lightCounter].color.x = cl->baseLight->color.R();
		data.cylinderLights[lightCounter].color.y = cl->baseLight->color.G();
		data.cylinderLights[lightCounter].color.z = cl->baseLight->color.B();

		data.cylinderLights[lightCounter].intensity = cl->baseLight->intensity;
		data.cylinderLights[lightCounter].radius = cl->radius;

		data.cylinderLights[lightCounter].posStart = XMVector3Transform(cl->start, cylinderTransform->getWorldTransform());
		data.cylinderLights[lightCounter].posEnd = XMVector3Transform(cl->end, cylinderTransform->getWorldTransform());
	}

	for (; lightCounter < lightEntities.size() + pointLights.size(); ++lightCounter) {
		ft_render::PointLight* pl = pointLights[lightCounter - lightEntities.size()].getComponent<ft_render::PointLight>().get();
		ft_engine::Transform* pointTransform = pointLights[lightCounter - lightEntities.size()].getComponent<ft_engine::Transform>().get();

		data.cylinderLights[lightCounter].attenuation = pl->attenuation;

		data.cylinderLights[lightCounter].color.x = pl->baseLight->color.R();
		data.cylinderLights[lightCounter].color.y = pl->baseLight->color.G();
		data.cylinderLights[lightCounter].color.z = pl->baseLight->color.B();

		data.cylinderLights[lightCounter].intensity = pl->baseLight->intensity;
		data.cylinderLights[lightCounter].radius = pl->radius;

		data.cylinderLights[lightCounter].posStart = pointTransform->getWorldPosition();
		data.cylinderLights[lightCounter].posEnd = pointTransform->getWorldPosition();
	}

	eecs::Entity dirLight = entities.getEntityWithComponents<ft_render::DirectionalLight, ft_engine::Transform>();

	isDirLightPresent_ = dirLight.isValid();
	if (isDirLightPresent_) {
		ft_engine::Transform* dl = dirLight.getComponent<ft_engine::Transform>().get();
		ft_render::DirectionalLight* light = dirLight.getComponent<ft_render::DirectionalLight>().get();

		data.dirLight = globalDirectionaLightDirection_ = -dl->getWorldForward();
		data.dirLightColor.x = light->baseLight->color.R();
		data.dirLightColor.y = light->baseLight->color.G();
		data.dirLightColor.z = light->baseLight->color.B();
		data.dirLightIntensity = light->baseLight->intensity;

		DirectX::BoundingBox bd = dl->getTransformedBoundingBox();
		Vector3 absExtents = Vector3(std::abs(bd.Extents.x), std::abs(bd.Extents.y), std::abs(bd.Extents.z));
		sceneAABBMin = bd.Center - absExtents;
		sceneAABBMax = bd.Center + absExtents;
	}

	ID3D11DeviceContext* pDevCon = FRendererD3D11::getInstance().getD3D11DeviceContext();
	cbufferLighting_.UpdateBuffer(pDevCon);
	cbufferLighting_.SetPS(pDevCon, 5);
	cbufferLighting_.SetVS(pDevCon, 5);

}

//------------------------------------------------------------------------
void ft_render::RenderSystem::renderShadows(TVecEntities& staticMeshes, const uint32 staticMeshesCount,
	TVecEntities& skinnedMeshes, const uint32 skinnedMeshesCount) {
	framework::FRendererD3D11& renderer = framework::FRendererD3D11::getInstance();
	ID3D11DeviceContext* pDevCon = renderer.getD3D11DeviceContext();


	// Setup light's view and projection matrices.
	Vector3 lightDir = globalDirectionaLightDirection_;
	lightDir.Normalize();


	// hard-coded scene bounding box
	//Vector3 sceneAABBMin(-20.f, -0.5f, -5.f);
	//Vector3 sceneAABBMax(+20.f, +5.f, 10.f);

	Vector3 sceneSphereCenter = (sceneAABBMin + sceneAABBMax) * 0.5f;
	float sceneSphereRadius = (sceneAABBMax - sceneSphereCenter).Length();


	Vector3 lightPos = -2.f * sceneSphereRadius * lightDir;
	Vector3 lightLookAt = sceneSphereCenter;

	Vector3 lightPos2 = -40.f * lightDir;
	// calculate light view/proj matrix
	Matrix lightView = DirectX::XMMatrixLookAtLH(lightPos, lightLookAt, Vector3::Up);

	// transform scene center by lightView matrix
	Vector3 sphereCenterLS(0.f, 0.f, 0.f);
	sphereCenterLS = DirectX::XMVector3Transform(sphereCenterLS, lightView);

	float l = sphereCenterLS.x - sceneSphereRadius;
	float b = sphereCenterLS.y - sceneSphereRadius;
	float n = sphereCenterLS.z - sceneSphereRadius;
	float r = sphereCenterLS.x + sceneSphereRadius;
	float t = sphereCenterLS.y + sceneSphereRadius;
	float f = sphereCenterLS.z + sceneSphereRadius;

	const Matrix lightProj = DirectX::XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);
	//const Matrix lightProj = DirectX::XMMatrixOrthographicLH(20.f, 20.f, 0.1, 100.f);

	renderer.updateViewConstantBufferShadow(lightView, lightProj);

	// Setup viewport for shadow map
	D3D11_VIEWPORT vp;
	vp.Width = static_cast<float>(shadowMapResolution_);
	vp.Height = vp.Width;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	pDevCon->RSSetViewports(1, &vp);


	ID3D11RenderTargetView* pNullRTV[1] = { nullptr };

	pDevCon->ClearDepthStencilView(shadowMap_.m_DSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
	pDevCon->OMSetRenderTargets(0, pNullRTV, shadowMap_.m_DSV);

	pDevCon->PSSetShader(nullptr, nullptr, 0);

	pDevCon->VSSetShader(depthOnlyStaticMeshesVS_, nullptr, 0);
	pDevCon->IASetInputLayout(inputLayoutStaticGeometry_);

	pDevCon->RSSetState(rasterizerStateShadows);

	// Rendering, depth only.
	for (uint32 i = 0; i < staticMeshesCount; i++) {
		StaticMeshRenderer* staticmesh = staticMeshes[i].getComponent<ft_render::StaticMeshRenderer>().get();

		ft_engine::Transform* xform = staticMeshes[i].getComponent<ft_engine::Transform>().get();
		const Matrix matWorld = xform->getWorldTransform();

		const FMaterial& meshMaterial = staticmesh->material_;
		if (meshMaterial.colorTint.A() < 0.99f) {
			continue;
		}

		renderer.updateStaticObjectConstantBuffer(matWorld, meshMaterial.uvTiling, meshMaterial.uvOffset, meshMaterial.colorTint.ToVector3(), meshMaterial.specialEffect, meshMaterial.smoothness);

		// TODO (MN): "renderDepth" method without setting textures - we don't need them
		staticmesh->render();

	}


	for (uint32 i = 0; i < skinnedMeshesCount; i++) {
		SkinnedMeshRenderer* skinnedmesh = skinnedMeshes[i].getComponent<ft_render::SkinnedMeshRenderer>().get();

		ft_engine::Transform* xform = skinnedMeshes[i].getComponent<ft_engine::Transform>().get();
		const Matrix matWorld = xform->getWorldTransform();

		const FMaterial& meshMaterial = skinnedmesh->material_;
		renderer.updateStaticObjectConstantBuffer(matWorld, meshMaterial.uvTiling, meshMaterial.uvOffset, meshMaterial.colorTint.ToVector3(), meshMaterial.specialEffect, meshMaterial.smoothness);

		if (skinnedmesh->bAnimationEnabled) {
			std::vector<Matrix> bones;
			skinnedmesh->boneTransform(bones);

			const uint32 numBones = bones.size();
			for (uint32 ij = 0; ij < numBones; ij++) {
				renderer.cbufferSkinning_.GetBufferData().bones[ij] = bones[ij];
			}

			renderer.cbufferSkinning_.SetVS(pDevCon, 2);
			renderer.cbufferSkinning_.UpdateBuffer(pDevCon);

			pDevCon->VSSetShader(depthOnlySkinnedMeshesVS_, nullptr, 0);
			pDevCon->IASetInputLayout(inputLayoutSkinnedGeometry_);

			skinnedmesh->render();
		} else {
			pDevCon->VSSetShader(gbufferFeedingVS_, nullptr, 0);
			pDevCon->IASetInputLayout(inputLayoutStaticGeometry_);


			skinnedmesh->renderNoSkinning();
		}
	}




}

void ft_render::RenderSystem::moveCamera(ft_render::Camera* camera, ft_engine::Transform* transform) {
	Mouse& mouse = framework::FInput::getMouse();
	const Keyboard& keyboard = framework::FInput::getKeyboard();
	if (ASSERT_FAIL(mouse.IsConnected(), "Mouse not connected"))
		return;
	if (ASSERT_FAIL(keyboard.IsConnected(), "Keboard not connected"))
		return;

	Mouse::State mouseState = mouse.getState();
	const Keyboard::State keyboardState = keyboard.getState();

	static float horizontalSpeed = 4.0f, verticalSpeed = 4.0f, scrollSpeed = 0.5f, rotationSpeed = 1.0f;
	const float horizontal = (static_cast<float>(keyboardState.D) - static_cast<float>(keyboardState.A)) * framework::FTime::deltaTime;
	const float vertical = (static_cast<float>(keyboardState.W) - static_cast<float>(keyboardState.S)) * framework::FTime::deltaTime;
	const float forward = (mouseState.scrollWheelValue) * framework::FTime::deltaTime;
	mouse.ResetScrollWheelValue();

	const Vector3 horizontalVec = transform->getWorldRight() * horizontal * horizontalSpeed;
	const Vector3 verticalVec = transform->getWorldUp() * vertical * verticalSpeed;
	const Vector3 forwardVec = -transform->getWorldForward() * forward * scrollSpeed;
	transform->translate(horizontalVec + verticalVec + forwardVec);

	const float rotationY = -(static_cast<float>(keyboardState.Q) - static_cast<float>(keyboardState.E)) * framework::FTime::deltaTime;
	const float rotationX = -(static_cast<float>(keyboardState.Z) - static_cast<float>(keyboardState.X)) * framework::FTime::deltaTime;
	if (rotationX != 0.0f || rotationY != 0.0f) {
		float theoreticalEulerX = transform->getLocalRotation().Euler().x + rotationX * rotationSpeed;
		if (theoreticalEulerX < DEG2RAD(180.0f)) {
			theoreticalEulerX = clamp(theoreticalEulerX, DEG2RAD(1.0f), DEG2RAD(85.0f));
		}

		Quaternion rotationNoZ = Quaternion::CreateFromYawPitchRoll(
			transform->getLocalRotation().Euler().y + rotationY * rotationSpeed,
			theoreticalEulerX,
			0.0f
		);
		rotationNoZ.Normalize();

		transform->setLocalTransform(Matrix::Compose(
			transform->getLocalPosition(),
			rotationNoZ,
			transform->getLocalScale()
		));
	}
}

bool ft_render::RenderSystem::checkIntersection(const DirectX::BoundingFrustum & boundingFrustrum, const DirectX::BoundingBox & boundingBox) {
	DirectX::XMVECTOR FrustumOrientation = XMLoadFloat4(&boundingFrustrum.Orientation);
	DirectX::BoundingOrientedBox OBB = DirectX::BoundingOrientedBox(boundingBox.Center, boundingBox.Extents, DirectX::XMFLOAT4(0.f, 0.f, 0.f, 1.f));
	DirectX::XMVECTOR BoxOrientation = XMLoadFloat4(&OBB.Orientation);
	if (!DirectX::Internal::XMQuaternionIsUnit(FrustumOrientation) || !DirectX::Internal::XMQuaternionIsUnit(BoxOrientation)) {
		return true;
	}

	try {
		bool bReturnValue = boundingFrustrum.Intersects(boundingBox);
		return bReturnValue;
	} catch (std::exception& exception) {
		return true;
	}
}
