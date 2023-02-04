#include "FRenderTexture.h"

framework::FRenderTexture2D::FRenderTexture2D()
	: tex2D_(nullptr)
	, rtv_(nullptr)
	, srv_(nullptr)
	, uav_(nullptr) {

}

void framework::FRenderTexture2D::initialize(ID3D11Device* pDevice, UINT width, UINT height, DXGI_FORMAT format,
	bool bUseUAV /*= false*/, bool bGenerateMips /*= false*/, UINT numMipLevels /*= 1 */) {
	width_ = width;
	height_ = height;

	UINT bindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	if (bUseUAV) {
		bindFlags |= D3D11_BIND_UNORDERED_ACCESS;
	}

	UINT miscFlags = 0;
	if (bGenerateMips && numMipLevels != 1) {
		miscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
	}

	D3D11_TEXTURE2D_DESC tex2dDesc;
	memset(&tex2dDesc, 0, sizeof(tex2dDesc));

	tex2dDesc.ArraySize = 1;
	tex2dDesc.BindFlags = bindFlags;
	tex2dDesc.Usage = D3D11_USAGE_DEFAULT;
	tex2dDesc.Format = format;
	tex2dDesc.Width = width_;
	tex2dDesc.Height = height_;
	tex2dDesc.MipLevels = numMipLevels;
	tex2dDesc.SampleDesc.Count = 1;
	tex2dDesc.MiscFlags = miscFlags;

	HRESULT hr;
	hr = pDevice->CreateTexture2D(&tex2dDesc, nullptr, &tex2D_);
	if (FAILED(hr)) {
		return;
	}

	// Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	rtvDesc.Format = tex2dDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	pDevice->CreateRenderTargetView(tex2D_, &rtvDesc, &rtv_);
	if (FAILED(hr)) {
		return;
	}

	// Shader Resource View
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = tex2dDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = -1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	pDevice->CreateShaderResourceView(tex2D_, &srvDesc, &srv_);
	if (FAILED(hr)) {
		return;
	}

	// Unordered Access View
	if (bUseUAV) {
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		uavDesc.Format = tex2dDesc.Format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;

		pDevice->CreateUnorderedAccessView(tex2D_, &uavDesc, &uav_);
		if (FAILED(hr)) {
			return;
		}
	}
}

void framework::FRenderTexture2D::cleanup() {
	SAFE_RELEASE(srv_);
	SAFE_RELEASE(rtv_);
	SAFE_RELEASE(uav_);
	SAFE_RELEASE(tex2D_);
}

// Depth/Stencil buffer
framework::FDepthStencilBuffer::FDepthStencilBuffer()
	: m_texture2D(nullptr)
	, m_DSV(nullptr)
	, m_DSVReadOnly(nullptr)
	, m_SRV(nullptr)
	, m_SRVStencil(nullptr) {
	width_ = height_ = 0;
}

framework::FDepthStencilBuffer::~FDepthStencilBuffer() {

}

void framework::FDepthStencilBuffer::initialize(ID3D11Device* pDevice, UINT width, UINT height, DXGI_FORMAT format /*= DXGI_FORMAT_D24_UNORM_S8_UINT*/,
	bool bAsShaderResource /*= false*/, UINT multiSamples /*= 1*/, UINT msQuality /*= 0*/, UINT arraySize /*= 1 */) {
	width_ = width;
	height_ = height;


	UINT BindFlags = D3D11_BIND_DEPTH_STENCIL;
	if (bAsShaderResource == true) {
		BindFlags |= D3D11_BIND_SHADER_RESOURCE;
	}

	DXGI_FORMAT dsTexFormat;
	if (!bAsShaderResource)
		dsTexFormat = format;
	else if (format == DXGI_FORMAT_D16_UNORM)
		dsTexFormat = DXGI_FORMAT_R16_TYPELESS;
	else if (format == DXGI_FORMAT_D24_UNORM_S8_UINT)
		dsTexFormat = DXGI_FORMAT_R24G8_TYPELESS;
	else
		dsTexFormat = DXGI_FORMAT_R32_TYPELESS;

	D3D11_TEXTURE2D_DESC Tex2DDesc;
	memset(&Tex2DDesc, 0, sizeof(Tex2DDesc));
	Tex2DDesc.ArraySize = arraySize;
	Tex2DDesc.Width = width;
	Tex2DDesc.Height = height;
	Tex2DDesc.Format = dsTexFormat;
	Tex2DDesc.CPUAccessFlags = 0;
	Tex2DDesc.MipLevels = 1;
	Tex2DDesc.MiscFlags = 0;
	Tex2DDesc.BindFlags = BindFlags;
	Tex2DDesc.SampleDesc.Count = multiSamples;
	Tex2DDesc.SampleDesc.Quality = msQuality;
	Tex2DDesc.Usage = D3D11_USAGE_DEFAULT;

	pDevice->CreateTexture2D(&Tex2DDesc, nullptr, &m_texture2D);

	// Create DSV
	HRESULT hr = E_FAIL;
	m_DSVArraySlices.clear();
	for (uint32 i = 0; i < arraySize; ++i) {
		ID3D11DepthStencilView* pArraySliceDSV = nullptr;

		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Format = format;
		dsvDesc.Flags = 0;

		if (arraySize == 1) {
			dsvDesc.Texture2D.MipSlice = 0;
			dsvDesc.ViewDimension = (multiSamples > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
		}

		else {
			dsvDesc.ViewDimension = (multiSamples > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMSARRAY : D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
			dsvDesc.Texture2DArray.ArraySize = 1;
			dsvDesc.Texture2DArray.FirstArraySlice = i;
			dsvDesc.Texture2DArray.MipSlice = 0;
		}

		hr = pDevice->CreateDepthStencilView(m_texture2D, &dsvDesc, &pArraySliceDSV);
		if (FAILED(hr)) {
			return;
		}

		m_DSVArraySlices.push_back(pArraySliceDSV);

		if (i == 0) {
			// Assign top level array slice DSV
			m_DSV = m_DSVArraySlices[0];

			// Create read-only DSV also
			dsvDesc.Flags = D3D11_DSV_READ_ONLY_DEPTH;
			if (format == DXGI_FORMAT_D24_UNORM_S8_UINT || format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT)
				dsvDesc.Flags |= D3D11_DSV_READ_ONLY_DEPTH;
			hr = pDevice->CreateDepthStencilView(m_texture2D, &dsvDesc, &m_DSVReadOnly);
			if (FAILED(hr)) {
				return;
			}
		}
	}

	// Create SRV if requested for Depth Buffer
	if (bAsShaderResource) {
		DXGI_FORMAT dsSRVFormat;
		if (format == DXGI_FORMAT_D16_UNORM)
			dsSRVFormat = DXGI_FORMAT_R16_UNORM;
		else if (format == DXGI_FORMAT_D24_UNORM_S8_UINT)
			dsSRVFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		else
			dsSRVFormat = DXGI_FORMAT_R32_FLOAT;

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = dsSRVFormat;

		if (arraySize == 1) {
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.ViewDimension = (multiSamples > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
		} else {
			srvDesc.ViewDimension = (multiSamples > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY : D3D11_SRV_DIMENSION_TEXTURE2DARRAY;

			srvDesc.Texture2DArray.ArraySize = arraySize;
			srvDesc.Texture2DArray.MipLevels = 1;
			srvDesc.Texture2DArray.MostDetailedMip = 0;
			srvDesc.Texture2DArray.FirstArraySlice = 0;
		}

		hr = pDevice->CreateShaderResourceView(m_texture2D, &srvDesc, &m_SRV);
		if (FAILED(hr)) {
			return;
		}


		// Create SRV for each slice separately
		m_SRVArraySlices.clear();
		for (uint32 i = 0; i < arraySize; ++i) {
			ID3D11ShaderResourceView* pSliceSRV = nullptr;

			if (arraySize == 1) {
				srvDesc.Texture2D.MipLevels = 1;
				srvDesc.Texture2D.MostDetailedMip = 0;
				srvDesc.ViewDimension = (multiSamples > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
			} else {
				srvDesc.ViewDimension = (multiSamples > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY : D3D11_SRV_DIMENSION_TEXTURE2DARRAY;

				srvDesc.Texture2DArray.ArraySize = 1;
				srvDesc.Texture2DArray.MipLevels = 1;
				srvDesc.Texture2DArray.MostDetailedMip = 0;
				srvDesc.Texture2DArray.FirstArraySlice = i;
			}

			hr = pDevice->CreateShaderResourceView(m_texture2D, &srvDesc, &pSliceSRV);
			if (FAILED(hr)) {
				return;
			}
			m_SRVArraySlices.push_back(pSliceSRV);
		}
	} else {
		m_SRV = nullptr;
	}


	// Create SRV for Stencil Buffer
	if (format == DXGI_FORMAT_D24_UNORM_S8_UINT) {
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_X24_TYPELESS_G8_UINT;

		if (arraySize == 1) {
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.ViewDimension = (multiSamples > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DMS : D3D11_SRV_DIMENSION_TEXTURE2D;
		}

		hr = pDevice->CreateShaderResourceView(m_texture2D, &srvDesc, &m_SRVStencil);
		if (FAILED(hr)) {
			return;
		}
	} else {
		m_SRVStencil = nullptr;
	}
}

void framework::FDepthStencilBuffer::cleanup() {
	for (auto i : m_DSVArraySlices) {
		SAFE_RELEASE(i);
	}

	for (auto i : m_SRVArraySlices) {
		SAFE_RELEASE(i);
	}

	SAFE_RELEASE(m_DSVReadOnly);
	SAFE_RELEASE(m_SRV);
	SAFE_RELEASE(m_SRVStencil);
	SAFE_RELEASE(m_texture2D);
}
