#pragma once

#include <Global.h>
#include <d3d11.h>
#include <vector>

namespace framework
{
	class FRenderTexture2D
	{
	public:
		FRenderTexture2D();

		void initialize( ID3D11Device* pDevice,
						 UINT width,
						 UINT height,
						 DXGI_FORMAT format,
						 bool bUseUAV = false,
						 bool bGenerateMips = false,
						 UINT numMipLevels = 1 );

		void cleanup();


		ID3D11Texture2D*			tex2D_;
		ID3D11RenderTargetView*		rtv_;
		ID3D11ShaderResourceView*	srv_;
		ID3D11UnorderedAccessView*	uav_;

		UINT width_;
		UINT height_;
	};


	class FDepthStencilBuffer
	{
	public:
		typedef std::vector<ID3D11DepthStencilView*>	TVecDSV;
		typedef std::vector<ID3D11ShaderResourceView*>	TVecSRV;

		FDepthStencilBuffer();
		~FDepthStencilBuffer();

		void initialize( ID3D11Device* pDevice,
						 UINT width,
						 UINT height,
						 DXGI_FORMAT format = DXGI_FORMAT_D24_UNORM_S8_UINT,
						 bool bAsShaderResource = false,
						 UINT multiSamples = 1,
						 UINT msQuality = 0,
						 UINT arraySize = 1 );

		void cleanup();

		operator ID3D11ShaderResourceView*() const { return m_SRV; }
		operator ID3D11DepthStencilView*() const { return m_DSVArraySlices[0]; }

		ID3D11Texture2D* m_texture2D;
		ID3D11DepthStencilView* m_DSV;
		TVecDSV					m_DSVArraySlices;
		ID3D11DepthStencilView* m_DSVReadOnly;
		ID3D11ShaderResourceView* m_SRV;
		TVecSRV						m_SRVArraySlices;
		ID3D11ShaderResourceView* m_SRVStencil;

		UINT width_;
		UINT height_;
	};

} // namespace framework