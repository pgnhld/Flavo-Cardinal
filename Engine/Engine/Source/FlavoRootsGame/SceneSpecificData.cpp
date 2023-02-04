#include "FlavoRootsGame/SceneSpecificData.h"
#include "DxtkString.h"

void ft_game::to_json(nlohmann::json& json, const PostprocessData& obj) {
	json = {
		{ "tonemappingCurveA_ShoulderStrength_", obj.tonemappingCurveA_ShoulderStrength_ },
		{ "tonemappingCurveB_LinearStrength_", obj.tonemappingCurveB_LinearStrength_ },
		{ "tonemappingCurveC_LinearAngle_", obj.tonemappingCurveC_LinearAngle_ },
		{ "tonemappingCurveD_ToeStrength_", obj.tonemappingCurveD_ToeStrength_ },
		{ "tonemappingCurveE_ToeNumerator_", obj.tonemappingCurveE_ToeNumerator_ },
		{ "tonemappingCurveF_ToeDenominator_", obj.tonemappingCurveF_ToeDenominator_ },
		{ "tonemappingNominatorMultiplier_", obj.tonemappingNominatorMultiplier_ },
		{ "tonemappingWhitePointScale_", obj.tonemappingWhitePointScale_ },
		{ "tonemappingExposureExponent_", obj.tonemappingExposureExponent_ },
		{ "eyeAdaptationMinAllowedLuminance_", obj.eyeAdaptationMinAllowedLuminance_ },
		{ "eyeAdaptationMaxAllowedLuminance_", obj.eyeAdaptationMaxAllowedLuminance_ },
		{ "vignetteColor_", obj.vignetteColor_ },
		{ "vignetteWeights_", obj.vignetteWeights_ },
		{ "vignetteIntensity_", obj.vignetteIntensity_ },
		{ "chromaticAberrationCenter_", obj.chromaticAberrationCenter_ },
		{ "chromaticAberrationIntensity_", obj.chromaticAberrationIntensity_ },
		{ "chromaticAberrationSize_", obj.chromaticAberrationSize_ },
		{ "chromaticAberrationRange_", obj.chromaticAberrationRange_ },
		{ "chromaticAberrationStart_", obj.chromaticAberrationStart_ },
		{ "bloomBlurSigma_", obj.bloomBlurSigma_ },
		{ "bloomThreshold_", obj.bloomThreshold_ },
		{ "bloomMultiplier_", obj.bloomMultiplier_ },
		{ "#skyboxCubemapPath", obj.skyboxCubemapPath_ }
	};
}

void ft_game::from_json(const nlohmann::json& json, PostprocessData& obj) {
	obj.bloomBlurSigma_ = json.at("bloomBlurSigma_");
	obj.bloomThreshold_ = json.at( "bloomThreshold_" );
	obj.bloomMultiplier_ = json.at( "bloomMultiplier_" );

	obj.chromaticAberrationCenter_ = json.at("chromaticAberrationCenter_").get<Vector2>();
	obj.chromaticAberrationIntensity_ = json.at("chromaticAberrationIntensity_");
	obj.chromaticAberrationSize_ = json.at( "chromaticAberrationSize_" );
	obj.chromaticAberrationRange_ = json.at( "chromaticAberrationRange_" );
	obj.chromaticAberrationStart_ = json.at( "chromaticAberrationStart_" );

	obj.tonemappingCurveA_ShoulderStrength_ = json.at("tonemappingCurveA_ShoulderStrength_");
	obj.tonemappingCurveB_LinearStrength_ = json.at( "tonemappingCurveB_LinearStrength_" );
	obj.tonemappingCurveC_LinearAngle_ = json.at( "tonemappingCurveC_LinearAngle_" );
	obj.tonemappingCurveD_ToeStrength_ = json.at( "tonemappingCurveD_ToeStrength_" );
	obj.tonemappingCurveE_ToeNumerator_ = json.at( "tonemappingCurveE_ToeNumerator_" );
	obj.tonemappingCurveF_ToeDenominator_ = json.at( "tonemappingCurveF_ToeDenominator_" );
	obj.tonemappingNominatorMultiplier_ = json.at( "tonemappingNominatorMultiplier_" );
	obj.tonemappingWhitePointScale_ = json.at( "tonemappingWhitePointScale_" );
	obj.tonemappingExposureExponent_ = json.at( "tonemappingExposureExponent_" );

	obj.eyeAdaptationMinAllowedLuminance_ = json.at( "eyeAdaptationMinAllowedLuminance_" );
	obj.eyeAdaptationMaxAllowedLuminance_ = json.at( "eyeAdaptationMaxAllowedLuminance_" );

	obj.vignetteColor_ = json.at("vignetteColor_").get<Vector3>();
	obj.vignetteWeights_ = json.at("vignetteWeights_").get<Vector3>();
	obj.vignetteIntensity_ = json.at("vignetteIntensity_");

	obj.skyboxCubemapPath_ = json.at("#skyboxCubemapPath").get<std::string>();
}

nlohmann::json ft_game::SceneSpecificData::serialize() {
	return {
		{ "postprocess", postprocess },
		{ "backgroundMusic", backgroundMusic }
	};
}

void ft_game::SceneSpecificData::deserialize(const nlohmann::json& json) {
	postprocess = json.at("postprocess").get<PostprocessData>();
	backgroundMusic = json.at("backgroundMusic");
}
