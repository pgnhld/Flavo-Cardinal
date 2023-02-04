#pragma once

#include "Global.h"
#include "FRenderTexture.h"

namespace ft_render
{
	class FullscreenPass
	{
	public:
		FullscreenPass();

		static FullscreenPass& getInstance();

		bool initialize();
		void cleanup();

		void renderFullscreen(const uint32 width, const uint32 height, const uint32 viewportTopLeftX = 0, const uint32 viewportTopLeftY = 0);

	private:
		void drawFullscreenTriangle();

		ID3D11VertexShader* fullscreenVS_;
	};
}
