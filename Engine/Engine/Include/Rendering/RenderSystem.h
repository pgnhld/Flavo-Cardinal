#pragma once

#include "Global.h"
#include <EECS.h>
#include "FRendererD3D11.h"
#include "FExternalTexture.h"
#include "FD3D11ConstantBuffer.h"
#include "DeferredShading.h"
#include "FullscreenPass.h"
#include "Rendering/StaticMeshRenderer.h"
#include "RenderSystemEyeAdaptation.h"
#include "RenderSystemSSAO.h"
#include "RenderSystemSkybox.h"
#include "EngineEvent.h"

namespace ft_engine {
	class Transform;
}

FLAVO_SYSTEM(ft_render, RenderSystem)
namespace ft_render
{
	class Camera;

	class RenderSystem : public eecs::System<RenderSystem>, public eecs::IInvoker, public eecs::IReceiver<RenderSystem>
	{
	public:
		RenderSystem();
		~RenderSystem();

		void update(eecs::EntityManager& entities, double deltaTime) override;

	private:
		void onPostSceneLoaded(const EventPostSceneLoaded& event);

		void prepareDeferredShadingPass();
		void setupLighting(eecs::EntityManager& entities);

		typedef std::vector<eecs::Entity> TVecEntities;
		void renderShadows( TVecEntities& staticMeshes, const uint32 staticMeshesCount,
							TVecEntities& skinnedMeshes, const uint32 skinnedMeshesCount);

		void moveCamera(ft_render::Camera* camera, ft_engine::Transform* transform);

		bool checkIntersection(const DirectX::BoundingFrustum& boundingFrustrum, const DirectX::BoundingBox& boundingBox);

		struct SRenderPointLight
		{
			SRenderPointLight()
			{
				radius=intensity=0.0f;
			}

			Vector3 position;
			float radius;
			Vector3 attenuation;
			float intensity;

			Vector3 color;
			float pad;
		};

		
		struct SRenderCylinderLight
		{
			Vector3 posStart;
			float intensity;

			Vector3 posEnd;
			float attenuation;

			Vector3 color;
			float radius;
		};

		struct SCBLightingTemp
		{
			uint32 numCylinderLights = 0;
			Vector3 dirLight;

			Vector3 dirLightColor;
			float dirLightIntensity;

			SRenderCylinderLight cylinderLights[16];
		};

		struct SCBPerFrame
		{
			Vector4 viewportSize;
			float fElapsedTime;
			Vector3 dummy;
		};
		

		struct SCBPostprocess
		{
			Vector4 tonemappingCurveABCD;

			Vector2 tonemappingCurveEF;
			float tonemappingNominatorMultiplier;
			float tonemappingWhitePointScale;

			float tonemappingExposureExponent;
			float eyeAdaptationMinAllowedLuminance;
			float eyeAdaptationMaxAllowedLuminance;
			float bloomBlurSigma;

			Vector3 vignetteColor;
			float vignetteIntensity;

			Vector3 vignetteWeights;
			float chromaticAberrationIntensity;

			Vector2 chromaticAberrationCenter;
			float chromaticAberrationSize;
			float chromaticAberrationRange;

			float chromaticAberrationStart;
			float bloomThreshold;
			float bloomMultiplier;
			float xpadding_postprocess;
		};


	private:

		struct AlphaPosition;

		framework::RenderViewport renderViewports_[2];

		// Deferred Shading
		GBuffer		gBuffer_;
		ID3D11VertexShader* gbufferFeedingVS_;
		ID3D11VertexShader* gbufferFeedingSkinnedVS_;
		ID3D11PixelShader* gbufferFeedingPS_;
		ID3D11PixelShader* deferredShadingSimplePS_;
		ID3D11InputLayout* inputLayoutStaticGeometry_;
		ID3D11InputLayout* inputLayoutSkinnedGeometry_;
		framework::FExternalTexture texNormalsFitting_;

		// Forward pass
		ID3D11BlendState* blendStateForwardPassTransparency_;
		ID3D11PixelShader* forwardPassSimplePS_;
		ID3D11PixelShader* forwardPassHologramPS_;
		ID3D11PixelShader* waterPS_;
		ID3D11BlendState* blendStateForwardPassTransparencyFromAlpha_;

		// shadows
		Vector3		globalDirectionaLightDirection_;
		bool		isDirLightPresent_;
		framework::FDepthStencilBuffer shadowMap_;
		uint32		shadowMapResolution_ = 4096;
		ID3D11VertexShader* depthOnlyStaticMeshesVS_;
		ID3D11VertexShader* depthOnlySkinnedMeshesVS_;
		ID3D11RasterizerState* rasterizerStateShadows;
		Vector3 sceneAABBMin;
		Vector3 sceneAABBMax;

		// Skybox
		FSkybox		skybox_;


		// Lighting (todo: tiled lighting)
		framework::D3DConstantBuffer<SCBLightingTemp> cbufferLighting_;

		framework::D3DConstantBuffer<SCBPerFrame> cbufferPerFrame_;

		// HDR buffer
		framework::FRenderTexture2D hdrColorBuffer_;
		framework::FRenderTexture2D colorBufferAfterTonemapping_;
		ID3D11PixelShader*	pixelShaderTonemapping_;

		// eye adaptation
		FEyeAdaptation		eyeAdaptation_[2];
		ID3D11PixelShader*	pixelShaderLogLuminance_Split[2];
		ID3D11PixelShader*	pixelShaderLogLuminance_Full;

		// bloom
		framework::FRenderTexture2D bloomThresholdFullOutput;
		framework::FRenderTexture2D	bloomColorBuffer_half[2];
		ID3D11PixelShader* bloomThresholdPS;
		ID3D11PixelShader* bloomGaussianBlurVPS;
		ID3D11PixelShader* bloomGaussianBlurHPS;

		// Lens flares
		framework::FRenderTexture2D lensFlaresTex[2];
		ID3D11PixelShader* lensFlaresPS;
		ID3D11PixelShader* lensFlaresThresholdPS;

		// AA
		ID3D11PixelShader* fxaaPS;

		ID3D11PixelShader*	pixelShaderSimpleCopy_;
		ID3D11PixelShader*	pixelShaderFinalPass_;

		FAmbientOcclusionSSAO	ao_;

		ID3D11DepthStencilState*	depthStencilState_NoDepthWrite;
		ID3D11DepthStencilState*	depthStencilState_Skybox;


		// for tonemapping and per-player viewport
		uint32				numPlayersViewports;

		D3DConstantBuffer<SCBPostprocess> cbufferPostprocess_;
		bool requestSceneSpecificDataUpdate_ = true;


		float				elapsedTime = 0.0f;
		uint32				numFrames = 0;

		Vector3				eyePosPlayers_[2];

		// Additional effects
		ID3D11PixelShader*	fullscreenGlitchPS_;
	};

}
