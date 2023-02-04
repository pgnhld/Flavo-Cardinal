#pragma once

#include "Global.h"
#include "EECS.h"
#include <d3d11.h>
#include "FMesh.h"
#include "FMaterial.h"
#include <Maths/Maths.h>
#include <assimp/scene.h>
#include "CoroutineManager.h"

FLAVO_COMPONENT(ft_render, SkinnedMeshRenderer)
namespace ft_render
{
	using namespace framework;
	using DirectX::SimpleMath::Matrix;

	class SkinnedMeshRenderer : public eecs::Component<SkinnedMeshRenderer>
	{
	public:
		enum class ECrossFadeType;

		SkinnedMeshRenderer();
		~SkinnedMeshRenderer();

		void changeAnimation(uint32 newIndex, float animationSpeed = 1.0f);

		void reloadMesh(const FMeshIdentifier& newIdentifier);
		void replaceMaterial(const FMaterial& newMaterial);
		void setEnabledBoth(bool bEnabled);

		nlohmann::json serialize() override;
		void deserialize(const nlohmann::json& json) override;

		void render(); //consider moving it to some System
		void renderNoSkinning();

		void boneTransform(std::vector<Matrix>& vMatrices);

		int32 currentAnimationIndex;
		float currentAnimationSpeed_;
		//If equal to currentAnimationIndex then no blending is currently being applied
		int32 previousAnimationIndex;
		float previousAnimationSpeed_;
		Coroutine blendingCoroutine;

		ECrossFadeType crossFadeType;
		//first index -> starting anim, second index -> ending anim
		std::vector<std::vector<float>> blendingDurationMatrix;
		//-1 means to skip neutral animation
		std::vector<std::vector<int>> blendingNeutralPoseMatrix;

		float animationSliceTime_;
		bool bAnimationEnabled;
		bool bEnabledOwn;
		bool bEnabledOther;
		std::vector<Matrix> additionalBoneOffsets;

		// per-component skinning data
		std::vector<FBoneInfo> thisMeshBoneInfo_;

	private:
		//Typical animation bone transform calculations
		void readNodeHierarchy(float animationTime, const aiNode* pNode, const Matrix& parentTransform);
		//Blending animation bone transform calculations
		void readNodeHierarchy(float animationTime, const aiNode* pNode, const Matrix& parentTransform, uint32 animationIndex, std::vector<Matrix>& finalTransformMatrices);
		uint32 findRotation(float animationTime, const aiNodeAnim* pNodeAnim);
		uint32 findScaling(float animationTime, const aiNodeAnim* pNodeAnim);
		uint32 findPosition(float animationTime, const aiNodeAnim* pNodeAnim);
		void calcInterpolatedRotation(aiQuaternion& out, float animationTime, const aiNodeAnim* pNodeAnim);
		void calcInterpolatedScaling(aiVector3D& out, float animationTime, const aiNodeAnim* pNodeAnim);
		void calcInterpolatedPosition(aiVector3D& out, float animationTime, const aiNodeAnim* pNodeAnim);

		IEnumerator blendAnimations(CoroutineArg arg);

	private:
		friend class RenderSystem;

		FMesh* mesh_;
		FMaterial material_;
	};

	enum class SkinnedMeshRenderer::ECrossFadeType
	{
		LINEAR,
		SMOOTH_STOP_2,
		SMOOTH_STOP_3,
		SMOOTH_STOP_4,
		SMOOTH_START_2,
		SMOOTH_START_3,
		SMOOTH_START_4
	};
}