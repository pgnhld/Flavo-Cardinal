#include "FRendererD3D11.h"
#include <d3dcompiler.h>
#include "Assertion.h"
#include "FMaterial.h"

#include "FWindow.h"
#include <locale>

#define EXPORT_SHADERS_TO_BINARY	1
#define LOAD_SHADERS_FROM_BINARY	0

std::string WStringToAnsi(const wchar_t* wstr) {
	char buffer[512];
	::WideCharToMultiByte(CP_ACP, 0, wstr, -1, buffer, 512, nullptr, nullptr);

	return std::string(buffer);
}



framework::FRendererD3D11* framework::FRendererD3D11::instance_ = nullptr;

framework::FRendererD3D11::FRendererD3D11(HWND hwnd, int32 width, int32 height, bool fullscreen)
	: device_(nullptr)
	, deviceContext_(nullptr)
	, swapChain_(nullptr)
	, pBackbufferRTV_(nullptr) {
	instance_ = this;

	const bool bResult = initRenderer(hwnd, width, height, fullscreen);
	ASSERT_FAIL(bResult, "Init failed");
}

framework::FRendererD3D11& framework::FRendererD3D11::getInstance() {
	return *instance_;
}

void framework::FRendererD3D11::cleanup() {
	SAFE_RELEASE(device_);
	SAFE_RELEASE(deviceContext_);
	SAFE_RELEASE(swapChain_);
	SAFE_RELEASE(pBackbufferRTV_);
	SAFE_RELEASE(pBackbufferDS_);
	SAFE_RELEASE(pBackbufferDSView_);

	cbufferPerView_.Release();
	cbufferPerStaticObject_.Release();
	cbufferSkinning_.Release();

	// common states
	commonStates_->ReleaseResources();
	framework::rendering::FD3D11States::Destroy();
	commonStates_ = nullptr;
}

framework::FRendererD3D11::FRendererD3D11()
	: device_(nullptr)
	, deviceContext_(nullptr)
	, swapChain_(nullptr)
	, pBackbufferRTV_(nullptr) {
}

void framework::FRendererD3D11::setRenderViewport(const RenderViewport& rvp) {
	deviceContext_->RSSetViewports(1, (const D3D11_VIEWPORT*)&rvp);
}

void framework::FRendererD3D11::setRenderViewports(const RenderViewport* rvps, uint32 numViewports) {
	deviceContext_->RSSetViewports(numViewports, (const D3D11_VIEWPORT*)rvps);
}

ID3D11Device* framework::FRendererD3D11::getD3D11Device() const {
	return device_;
}

ID3D11DeviceContext* framework::FRendererD3D11::getD3D11DeviceContext() const {
	return deviceContext_;
}

void framework::FRendererD3D11::setAllSamplers() {
	auto ps = commonStates_->Get();

	ID3D11SamplerState* pSamplers[] =
	{
		ps->pSamplerLinearWrap,
		ps->pSamplerLinearClamp,
		ps->pSamplerPointWrap,
		ps->pSamplerPointClamp,
		ps->pSamplerAnisoWrap,
		ps->pSamplerAnisoClamp,
		ps->pSamplerComparisonLinear
	};

	const unsigned int nSamplers = ARRAYSIZE(pSamplers);

	deviceContext_->VSSetSamplers(0, nSamplers, pSamplers);
	deviceContext_->PSSetSamplers(0, nSamplers, pSamplers);
	deviceContext_->CSSetSamplers(0, nSamplers, pSamplers);
	//deviceContext_->HSSetSamplers( 0, nSamplers, pSamplers );
	//deviceContext_->DSSetSamplers( 0, nSamplers, pSamplers );
}

uint32 framework::FRendererD3D11::getRendererWidth() const {
	return rendererWidth_;
}

uint32 framework::FRendererD3D11::getRendererHeight() const {
	return rendererHeight_;
}

ID3D11RenderTargetView* framework::FRendererD3D11::getBackbuffer() const {
	return pBackbufferRTV_;
}

void framework::FRendererD3D11::clearBackbuffer() {
	deviceContext_->ClearRenderTargetView(pBackbufferRTV_, framework::CLEAR_COLOR);
}

void framework::FRendererD3D11::updateViewConstantBuffer(const Vector3& eyePos, const Matrix44& viewMatrix, const Matrix44& projMatrix) {
	cbufferPerView_.GetBufferData().matView = viewMatrix;
	cbufferPerView_.GetBufferData().matProj = projMatrix;
	cbufferPerView_.GetBufferData().cameraPos_ = eyePos;


	Matrix matView = viewMatrix;
	Matrix matProj = projMatrix;
	Matrix matViewProj = matView * matProj;
	cbufferPerView_.GetBufferData().matViewProj = matViewProj;

	float Width = (float)getRendererWidth();
	const float Height = (float)getRendererHeight();

	cbufferPerView_.UpdateBuffer(deviceContext_);
	cbufferPerView_.SetVS(deviceContext_, 1);
	cbufferPerView_.SetPS(deviceContext_, 1);
	cbufferPerView_.SetGS(deviceContext_, 1);
}

void framework::FRendererD3D11::updateViewConstantBufferMultiCamera( const Vector3& eyePos1, const Vector3& eyePos2 ) {
	cbufferPerView_.GetBufferData().cameraPos1_ = eyePos1;
	cbufferPerView_.GetBufferData().cameraPos2_ = eyePos2;

	cbufferPerView_.UpdateBuffer( deviceContext_ );
}

void framework::FRendererD3D11::updateViewConstantBufferShadow(const Matrix44& viewMatrix, const Matrix44& projMatrix) {
	// We only care about lightView*lightProj matrix


	//cbufferPerView_.GetBufferData().matView = viewMatrix;
	//cbufferPerView_.GetBufferData().matProj = projMatrix;

	Matrix matView = viewMatrix;
	Matrix matProj = projMatrix;
	Matrix matViewProj = matView * matProj;
	cbufferPerView_.GetBufferData().matDirLightViewProj = matViewProj;

	cbufferPerView_.UpdateBuffer( deviceContext_ );
	cbufferPerView_.SetVS( deviceContext_, 1 );
}

void framework::FRendererD3D11::updateStaticObjectConstantBuffer(const Matrix44& worldMatrix, Vector2 uvScale, Vector2 uvOffset, Vector3 colorTint, float specialEffect, float smoothness) {
	auto& buffer = cbufferPerStaticObject_.GetBufferData();
	buffer.matWorld = worldMatrix;
	buffer.uvScale = uvScale;
	buffer.uvOffset = uvOffset;
	buffer.colorTint = colorTint;
	buffer.specialEffect = specialEffect;
	buffer.smoothness = smoothness;

	cbufferPerStaticObject_.UpdateBuffer(deviceContext_);
	cbufferPerStaticObject_.SetVS(deviceContext_, 0);
	cbufferPerStaticObject_.SetPS(deviceContext_, 0);
}

void framework::FRendererD3D11::swapchainPresent() {
	
	const UINT syncInterval = FWindow::getInstance().isVsyncEnabled() ? 1 : 0;

	swapChain_->Present(syncInterval, 0);
}

bool framework::FRendererD3D11::initRenderer(HWND hwnd, int32 width, int32 height, bool fullscreen) {
	rendererWidth_ = (uint32)width;
	rendererHeight_ = (uint32)height;

	HRESULT hr = S_OK;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		/* D3D_FEATURE_LEVEL_11_1, */
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	// Attempt to create D3D11 device.
	for (uint32 driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
		const D3D_DRIVER_TYPE selectedDriverType = driverTypes[driverTypeIndex];

		hr = D3D11CreateDevice(nullptr, selectedDriverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &device_, &featureLevel_, &deviceContext_);

		if (SUCCEEDED(hr)) {
			break;
		}
	}

	if (FAILED(hr)) {
		return false;
	}

	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	IDXGIFactory1* dxgiFactory = nullptr;
	{
		IDXGIDevice* dxgiDevice = nullptr;
		hr = device_->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
		if (SUCCEEDED(hr)) {
			IDXGIAdapter* adapter = nullptr;
			hr = dxgiDevice->GetAdapter(&adapter);

			if (SUCCEEDED(hr)) {
				hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
				adapter->Release();
			}
			dxgiDevice->Release();
		}
	}
	if (FAILED(hr)) {
		return false;
	}

	// Creating swap chain.
	{
		DXGI_SWAP_CHAIN_DESC sd;
		memset(&sd, 0, sizeof(sd));

		sd.BufferCount = 2;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 0;    // use 0 to avoid a potential mismatch with hw
		sd.BufferDesc.RefreshRate.Denominator = 0;	// same as above

		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hwnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = !fullscreen;

		hr = dxgiFactory->CreateSwapChain(device_, &sd, &swapChain_);
	}

	// Disable Alt+Enter shortcut
	dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);
	SAFE_RELEASE(dxgiFactory);

	if (FAILED(hr)) {
		return false;
	}

	// Create render target view.
	ID3D11Texture2D* pBackbuffer = nullptr;
	hr = swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackbuffer);
	if (FAILED(hr)) {
		return false;
	}

	hr = device_->CreateRenderTargetView(pBackbuffer, nullptr, &pBackbufferRTV_);
	SAFE_RELEASE(pBackbuffer);

	if (FAILED(hr)) {
		return false;
	}

	// Create depth/stencil (D24S8) texture.
	// TODO: Maybe later we won't need this one.
	D3D11_TEXTURE2D_DESC dsDesc;
	memset(&dsDesc, 0, sizeof(dsDesc));
	dsDesc.Width = width;
	dsDesc.Height = height;
	dsDesc.MipLevels = 1;
	dsDesc.ArraySize = 1;
	dsDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsDesc.SampleDesc.Count = 1;
	dsDesc.SampleDesc.Quality = 0;
	dsDesc.Usage = D3D11_USAGE_DEFAULT;
	dsDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dsDesc.CPUAccessFlags = 0;
	dsDesc.MiscFlags = 0;
	hr = device_->CreateTexture2D(&dsDesc, nullptr, &pBackbufferDS_);
	if (FAILED(hr)) {
		return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	memset(&descDSV, 0, sizeof(descDSV));
	descDSV.Format = dsDesc.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = device_->CreateDepthStencilView(pBackbufferDS_, &descDSV, &pBackbufferDSView_);
	if (FAILED(hr)) {
		return false;
	}


	// Okay, we have created D3D11 device.
	framework::rendering::FD3D11States::Claim();
	commonStates_ = framework::rendering::FD3D11States::Get();
	commonStates_->CreateResources(device_);

	// Create constant buffers
	cbufferPerView_.Create(device_);
	cbufferPerStaticObject_.Create(device_);
	cbufferSkinning_.Create(device_);

	return true;
}

bool framework::FRendererD3D11::compileD3DShader(const wchar_t* fileName, const char* entryPoint, const char* shaderModel, ID3DBlob** blobOut) {
	HRESULT hr = S_OK;
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	dwShaderFlags |= D3DCOMPILE_DEBUG;
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	ID3DBlob* pErrorBlob = nullptr;
	
	// Process some strings related to binary shaders
	// Get strings
	std::string strFilename = WStringToAnsi( fileName );
	std::string strEntrypoint( entryPoint );

	// Get directory
	char exePath[MAX_PATH + 1];
	::GetModuleFileName( nullptr, exePath, MAX_PATH + 1 );
	std::string strDirectory(exePath);
	strDirectory = strDirectory.substr(0, strDirectory.rfind("Bin") );
	strDirectory += std::string("Data\\Shaders\\");

	std::string shaderName = strFilename.substr( strFilename.find_last_of( "\\/" ) + 1 );
	shaderName = shaderName.substr( 0, shaderName.rfind( ".hlsl" ) );

	// Concatenate
	const std::string fullShaderFileName = shaderName + std::string( "_" ) + strEntrypoint + std::string( ".bin" );
	const std::string shaderBinPath = strDirectory + fullShaderFileName;

#if (LOAD_SHADERS_FROM_BINARY == 0)
	hr = D3DCompileFromFile(fileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint,
		shaderModel, dwShaderFlags, 0, blobOut, &pErrorBlob);

	if (FAILED(hr)) {
		if (pErrorBlob) {
			::OutputDebugStringA((const char*)pErrorBlob->GetBufferPointer());
			DEBUG_BREAK
		}

		return false;
	}
#else
	// Load shaders from file

	FILE* pBinaryShaderFile = nullptr;
	fopen_s(&pBinaryShaderFile, shaderBinPath.c_str(), "rb");

	if (pBinaryShaderFile)	{
		uint32 dataLength = 0;

		fseek( pBinaryShaderFile, 0, SEEK_END );
		dataLength = ftell(pBinaryShaderFile);
		fseek( pBinaryShaderFile, 0, SEEK_SET );

		D3DCreateBlob(dataLength, blobOut);
		char* pBuffer = (char*) (*blobOut)->GetBufferPointer();

		fread( pBuffer, sizeof(char) * dataLength, 1, pBinaryShaderFile );
	}
	fclose( pBinaryShaderFile );

#endif



	// TODO: Handle failed case of compiling shader.

#ifdef EXPORT_SHADERS_TO_BINARY
	// binary file format: ShaderFile__Entrypoint.bin

	

	// For safety reasons, get rid of all potential debug info from shaders
	ID3DBlob* pStrippedShaderBlob = nullptr;

	const UINT stripFlags = D3DCOMPILER_STRIP_DEBUG_INFO | D3DCOMPILER_STRIP_REFLECTION_DATA | D3DCOMPILER_STRIP_TEST_BLOBS;
	D3DStripShader( (*blobOut)->GetBufferPointer(), (*blobOut)->GetBufferSize(), stripFlags, &pStrippedShaderBlob );
	
	FILE* file = nullptr;
	fopen_s(&file, shaderBinPath.c_str(), "wb");

	fwrite( pStrippedShaderBlob->GetBufferPointer(), pStrippedShaderBlob->GetBufferSize(), 1, file );
	fclose(file);

	SAFE_RELEASE(pStrippedShaderBlob);

#endif


	SAFE_RELEASE(pErrorBlob);

	return true;
}
