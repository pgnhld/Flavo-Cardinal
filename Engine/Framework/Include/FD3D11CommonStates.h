#pragma once

#include <Global.h>
#include <d3d11.h>

namespace framework
{
	namespace rendering
	{
		class FD3D11States
		{
		public:

			static void Claim();
			static void Destroy();
			static FD3D11States* Get();

			ID3D11RasterizerState*		pBackfaceCull_RS;
			ID3D11RasterizerState*		pBackfaceCullMSAA_RS;
			ID3D11RasterizerState*		pBackfaceCull_WireFrame_RS;
			ID3D11RasterizerState*		pBackfaceCull_WireFrameMSAA_RS;
			ID3D11RasterizerState*		pNoCull_RS;

			ID3D11DepthStencilState*	pDepthNoStencil_DS;
			ID3D11DepthStencilState*	pNoDepthNoStencil_DS;
			ID3D11DepthStencilState*	pStencilSetBits_DS;

			ID3D11DepthStencilState*	pNoDepthStencilComplex_DS;
			ID3D11DepthStencilState*	pDepthStencilComplex_DS;
			ID3D11DepthStencilState*	pDS_Skybox;

			ID3D11DepthStencilState*	pDS_DepthGreater;

			ID3D11BlendState*			pNoBlend_BS;
			ID3D11BlendState*			pBlend_BS;
			ID3D11BlendState*			pBlendAdditive_BS;
			ID3D11BlendState*			pBlendAlphaToCoverage_BS;
			ID3D11BlendState*			pBS_Blend_NoColorWrite;


			ID3D11SamplerState*			pSamplerPointWrap;
			ID3D11SamplerState*			pSamplerPointClamp;
			ID3D11SamplerState*			pSamplerLinearWrap;
			ID3D11SamplerState*			pSamplerLinearClamp;
			ID3D11SamplerState*			pSamplerAnisoWrap;
			ID3D11SamplerState*			pSamplerAnisoClamp;
			ID3D11SamplerState*			pSamplerComparisonLinear;

			void CreateResources( ID3D11Device *pd3dDevice );
			void ReleaseResources();

		private:
			void CreateSamplers( ID3D11Device *pd3dDevice );
			void CreateRasterizerStates( ID3D11Device *pd3dDevice );
			void CreateDepthStencilStates( ID3D11Device *pd3dDevice );
			void CreateBlendStates( ID3D11Device *pd3dDevice );

			FD3D11States();
			~FD3D11States();
		};
	}
}