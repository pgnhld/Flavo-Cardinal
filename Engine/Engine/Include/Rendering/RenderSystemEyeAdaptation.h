#pragma once

#include "Global.h"
#include "FRenderTexture.h"

// Average luminance and eye adaptation
namespace ft_render
{
	class FEyeAdaptation
	{
	public:
		FEyeAdaptation();
		~FEyeAdaptation();

		bool initialize(const uint32 fullscreenWidth, const uint32 fullscreenHeight, const uint32 luminanceMips = 10);
		void release();

		void renderAverageLuminanceAndEyeAdaptation(const framework::FRenderTexture2D& inputColor);

		const framework::FRenderTexture2D& getAdaptedLuminanceTexture() const;

	private:
		void renderAverageLuminance(const framework::FRenderTexture2D& inputColor);
		void renderEyeAdaptation();

		framework::FRenderTexture2D		texLogLuminance_;
		framework::FRenderTexture2D		texAdaptedLuminance_[2];
		uint32							currentRenderLumTarget_;

		ID3D11PixelShader*				psAdaptedLuminance_ = nullptr;
	};
}