#ifndef __RENDERERD3D11_H__
#define __RENDERERD3D11_H__

#pragma once

#include <Global.h>
#include <d3d11.h>
#include "FExternalTexture.h"
#include "FMaterial.h"
#include "FRenderTexture.h"
#include "FD3D11CommonStates.h"
#include "FD3D11ConstantBuffer.h"
#include "Maths/Maths.h"

namespace framework
{
	using DirectX::SimpleMath::Vector2;
	using DirectX::SimpleMath::Vector3;

	const float CLEAR_COLOR[4] = { 32.0f / 255.0f, 28.0f / 255.0f, 29.0f / 255.0f, 1.0f };

	struct RenderViewport
	{
		float TopLeftX;
		float TopLeftY;
		float Width;
		float Height;
		float MinDepth;
		float MaxDepth;
	};

	struct SimpleVertex
	{
		Vector3 pos;
		Vector2 uv;
	};

	class FRendererD3D11
	{
	public:
		FRendererD3D11();
		FRendererD3D11(HWND hwnd, int32 width, int32 height, bool fullscreen);

		static FRendererD3D11& getInstance();

		void cleanup();

		void setRenderViewport(const RenderViewport& rvp);
		void setRenderViewports(const RenderViewport* rvps, uint32 numViewports);

		ID3D11Device* getD3D11Device() const;
		ID3D11DeviceContext* getD3D11DeviceContext() const;

		void setAllSamplers();

		uint32 getRendererWidth() const;
		uint32 getRendererHeight() const;

		ID3D11RenderTargetView* getBackbuffer() const;
		void clearBackbuffer();
		void updateViewConstantBuffer(const Vector3& eyePos, const Matrix44& viewMatrix, const Matrix44& projMatrix);
		void updateViewConstantBufferMultiCamera(const Vector3& eyePos1, const Vector3& eyePos2);
		void updateViewConstantBufferShadow(const Matrix44& viewMatrix, const Matrix44& projMatrix);
		void updateStaticObjectConstantBuffer(const Matrix44& worldMatrix, Vector2 uvScale, Vector2 uvOffset, Vector3 colorTint, float specialEffect, float smoothness);

		void swapchainPresent();

		static bool compileD3DShader( const wchar_t* fileName, const char* entryPoint,
									  const char* shaderModel, ID3DBlob** blobOut );

	private:
		static FRendererD3D11* instance_;

		bool initRenderer(HWND hwnd, int32 width, int32 height, bool fullscreen);	


		uint32 rendererWidth_;
		uint32 rendererHeight_;

		D3D_FEATURE_LEVEL featureLevel_;
		ID3D11Device*	device_;
		ID3D11DeviceContext* deviceContext_;
		IDXGISwapChain* swapChain_;

		ID3D11RenderTargetView* pBackbufferRTV_;
		ID3D11Texture2D* pBackbufferDS_;
		ID3D11DepthStencilView* pBackbufferDSView_;

		// Common states
		rendering::FD3D11States* commonStates_;

		// constant buffers
		struct SCBPerView
		{
			Matrix44 matView;
			Matrix44 matProj;
			Matrix44 matViewProj;
			Matrix44 matViewProjInvViewport;
			Matrix44 matDirLightViewProj;
			Vector3 cameraPos_;		float dummy;
			Vector3 cameraPos1_;	float dummy1;
			Vector3 cameraPos2_;	float dummy2;
		};
		framework::D3DConstantBuffer<SCBPerView> cbufferPerView_;

		struct SCBPerStaticObject
		{
			Matrix44 matWorld;

			Vector2 uvScale;
			Vector2 uvOffset;
			Vector3 colorTint;
			float specialEffect = 0.0f;

			float smoothness;
			float padding[3];
		};
		framework::D3DConstantBuffer<SCBPerStaticObject> cbufferPerStaticObject_;

		struct SCBSkinning
		{
			Matrix44 bones[64];
		};
	public:
		framework::D3DConstantBuffer<SCBSkinning> cbufferSkinning_;

	};
}




#endif