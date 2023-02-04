#pragma once

#include "Global.h"
#include "FRenderTexture.h"

namespace ft_render
{
	using framework::FDepthStencilBuffer;
	using framework::FRenderTexture2D;

	struct GBuffer
	{
		bool initialize(const uint32 width, const uint32 height);
		void cleanup();

		void attach();
		void unbind();
		void clear(bool isReversedDepth, bool clearRTs);

		FRenderTexture2D		rt0_;
		FRenderTexture2D		rt1_;
		FRenderTexture2D		rt2_;
		FRenderTexture2D		rt3_;
		FDepthStencilBuffer		ds_;
	};
}
