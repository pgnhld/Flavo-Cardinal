#pragma once

#include "Global.h"
#include "FRenderTexture.h"
#include "FExternalTexture.h"

// Screen Space Ambient Occlusion
namespace ft_render
{
	class FAmbientOcclusionSSAO
	{
	public:
		FAmbientOcclusionSSAO();
		~FAmbientOcclusionSSAO();

		bool initialize(const uint32 fullscreenWidth, const uint32 fullscreenHeight);
		void release();

		void renderSSAO(const framework::FRenderTexture2D& texWorldPos, const framework::FRenderTexture2D& texNormals);

		const framework::FRenderTexture2D& getSSAOTexture() const;

	private:
		framework::FRenderTexture2D		texSSAO_;
		framework::FExternalTexture		texSSAONoise_;

		ID3D11PixelShader*				psSSAO_;
		ID3D11PixelShader*				psSSAOBlur_;
	};
}