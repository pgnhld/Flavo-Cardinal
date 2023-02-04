#include "FExternalTexture.h"
#include <d3d11.h>
#include <WICTextureLoader.h>
#include <DDSTextureLoader.h>
#include "Logger.h"

framework::FExternalTexture::FExternalTexture()
	: path("-"), srv_(nullptr) {

}

framework::FExternalTexture::FExternalTexture(ID3D11Device* device, ID3D11DeviceContext* devCon, const string& path) {
	load(device, devCon, path);
}

framework::FExternalTexture::~FExternalTexture() {
	cleanup();
}

bool framework::FExternalTexture::load(ID3D11Device* device, ID3D11DeviceContext* devCon, const string& path) {
	SAFE_RELEASE(srv_);
	this->path = path;

	wchar_t unicodeString[MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, path.c_str(), -1, unicodeString, 260);

	HRESULT hr = S_OK;

	if (path.find(".dds") != std::string::npos) {
		hr = DirectX::CreateDDSTextureFromFile(device, devCon, unicodeString, nullptr, &srv_);

		if (FAILED(hr)) {
			// unable to load dds texture
			return false;
		}
	} else {
		hr = DirectX::CreateWICTextureFromFile(device, devCon, unicodeString, nullptr, &srv_);

		if (FAILED(hr)) {
			// unable to load jpg/png/bmp texture
			return false;
		}
	}

	return true;
}

void framework::FExternalTexture::cleanup() {
	SAFE_RELEASE(srv_);
}

ID3D11ShaderResourceView* framework::FExternalTexture::getSRV() const {
	return srv_;
}

