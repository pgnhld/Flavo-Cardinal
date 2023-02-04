#include "Rendering/RenderSystemSkybox.h"
#include "FRendererD3D11.h"

ft_render::FSkybox::FSkybox() {

}

ft_render::FSkybox::~FSkybox() {

}

bool ft_render::FSkybox::initialize() {
	// Create geometry, shaders, input layout and so on for skybox.

	framework::FRendererD3D11& renderer = framework::FRendererD3D11::getInstance();
	ID3D11Device* dev = renderer.getD3D11Device();
	ID3D11DeviceContext* dc = renderer.getD3D11DeviceContext();

	ID3DBlob* blobVSSkybox = nullptr;
	if (framework::FRendererD3D11::compileD3DShader( L"..//Data//Shaders//Skybox.hlsl", "SkyboxVS", "vs_5_0", &blobVSSkybox )) {
		dev->CreateVertexShader( (const void*)blobVSSkybox->GetBufferPointer(), blobVSSkybox->GetBufferSize(), nullptr, &pSkyboxVS_ );
	}

	ID3DBlob* blobPSSkybox = nullptr;
	if (framework::FRendererD3D11::compileD3DShader( L"..//Data//Shaders//Skybox.hlsl", "SkyboxPS", "ps_5_0", &blobPSSkybox )) {
		dev->CreatePixelShader( (const void*)blobPSSkybox->GetBufferPointer(), blobPSSkybox->GetBufferSize(), nullptr, &pSkyboxPS_ );
	}

	ID3DBlob* blobVSMoon = nullptr;
	ID3DBlob* blobGSMoon = nullptr;
	ID3DBlob* blobPSMoon = nullptr;
	if (framework::FRendererD3D11::compileD3DShader( L"..//Data//Shaders//Moon.hlsl", "MoonVS", "vs_5_0", &blobVSMoon )) {
		dev->CreateVertexShader( (const void*)blobVSMoon->GetBufferPointer(), blobVSMoon->GetBufferSize(), nullptr, &pMoonVS_ );
	}
	if (framework::FRendererD3D11::compileD3DShader( L"..//Data//Shaders//Moon.hlsl", "MoonPS", "ps_5_0", &blobPSMoon )) {
		dev->CreatePixelShader( (const void*)blobPSMoon->GetBufferPointer(), blobPSMoon->GetBufferSize(), nullptr, &pMoonPS_ );
	}
	if (framework::FRendererD3D11::compileD3DShader( L"..//Data//Shaders//Moon.hlsl", "MoonGS", "gs_5_0", &blobGSMoon )) {
		dev->CreateGeometryShader( (const void*)blobGSMoon->GetBufferPointer(), blobGSMoon->GetBufferSize(), nullptr, &pMoonGS_ );
	}

	const D3D11_INPUT_ELEMENT_DESC inputLayoutSkybox[] =
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE( inputLayoutSkybox );
	dev->CreateInputLayout( inputLayoutSkybox, numElements, (const void*)blobVSSkybox->GetBufferPointer(),
							blobVSSkybox->GetBufferSize(), &pSkyboxInputLayout_ );


	SAFE_RELEASE(blobVSSkybox);
	SAFE_RELEASE(blobPSSkybox);

	SAFE_RELEASE(blobVSMoon);
	SAFE_RELEASE(blobGSMoon);
	SAFE_RELEASE(blobPSMoon);

	// TODO: ~SkyboxComponent, option to change per scene (MN)
	cubemap_.load(dev, dc, std::string("..//Data//Textures//Skybox//skybox2.dds") );
	moon_.load(dev, dc, std::string("..//Data//Textures//Skybox//moon.dds") );

	// Geometry
	SkyboxVertex skyboxVertices[] = {
		// positions          
		Vector3( -1.0f,  1.0f, -1.0f ),
		Vector3( -1.0f, -1.0f, -1.0f ),
		Vector3( 1.0f, -1.0f, -1.0f ),
		Vector3( 1.0f, -1.0f, -1.0f ),
		Vector3( 1.0f,  1.0f, -1.0f ),
		Vector3( -1.0f,  1.0f, -1.0f ),

		Vector3( -1.0f, -1.0f,  1.0f ),
		Vector3( -1.0f, -1.0f, -1.0f ),
		Vector3( -1.0f,  1.0f, -1.0f ),
		Vector3( -1.0f,  1.0f, -1.0f ),
		Vector3( -1.0f,  1.0f,  1.0f ),
		Vector3( -1.0f, -1.0f,  1.0f ),

		Vector3( 1.0f, -1.0f, -1.0f ),
		Vector3( 1.0f, -1.0f,  1.0f ),
		Vector3( 1.0f,  1.0f,  1.0f ),
		Vector3( 1.0f,  1.0f,  1.0f ),
		Vector3( 1.0f,  1.0f, -1.0f ),
		Vector3( 1.0f, -1.0f, -1.0f ),

		Vector3( -1.0f, -1.0f,  1.0f ),
		Vector3( -1.0f,  1.0f,  1.0f ),
		Vector3( 1.0f,  1.0f,  1.0f ),
		Vector3( 1.0f,  1.0f,  1.0f ),
		Vector3( 1.0f, -1.0f,  1.0f ),
		Vector3( -1.0f, -1.0f,  1.0f ),

		Vector3( -1.0f,  1.0f, -1.0f ),
		Vector3( 1.0f,  1.0f, -1.0f ),
		Vector3( 1.0f,  1.0f,  1.0f ),
		Vector3( 1.0f,  1.0f,  1.0f ),
		Vector3( -1.0f,  1.0f,  1.0f ),
		Vector3( -1.0f,  1.0f, -1.0f ),

		Vector3( -1.0f, -1.0f, -1.0f ),
		Vector3( -1.0f, -1.0f,  1.0f ),
		Vector3( 1.0f, -1.0f, -1.0f ),
		Vector3( 1.0f, -1.0f, -1.0f ),
		Vector3( -1.0f, -1.0f,  1.0f ),
		Vector3( 1.0f, -1.0f,  1.0f )
	};

	// Create vertex buffer
	D3D11_BUFFER_DESC bufDesc;
	bufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufDesc.ByteWidth = ARRAYSIZE(skyboxVertices) * sizeof(SkyboxVertex);
	bufDesc.CPUAccessFlags = 0;
	bufDesc.MiscFlags = 0;
	bufDesc.Usage = D3D11_USAGE_IMMUTABLE;

	D3D11_SUBRESOURCE_DATA resData;
	resData.pSysMem = &skyboxVertices[0];

	HRESULT hr = dev->CreateBuffer(&bufDesc, &resData, &pSkyboxVertexBuffer_);
	if (FAILED(hr)) {
		return false;
	}


	// Vertex buffer for the Moon (billboarding)
	SkyboxVertex moonVertexDummy[1];
	moonVertexDummy[0].position = Vector3(0.0f, 0.0f, 0.0f);

	bufDesc.ByteWidth = ARRAYSIZE(moonVertexDummy) * sizeof(SkyboxVertex);
	resData.pSysMem = &moonVertexDummy[0];

	hr = dev->CreateBuffer(&bufDesc, &resData, &pMoonVertexBuffer_);
	if (FAILED(hr)) {
		return false;
	}


	return true;
}

void ft_render::FSkybox::release() {
	SAFE_RELEASE(pSkyboxInputLayout_);
	SAFE_RELEASE(pSkyboxVS_);
	SAFE_RELEASE(pSkyboxPS_);
	SAFE_RELEASE(pSkyboxVertexBuffer_);

	SAFE_RELEASE(pMoonVertexBuffer_);
	SAFE_RELEASE(pMoonVS_);
	SAFE_RELEASE(pMoonGS_);
	SAFE_RELEASE(pMoonPS_);

	cubemap_.cleanup();
	moon_.cleanup();
}

void ft_render::FSkybox::renderSkybox() {
	ID3D11DeviceContext* devCon = framework::FRendererD3D11::getInstance().getD3D11DeviceContext();

	ID3D11ShaderResourceView* skyboxInputs[1] = { cubemap_.getSRV() };
	devCon->PSSetShaderResources(13, 1, skyboxInputs);

	devCon->IASetInputLayout(pSkyboxInputLayout_);

	const uint32 stride[1] = { sizeof(SkyboxVertex) };
	const uint32 offset[1] = { 0 };

	devCon->IASetVertexBuffers(0, 1, &pSkyboxVertexBuffer_, stride, offset);
	devCon->VSSetShader(pSkyboxVS_, nullptr, 0);
	devCon->PSSetShader(pSkyboxPS_, nullptr, 0);

	devCon->Draw(36, 0);

	// Render the Moon
	{
		devCon->IASetVertexBuffers(0, 1, &pMoonVertexBuffer_, stride, offset);
		devCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

		devCon->VSSetShader(pMoonVS_, nullptr, 0);
		devCon->GSSetShader(pMoonGS_, nullptr, 0);
		devCon->PSSetShader(pMoonPS_, nullptr, 0);

		skyboxInputs[0] = moon_.getSRV();
		devCon->PSSetShaderResources(0, 1, skyboxInputs);

		devCon->Draw(1, 0);

		// reset
		devCon->GSSetShader(nullptr, nullptr, 0);
		devCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
}
