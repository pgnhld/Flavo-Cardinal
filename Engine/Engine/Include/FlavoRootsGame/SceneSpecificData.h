#pragma once

#include "Global.h"
#include "EECS.h"
#include "FAudio.h"
#include "Maths/Maths.h"

FLAVO_COMPONENT(ft_game, SceneSpecificData)
namespace ft_game
{
	struct PostprocessData
	{
		PostprocessData() 
		{
			tonemappingCurveA_ShoulderStrength_ = 0.30f;
			tonemappingCurveB_LinearStrength_ = 0.30f;
			tonemappingCurveC_LinearAngle_ = 0.10f;
			tonemappingCurveD_ToeStrength_ = 0.20f;
			tonemappingCurveE_ToeNumerator_ = 0.01f;
			tonemappingCurveF_ToeDenominator_ = 0.30f;

			tonemappingWhitePointScale_ = 1.5f;
			tonemappingNominatorMultiplier_ = 1.05f;
			tonemappingExposureExponent_ = 0.45f;

			eyeAdaptationMinAllowedLuminance_ = 0.01f;
			eyeAdaptationMaxAllowedLuminance_ = 1.25f;

			vignetteColor_ = Vector3(0.04706f, 0.04706f, 0.04706f);
			vignetteWeights_ = Vector3(0.8f, 0.8f, 0.7f);
			vignetteIntensity_ = 1.0f;

			chromaticAberrationCenter_ = Vector2(0.5f, 0.5f);
			chromaticAberrationIntensity_ = 5.0f;
			chromaticAberrationRange_ = 1.25f;
			chromaticAberrationSize_ = 0.75f;
			chromaticAberrationStart_ = 0.25f;

			bloomBlurSigma_ = 1.5f;
			bloomThreshold_ = 3.f;
			bloomMultiplier_ = 0.6f;
		}

		// Tonemapping
		float tonemappingCurveA_ShoulderStrength_;
		float tonemappingCurveB_LinearStrength_;
		float tonemappingCurveC_LinearAngle_;
		float tonemappingCurveD_ToeStrength_;
		float tonemappingCurveE_ToeNumerator_;
		float tonemappingCurveF_ToeDenominator_;

		float tonemappingNominatorMultiplier_;
		float tonemappingWhitePointScale_;
		float tonemappingExposureExponent_;

		// Eye adaptation
		float eyeAdaptationMinAllowedLuminance_;
		float eyeAdaptationMaxAllowedLuminance_;

		// Vignette
		Vector3 vignetteColor_;
		Vector3 vignetteWeights_;
		float vignetteIntensity_;

		// Chromatic aberration
		Vector2 chromaticAberrationCenter_;
		float chromaticAberrationIntensity_;
		float chromaticAberrationSize_;
		float chromaticAberrationRange_;
		float chromaticAberrationStart_;

		// bloom
		float bloomBlurSigma_;
		float bloomThreshold_;
		float bloomMultiplier_;

		// skybox cubemap
		std::string skyboxCubemapPath_;
	};

	void to_json(nlohmann::json& json, const PostprocessData& obj);
	void from_json(const nlohmann::json& json, PostprocessData& obj);

	class SceneSpecificData : public eecs::Component<SceneSpecificData>
	{
	public:
		nlohmann::json serialize() override;
		void deserialize(const nlohmann::json& json) override;

		PostprocessData postprocess;
		framework::BackgroundMusicType backgroundMusic;
	};
}