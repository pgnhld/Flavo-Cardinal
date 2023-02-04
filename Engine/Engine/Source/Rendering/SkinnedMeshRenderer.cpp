#include "Rendering/SkinnedMeshRenderer.h"
#include "FRendererD3D11.h"
#include "FResourceManager.h"
#include "Assertion.h"
#include "FLoader.h"
#include "Logger.h"

using DirectX::SimpleMath::Matrix;
Matrix getMatrix44FromAssimp(const aiMatrix4x4& AssimpMatrix) {
	Matrix m;
	m._11 = AssimpMatrix.a1; m._12 = AssimpMatrix.a2; m._13 = AssimpMatrix.a3; m._14 = AssimpMatrix.a4;
	m._21 = AssimpMatrix.b1; m._22 = AssimpMatrix.b2; m._23 = AssimpMatrix.b3; m._24 = AssimpMatrix.b4;
	m._31 = AssimpMatrix.c1; m._32 = AssimpMatrix.c2; m._33 = AssimpMatrix.c3; m._34 = AssimpMatrix.c4;
	m._41 = AssimpMatrix.d1; m._42 = AssimpMatrix.d2; m._43 = AssimpMatrix.d3; m._44 = AssimpMatrix.d4;

	m = m.Transpose();

	return m;
}

Matrix getMatrix44FromAssimp(const aiMatrix3x3& AssimpMatrix) {
	Matrix m;
	m._11 = AssimpMatrix.a1; m._12 = AssimpMatrix.a2; m._13 = AssimpMatrix.a3; m._14 = 0.0f;
	m._21 = AssimpMatrix.b1; m._22 = AssimpMatrix.b2; m._23 = AssimpMatrix.b3; m._24 = 0.0f;
	m._31 = AssimpMatrix.c1; m._32 = AssimpMatrix.c2; m._33 = AssimpMatrix.c3; m._34 = 0.0f;
	m._41 = 0.0f;			 m._42 = 0.0f;			  m._43 = 0.0f;			   m._44 = 1.f;

	m = m.Transpose();

	return m;
}

ft_render::SkinnedMeshRenderer::SkinnedMeshRenderer()
	: currentAnimationIndex(0), currentAnimationSpeed_(1.0f), previousAnimationIndex(0), previousAnimationSpeed_(1.0f), animationSliceTime_(0.0), bAnimationEnabled(false), bEnabledOwn(true), bEnabledOther(true), mesh_(nullptr) {

}

ft_render::SkinnedMeshRenderer::~SkinnedMeshRenderer() {
	// todo: it's never called.

}

void ft_render::SkinnedMeshRenderer::changeAnimation(uint32 newIndex, float animationSpeed) {
	if (!mesh_ || newIndex == currentAnimationIndex) {
		return;
	}

	if (ASSERT_FAIL(newIndex < mesh_->assimpImporter->pScene->mNumAnimations, "Requested anim index is too large")) {
		return;
	}

	if (previousAnimationIndex != currentAnimationIndex)
		STOP_COROUTINE(blendingCoroutine);

	previousAnimationSpeed_ = currentAnimationSpeed_;
	currentAnimationSpeed_ = animationSpeed;

	previousAnimationIndex = currentAnimationIndex;
	currentAnimationIndex = newIndex;

	animationSliceTime_ = 0.0f;
	bAnimationEnabled = true;

	blendingCoroutine = START_COROUTINE(
		&SkinnedMeshRenderer::blendAnimations,
		void*,
		nullptr
	);
}

void ft_render::SkinnedMeshRenderer::reloadMesh(const framework::FMeshIdentifier& newIdentifier) {
	if (mesh_ != nullptr && mesh_->identifier == newIdentifier)
		return;

	FResourceManager& resourceManager = FResourceManager::getInstance();
	MeshTypeLoadInfo meshInfo;
	mesh_ = resourceManager.getMesh(newIdentifier, meshInfo);

	// copy data for skinning
	const uint32 bonesInfoSize = mesh_->boneInfo_.size();
	this->thisMeshBoneInfo_.resize(bonesInfoSize);

	for (uint32 i = 0; i < bonesInfoSize; i++) {
		//this->thisMeshBoneInfo_[i].boneOffset_ = mesh_->boneInfo_[i].boneOffset_;
		//this->thisMeshBoneInfo_[i].finalTransform_ = mesh_->boneInfo_[i].finalTransform_;
		memcpy(&thisMeshBoneInfo_[i].boneOffset_, &mesh_->boneInfo_[i].boneOffset_, sizeof(Matrix44));
		memcpy(&thisMeshBoneInfo_[i].finalTransform_, &mesh_->boneInfo_[i].finalTransform_, sizeof(Matrix44));
	}

	additionalBoneOffsets = std::vector<Matrix>(bonesInfoSize);
}

void ft_render::SkinnedMeshRenderer::replaceMaterial(const framework::FMaterial& newMaterial) {
	material_ = FMaterial(newMaterial);
}

void ft_render::SkinnedMeshRenderer::setEnabledBoth(bool bEnabled) {
	bEnabledOther = bEnabled;
	bEnabledOwn = bEnabled;
}

nlohmann::json ft_render::SkinnedMeshRenderer::serialize() {
	return {
		{ "#meshPath", (mesh_ != nullptr) ? mesh_->identifier.filePath : "-" },
		{ "meshIndex", (mesh_ != nullptr) ? mesh_->identifier.submeshIndex : 0 },
		{ "material", material_ },
		{ "bEnabledOwn", bEnabledOwn },
		{ "bEnabledOther", bEnabledOther }
	};
}

void ft_render::SkinnedMeshRenderer::deserialize(const nlohmann::json& json) {
	FResourceManager& resourceManager = FResourceManager::getInstance();
	const framework::FMeshIdentifier meshId = framework::FMeshIdentifier(json.at("#meshPath").get<string>(), json.at("meshIndex").get<uint32>());
	reloadMesh(meshId);
	replaceMaterial(json.at("material").get<FMaterial>());
	bEnabledOwn = json.at("bEnabledOwn").get<bool>();
	bEnabledOther = json.at("bEnabledOther").get<bool>();

	size_t animCount = mesh_->nodeToAnimMapVectorByAnimationIndex.size();
	blendingDurationMatrix = std::vector<std::vector<float>>(animCount);
	blendingNeutralPoseMatrix = std::vector<std::vector<int>>(animCount);
	for (size_t it = 0; it < animCount; ++it) {
		blendingDurationMatrix.emplace_back(animCount);
		blendingNeutralPoseMatrix.emplace_back(animCount);
		for (size_t jt = 0; jt < animCount; ++jt) {
			blendingDurationMatrix[it].push_back(0.2f); //default blending duration
			blendingNeutralPoseMatrix[it].push_back(-1); //default neutral animation index
		}
	}
}

void ft_render::SkinnedMeshRenderer::render() {
	framework::FRendererD3D11& rendererInstance = framework::FRendererD3D11::getInstance();
	ID3D11DeviceContext* deviceContext_ = rendererInstance.getD3D11DeviceContext();

	if (mesh_ == nullptr) return;

	// set vertex & index buffers
	ID3D11Buffer* vertexBuffers[3] = { mesh_->vertexBufferStream1_, mesh_->vertexBufferStream2_, mesh_->vertexBufferSkinning_ };
	UINT strides[3] = {
		sizeof(framework::SVertexBufferStream1),
		sizeof(framework::SVertexBufferStream2),
		sizeof(framework::SVertexBufferSkinning)
	};
	UINT offsets[3] = { 0, 0, 0 };
	deviceContext_->IASetVertexBuffers(0, 3, vertexBuffers, strides, offsets);

	// Set material
	ID3D11ShaderResourceView* pTexturesSRV[] = {
		material_.diffuse ? material_.diffuse->getSRV() : nullptr,
		material_.normal ? material_.normal->getSRV() : nullptr,
		material_.roughness ? material_.roughness->getSRV() : nullptr,
		material_.metallic ? material_.metallic->getSRV() : nullptr
	};
	deviceContext_->PSSetShaderResources(0, 4, pTexturesSRV);

	deviceContext_->IASetIndexBuffer(mesh_->indexBuffer_, DXGI_FORMAT_R32_UINT, 0);
	deviceContext_->DrawIndexed(mesh_->numIndices_, 0, 0);
}

void ft_render::SkinnedMeshRenderer::renderNoSkinning() {
	framework::FRendererD3D11& rendererInstance = framework::FRendererD3D11::getInstance();
	ID3D11DeviceContext* deviceContext_ = rendererInstance.getD3D11DeviceContext();

	if (mesh_ == nullptr) return;

	// set vertex & index buffers
	ID3D11Buffer* vertexBuffers[2] = { mesh_->vertexBufferStream1_, mesh_->vertexBufferStream2_};
	UINT strides[2] = {
		sizeof(framework::SVertexBufferStream1),
		sizeof(framework::SVertexBufferStream2)
	};
	UINT offsets[2] = { 0, 0 };
	deviceContext_->IASetVertexBuffers(0, 2, vertexBuffers, strides, offsets);

	// Set material
	ID3D11ShaderResourceView* pTexturesSRV[] = {
		material_.diffuse ? material_.diffuse->getSRV() : nullptr,
		material_.normal ? material_.normal->getSRV() : nullptr,
		material_.roughness ? material_.roughness->getSRV() : nullptr,
		material_.metallic ? material_.metallic->getSRV() : nullptr
	};
	deviceContext_->PSSetShaderResources(0, 4, pTexturesSRV);

	deviceContext_->IASetIndexBuffer(mesh_->indexBuffer_, DXGI_FORMAT_R32_UINT, 0);
	deviceContext_->DrawIndexed(mesh_->numIndices_, 0, 0);
}

void ft_render::SkinnedMeshRenderer::boneTransform(std::vector<Matrix>& vMatrices) {
	if (!bAnimationEnabled) {
		return;
	}

	if (previousAnimationIndex == currentAnimationIndex) {
		const Matrix matIdentity;
		aiScene* pScene = const_cast<aiScene*>(mesh_->assimpImporter->pScene);

		const float ticksPerSecond = static_cast<float>(
			pScene->mAnimations[currentAnimationIndex]->mTicksPerSecond != 0
			? pScene->mAnimations[currentAnimationIndex]->mTicksPerSecond
			: 25.0f);
		const float timeInTicks = animationSliceTime_ * ticksPerSecond * currentAnimationSpeed_;
		const float animationTime = fmod(timeInTicks, static_cast<float>(pScene->mAnimations[currentAnimationIndex]->mDuration));
		readNodeHierarchy(animationTime, pScene->mRootNode, matIdentity);
	}

	vMatrices.resize(mesh_->numBones_);
	for (uint32 i = 0; i < mesh_->numBones_; i++) {
		vMatrices[i] = this->thisMeshBoneInfo_[i].finalTransform_;
	}
}

void ft_render::SkinnedMeshRenderer::readNodeHierarchy(float animationTime, const aiNode* pNode, const Matrix& parentTransform) {
	const string nodeName(pNode->mName.data);

	Matrix xfrom = getMatrix44FromAssimp(pNode->mTransformation);
	aiNodeAnim* pNodeAnim = nullptr;
	const auto boneAnimIterator = mesh_->nodeToAnimMapVectorByAnimationIndex[currentAnimationIndex].find(pNode);
	if (boneAnimIterator != mesh_->nodeToAnimMapVectorByAnimationIndex[currentAnimationIndex].end())
		pNodeAnim = boneAnimIterator->second;

	if (pNodeAnim) {
		aiQuaternion rotationQ;
		calcInterpolatedRotation(rotationQ, animationTime, pNodeAnim);
		const auto rotationM = rotationQ.GetMatrix();

		const Matrix matRot = getMatrix44FromAssimp(rotationM);

		// interpolate translation
		aiVector3D vTranslation;
		calcInterpolatedPosition(vTranslation, animationTime, pNodeAnim);

		const Matrix matTranslation = Matrix::CreateTranslation(Vector3(vTranslation.x, vTranslation.y, vTranslation.z));
		xfrom = matRot * matTranslation;
	}

	Matrix globalXfrom = xfrom * parentTransform;
	if (mesh_->boneMapping_.find(nodeName) != mesh_->boneMapping_.end()) {
		const uint32 boneIndex = mesh_->boneMapping_[nodeName];
		globalXfrom = additionalBoneOffsets[boneIndex] * globalXfrom;
		this->thisMeshBoneInfo_[boneIndex].finalTransform_ = this->thisMeshBoneInfo_[boneIndex].boneOffset_ * globalXfrom;
	}

	for (uint32 i = 0; i < pNode->mNumChildren; i++) {
		readNodeHierarchy(animationTime, pNode->mChildren[i], globalXfrom);
	}
}

void ft_render::SkinnedMeshRenderer::readNodeHierarchy(float animationTime, const aiNode* pNode
	, const Matrix& parentTransform, uint32 animationIndex, std::vector<Matrix>& finalTransformMatrices) {
	const string nodeName(pNode->mName.data);

	Matrix xfrom = getMatrix44FromAssimp(pNode->mTransformation);
	aiNodeAnim* pNodeAnim = nullptr;
	const auto boneAnimIterator = mesh_->nodeToAnimMapVectorByAnimationIndex[animationIndex].find(pNode);
	if (boneAnimIterator != mesh_->nodeToAnimMapVectorByAnimationIndex[animationIndex].end())
		pNodeAnim = boneAnimIterator->second;

	if (pNodeAnim) {
		aiQuaternion rotationQ;
		calcInterpolatedRotation(rotationQ, animationTime, pNodeAnim);
		const auto rotationM = rotationQ.GetMatrix();

		const Matrix matRot = getMatrix44FromAssimp(rotationM);

		// interpolate translation
		aiVector3D vTranslation;
		calcInterpolatedPosition(vTranslation, animationTime, pNodeAnim);

		const Matrix matTranslation = Matrix::CreateTranslation(Vector3(vTranslation.x, vTranslation.y, vTranslation.z));
		xfrom = matRot * matTranslation;
	}

	Matrix globalXfrom = xfrom * parentTransform;
	if (mesh_->boneMapping_.find(nodeName) != mesh_->boneMapping_.end()) {
		const uint32 boneIndex = mesh_->boneMapping_[nodeName];
		globalXfrom = additionalBoneOffsets[boneIndex] * globalXfrom;
		finalTransformMatrices[boneIndex] = this->thisMeshBoneInfo_[boneIndex].boneOffset_ * globalXfrom;
	}

	for (uint32 i = 0; i < pNode->mNumChildren; i++) {
		readNodeHierarchy(animationTime, pNode->mChildren[i], globalXfrom, animationIndex, finalTransformMatrices);
	}
}

uint32 ft_render::SkinnedMeshRenderer::findRotation(float animationTime, const aiNodeAnim* pNodeAnim) {
	for (uint32 i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
		if (animationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
			return i;
		}
	}

	return 0;
}

uint32 ft_render::SkinnedMeshRenderer::findScaling(float animationTime, const aiNodeAnim* pNodeAnim) {
	for (uint32 i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
		if (animationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
			return i;
		}
	}

	return 0;
}

uint32 ft_render::SkinnedMeshRenderer::findPosition(float animationTime, const aiNodeAnim* pNodeAnim) {
	for (uint32 i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
		if (animationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
			return i;
		}
	}

	return 0;
}

void ft_render::SkinnedMeshRenderer::calcInterpolatedRotation(aiQuaternion& out, float animationTime, const aiNodeAnim* pNodeAnim) {
	// we need at least two values to interpolate...
	if (pNodeAnim->mNumRotationKeys == 1) {
		out = pNodeAnim->mRotationKeys[0].mValue;
		return;
	}

	const uint32 rotationIndex = findRotation(animationTime, pNodeAnim);
	const uint32 nextRotationIndex = rotationIndex + 1;

	// assert (nextRotationIndex < pNodeanim->mNumrotationkeys)...

	const float deltaTime = static_cast<float>(pNodeAnim->mRotationKeys[nextRotationIndex].mTime - pNodeAnim->mRotationKeys[rotationIndex].mTime);
	const float factor = (animationTime - static_cast<float>(pNodeAnim->mRotationKeys[rotationIndex].mTime)) / deltaTime;

	const aiQuaternion& startRot = pNodeAnim->mRotationKeys[rotationIndex].mValue;
	const aiQuaternion& endRot = pNodeAnim->mRotationKeys[nextRotationIndex].mValue;

	aiQuaternion::Interpolate(out, startRot, endRot, factor);
	out = out.Normalize();

}

void ft_render::SkinnedMeshRenderer::calcInterpolatedScaling(aiVector3D& out, float animationTime, const aiNodeAnim* pNodeAnim) {

}

void ft_render::SkinnedMeshRenderer::calcInterpolatedPosition(aiVector3D& out, float animationTime, const aiNodeAnim* pNodeAnim) {
	if (pNodeAnim->mNumPositionKeys == 1) {
		out = pNodeAnim->mPositionKeys[0].mValue;
		return;
	}

	const uint32 posIndex = findPosition(animationTime, pNodeAnim);
	const uint32 nextPosIndex = (posIndex + 1);

	const float deltaTime = static_cast<float>(pNodeAnim->mPositionKeys[nextPosIndex].mTime - pNodeAnim->mPositionKeys[posIndex].mTime);
	const float factor = (animationTime - static_cast<float>(pNodeAnim->mPositionKeys[posIndex].mTime)) / deltaTime;

	const aiVector3D& start = pNodeAnim->mPositionKeys[posIndex].mValue;
	const aiVector3D& end = pNodeAnim->mPositionKeys[nextPosIndex].mValue;
	const aiVector3D delta = end - start;
	out = start + factor * delta;
}

IEnumerator ft_render::SkinnedMeshRenderer::blendAnimations(CoroutineArg arg) {
	const Matrix matIdentity;
	aiScene* pScene = const_cast<aiScene*>(mesh_->assimpImporter->pScene);
	const int neutralAnimationIndex = blendingNeutralPoseMatrix[previousAnimationIndex][currentAnimationIndex];
	const float blendingDuration = ((neutralAnimationIndex != -1) ? 0.5f : 1.0f) * blendingDurationMatrix[previousAnimationIndex][currentAnimationIndex];

	/* Old animation part */
	std::vector<Matrix> previousMatrices = std::vector<Matrix>(thisMeshBoneInfo_.size());
	for (size_t it = 0; it < thisMeshBoneInfo_.size(); ++it) {
		previousMatrices[it] = thisMeshBoneInfo_[it].finalTransform_;
	}

	/* Neutral animation part */
	std::vector<Matrix> neutralMatrices = std::vector<Matrix>(thisMeshBoneInfo_.size());
	if (neutralAnimationIndex != -1) {
		//inverse function
		float(*crossFuncInverse)(float) = nullptr;
		switch (crossFadeType) {
		case ECrossFadeType::LINEAR:
			crossFuncInverse = [](float x) { return x; };
			break;
		case ECrossFadeType::SMOOTH_START_2:
			crossFuncInverse = [](float x) { return 1.0f - (1.0f - x) * (1.0f - x); };
			break;
		case ECrossFadeType::SMOOTH_START_3:
			crossFuncInverse = [](float x) { return 1.0f - (1.0f - x) * (1.0f - x) * (1.0f - x); };
			break;
		case ECrossFadeType::SMOOTH_START_4:
			crossFuncInverse = [](float x) { return 1.0f - (1.0f - x) * (1.0f - x) * (1.0f - x) * (1.0f - x); };
			break;
		case ECrossFadeType::SMOOTH_STOP_2:
			crossFuncInverse = [](float x) { return x * x; };
			break;
		case ECrossFadeType::SMOOTH_STOP_3:
			crossFuncInverse = [](float x) { return x * x * x; };
			break;
		case ECrossFadeType::SMOOTH_STOP_4:
			crossFuncInverse = [](float x) { return x * x * x * x; };
			break;
		}

		readNodeHierarchy(0.0f, pScene->mRootNode, matIdentity, neutralAnimationIndex, neutralMatrices);
		while (true) {
			const bool bEnd = animationSliceTime_ >= blendingDuration;
			float t = animationSliceTime_ / blendingDuration;
			t = crossFuncInverse(t);
			if (t > 1.0f) t = 1.0f; //clamp
			for (size_t it = 0; it < thisMeshBoneInfo_.size(); ++it) {
				Vector3 oldPos, newPos, oldScale, newScale;
				Quaternion oldRot, newRot;
				previousMatrices[it].Decompose(oldScale, oldRot, oldPos);
				neutralMatrices[it].Decompose(newScale, newRot, newPos);

				const Vector3 lerpPos = Vector3::SmoothStep(oldPos, newPos, t);
				const Quaternion lerpRot = Quaternion::Slerp(oldRot, newRot, t);
				const Vector3 lerpScale = Vector3::Lerp(oldScale, newScale, t);
				thisMeshBoneInfo_[it].finalTransform_ = Matrix::Compose(lerpPos, lerpRot, lerpScale);
			}

			YIELD_RETURN_NULL();
			if (bEnd) break;
		}
	}

	float(*crossFunc)(float) = nullptr;
	switch (crossFadeType) {
	case ECrossFadeType::LINEAR:
		crossFunc = [](float x) { return x; };
		break;
	case ECrossFadeType::SMOOTH_STOP_2:
		crossFunc = [](float x) { return 1.0f - (1.0f - x) * (1.0f - x); };
		break;
	case ECrossFadeType::SMOOTH_STOP_3:
		crossFunc = [](float x) { return 1.0f - (1.0f - x) * (1.0f - x) * (1.0f - x); };
		break;
	case ECrossFadeType::SMOOTH_STOP_4:
		crossFunc = [](float x) { return 1.0f - (1.0f - x) * (1.0f - x) * (1.0f - x) * (1.0f - x); };
		break;
	case ECrossFadeType::SMOOTH_START_2:
		crossFunc = [](float x) { return x * x; };
		break;
	case ECrossFadeType::SMOOTH_START_3:
		crossFunc = [](float x) { return x * x * x; };
		break;
	case ECrossFadeType::SMOOTH_START_4:
		crossFunc = [](float x) { return x * x * x * x; };
		break;
	}

	/* New animation part */
	std::vector<Matrix> newMatrices = std::vector<Matrix>(thisMeshBoneInfo_.size());
	readNodeHierarchy(0.0f, pScene->mRootNode, matIdentity, currentAnimationIndex, newMatrices);

	animationSliceTime_ = 0.0f;
	/* Blending */
	while (true) {
		const bool bEnd = animationSliceTime_ >= blendingDuration;
		float t = animationSliceTime_ / blendingDuration;
		t = crossFunc(t);
		if (t > 1.0f) t = 1.0f; //clamp
		for (size_t it = 0; it < thisMeshBoneInfo_.size(); ++it) {
			Vector3 oldPos, newPos, oldScale, newScale;
			Quaternion oldRot, newRot;

			if (neutralAnimationIndex != -1) {
				neutralMatrices[it].Decompose(oldScale, oldRot, oldPos);
			} else {
				previousMatrices[it].Decompose(oldScale, oldRot, oldPos);
			}		
			newMatrices[it].Decompose(newScale, newRot, newPos);

			const Vector3 lerpPos = Vector3::SmoothStep(oldPos, newPos, t);
			Quaternion lerpRot = Quaternion::Slerp(oldRot, newRot, t);
			lerpRot.Normalize();
			const Vector3 lerpScale = Vector3::Lerp(oldScale, newScale, t);
			thisMeshBoneInfo_[it].finalTransform_ = Matrix::Compose(lerpPos, lerpRot, lerpScale);
		}

		YIELD_RETURN_NULL();
		if (bEnd) break;
	}

	previousAnimationIndex = currentAnimationIndex;
	animationSliceTime_ = 0.0f;
}
