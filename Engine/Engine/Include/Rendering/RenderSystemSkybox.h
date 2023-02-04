#pragma once

#include "Global.h"
#include "FRenderTexture.h"
#include "FExternalTexture.h"
#include "Maths/Maths.h"


// Skybox
namespace ft_render
{
	class FSkybox
	{
		struct SkyboxVertex
		{
			Vector3	position;
		};

	public:
		FSkybox();
		~FSkybox();

		bool initialize();
		void release();

		void renderSkybox();

	private:
		ID3D11Buffer*					pSkyboxVertexBuffer_;
		ID3D11InputLayout*				pSkyboxInputLayout_;

		ID3D11VertexShader*				pSkyboxVS_;
		ID3D11PixelShader*				pSkyboxPS_;

		ID3D11Buffer*					pMoonVertexBuffer_;
		ID3D11VertexShader*				pMoonVS_;
		ID3D11GeometryShader*			pMoonGS_;
		ID3D11PixelShader*				pMoonPS_;

		framework::FExternalTexture		cubemap_;
		framework::FExternalTexture		moon_;
	};
}