#pragma once

#include <Global.h>

// forward declarations
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11ShaderResourceView;

namespace framework
{
	class FExternalTexture
	{
	public:
		FExternalTexture();
		FExternalTexture( ID3D11Device* device, ID3D11DeviceContext* devCon, const string& path );
		~FExternalTexture();

		bool load(ID3D11Device* device, ID3D11DeviceContext* devCon, const string& path);
		void cleanup();

		ID3D11ShaderResourceView* getSRV() const;
		std::string path;

	private:
		struct ID3D11ShaderResourceView* srv_;
	};
}