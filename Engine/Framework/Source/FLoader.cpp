#include "FLoader.h"
#include "FRendererD3D11.h"
#include "Assertion.h"
#include "Logger.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using DirectX::SimpleMath::Matrix;
extern Matrix getMatrix44FromAssimp(const aiMatrix4x4& AssimpMatrix);

bool framework::FLoader::loadMesh(const std::string& meshPath, std::unordered_map<FMeshIdentifier, FMesh*>& meshMap, MeshTypeLoadInfo& meshInfo) {
	FMeshIdentifier parentMeshIdentifier(meshPath, 0);
	FMesh* parentMesh = new FMesh();
	parentMesh->identifier = parentMeshIdentifier;
	bool bLoadSuccess = true;

	FAssimpImporter* imp = new FAssimpImporter();
	imp->pScene = imp->importer.ReadFile(meshPath, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded);

	const aiScene* pScene = imp->pScene;

	uint32 childrenCount = 0;
	aiNode* rootNode = nullptr;

	if (!pScene || !pScene->mRootNode) {
		bLoadSuccess = false;
	} else {
		// check root - does it have any meshes/children?	
		rootNode = pScene->mRootNode;

		const uint32 rootMeshesCount = rootNode->mNumMeshes;
		childrenCount = rootNode->mNumChildren;
		meshInfo.childrenCount = childrenCount; //out param
	}

	if (ASSERT_FAIL(bLoadSuccess, format("Parent mesh couldn't be loaded; Path = ", meshPath))) {
		delete parentMesh;
		return false;
	}

	meshMap.insert({ parentMeshIdentifier, parentMesh });
	meshInfo.skinnedVector.push_back(MeshSkinnedType::NONE);
	meshInfo.boundingBoxes.emplace_back();
	//get name of mesh file
	size_t foundLastSlash = meshPath.rfind('/');
	if (foundLastSlash == std::string::npos)
		foundLastSlash = meshPath.rfind('\\');
	ASSERT_CRITICAL(foundLastSlash != std::string::npos, "Invalid file name");
	std::string fileName(meshPath.substr(foundLastSlash + 1));
	const size_t foundLastDot = fileName.rfind('.');
	ASSERT_CRITICAL(foundLastDot != std::string::npos, "File name does not have dot character");
	meshInfo.names.push_back(fileName.substr(0, foundLastDot));

	for (uint32 i = 0; i < childrenCount; ++i) {
		FMeshIdentifier childMeshIdentifier(meshPath, i + 1);
		FMesh* childMesh = new FMesh();
		childMesh->identifier = childMeshIdentifier;

		aiNode* childNode = rootNode->mChildren[i];
		std::string nodeName = childNode->mName.data;
		meshInfo.names.emplace_back(nodeName);
		meshInfo.skinnedVector.emplace_back(MeshSkinnedType::NONE);
		meshInfo.boundingBoxes.emplace_back();

		const uint32 numMeshes = childNode->mNumMeshes;
		if (numMeshes > 0) {
			for (uint32 numAnimations = 0; numAnimations < pScene->mNumAnimations; ++numAnimations)
				childMesh->nodeToAnimMapVectorByAnimationIndex.emplace_back();
			fillAnimMap(rootNode, pScene, childMesh->nodeToAnimMapVectorByAnimationIndex);
		}

		for (uint32 iMesh = 0; iMesh < numMeshes; iMesh++) {
			const uint32 meshID = childNode->mMeshes[iMesh];
			aiMesh* mesh = pScene->mMeshes[meshID];

			// assign aiScene to user data
			childMesh->assimpImporter = imp;

			// calc indices
			for (uint32 iFace = 0; iFace < mesh->mNumFaces; iFace++) {
				const aiFace& face = mesh->mFaces[iFace];
				childMesh->numIndices_ += face.mNumIndices;
			}

			childMesh->numVertices_ = mesh->mNumVertices;
			childMesh->numFaces_ = mesh->mNumFaces;

			std::vector<SVertexBufferStream1> vBuffer1;
			std::vector<SVertexBufferStream2> vBuffer2;
			std::vector<SVertexBufferSkinning> vBufferSkinning;
			Vector3 aabbMin, aabbMax;

			// load vertices
			for (uint32 iVertex = 0; iVertex < childMesh->numVertices_; iVertex++) {
				SVertexBufferStream1 s1;
				SVertexBufferStream2 s2;

				if (mesh->HasPositions()) {
					memcpy(&s1.position_, &mesh->mVertices[iVertex], sizeof(Vector3));

					// Set the first vertex for initializing min/max vertex.
					if (iVertex == 0) {
						memcpy(&aabbMin, &mesh->mVertices[iVertex], sizeof(Vector3));
						memcpy(&aabbMax, &mesh->mVertices[iVertex], sizeof(Vector3));
					}
				}

				if (mesh->HasNormals()) {
					memcpy(&s2.normal_, &mesh->mNormals[iVertex], sizeof(Vector3));
				}

				if (mesh->HasTangentsAndBitangents()) {
					memcpy(&s2.tangent_, &mesh->mTangents[iVertex], sizeof(Vector3));
					memcpy(&s2.bitangent_, &mesh->mBitangents[iVertex], sizeof(Vector3));
				}

				// texcoords
				const bool hasTexcoords = mesh->HasTextureCoords(0);
				if (hasTexcoords) {
					Vector2 uv(mesh->mTextureCoords[0][iVertex].x, mesh->mTextureCoords[0][iVertex].y);
					memcpy(&s1.texcoords_, &uv, sizeof(Vector2));
				}

				// bounding box (min/max)
				if (s1.position_.x < aabbMin.x)
					aabbMin.x = s1.position_.x;
				if (s1.position_.y < aabbMin.y)
					aabbMin.y = s1.position_.y;
				if (s1.position_.z < aabbMin.z)
					aabbMin.z = s1.position_.z;

				if (s1.position_.x > aabbMax.x)
					aabbMax.x = s1.position_.x;
				if (s1.position_.y > aabbMax.y)
					aabbMax.y = s1.position_.y;
				if (s1.position_.z > aabbMax.z)
					aabbMax.z = s1.position_.z;

				vBuffer1.push_back(s1);
				vBuffer2.push_back(s2);
			}

			// load bones (if any)
			const bool hasBones = mesh->HasBones();
			meshInfo.skinnedVector[i + 1] = (hasBones) ? MeshSkinnedType::SKINNED : MeshSkinnedType::STATIC;

			DirectX::BoundingBox boundingBox;
			boundingBox.Center = (aabbMax + aabbMin) * 0.5f;
			boundingBox.Extents = aabbMax - boundingBox.Center;
			meshInfo.boundingBoxes[i + 1] = boundingBox;

			if (hasBones) {
				vBufferSkinning.resize(mesh->mNumVertices);

				const uint32 numBones = mesh->mNumBones;
				for (uint32 iBoneIndex = 0; iBoneIndex < numBones; iBoneIndex++) {
					uint32 currBoneIndex = 0;
					string boneName(mesh->mBones[iBoneIndex]->mName.data);
					//CLOG(iBoneIndex, boneName, "\n");

					if (childMesh->boneMapping_.find(boneName) == childMesh->boneMapping_.end()) {
						// allocate an index for a new bone
						currBoneIndex = childMesh->numBones_;
						childMesh->numBones_++;

						FBoneInfo bi;
						childMesh->boneInfo_.push_back(bi);
						childMesh->boneInfo_[currBoneIndex].boneOffset_ = getMatrix44FromAssimp(mesh->mBones[iBoneIndex]->mOffsetMatrix);
						childMesh->boneMapping_[boneName] = currBoneIndex;
					} else {
						currBoneIndex = childMesh->boneMapping_[boneName];
					}

					for (uint32 iWeights = 0; iWeights < mesh->mBones[iBoneIndex]->mNumWeights; iWeights++) {
						// todo: for now we assume here 1 submesh in mesh
						const uint32 vertexID = mesh->mBones[iBoneIndex]->mWeights[iWeights].mVertexId;
						const float vertexWeight = mesh->mBones[iBoneIndex]->mWeights[iWeights].mWeight;

						vBufferSkinning[vertexID].addBoneData(currBoneIndex, vertexWeight);
					}
				}
			}

			childMesh->min_ = aabbMin;
			childMesh->max_ = aabbMax;

			// Indices
			std::vector<uint32> vIndices;
			for (uint32 iFace = 0; iFace < childMesh->numFaces_; iFace++) {
				const aiFace& f = mesh->mFaces[iFace];
				for (uint32 k = 0; k < f.mNumIndices; k++) {
					vIndices.push_back((uint32)f.mIndices[k]);
				}
			}

			/* D3D11 stuff */
			SAFE_RELEASE(childMesh->vertexBufferStream1_);
			SAFE_RELEASE(childMesh->vertexBufferStream2_);
			SAFE_RELEASE(childMesh->vertexBufferSkinning_);
			SAFE_RELEASE(childMesh->indexBuffer_);

			framework::FRendererD3D11& renderer = framework::FRendererD3D11::getInstance();
			ID3D11Device* deviceD3D11 = renderer.getD3D11Device();

			D3D11_BUFFER_DESC bufDesc;
			bufDesc.CPUAccessFlags = 0;
			bufDesc.MiscFlags = 0;
			bufDesc.StructureByteStride = 0;
			bufDesc.Usage = D3D11_USAGE_IMMUTABLE;
			D3D11_SUBRESOURCE_DATA subrData;

			// Create vertex buffer (stream 1)
			bufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bufDesc.ByteWidth = childMesh->numVertices_ * sizeof(SVertexBufferStream1);
			subrData.pSysMem = (const void*)vBuffer1.data();

			HRESULT hr = S_OK;
			hr = deviceD3D11->CreateBuffer(&bufDesc, &subrData, &childMesh->vertexBufferStream1_);
			ASSERT_FAIL(hr == S_OK, "Failed to create vertexbuffer1 for mesh");

			// Create vertex buffer (stream 2)
			bufDesc.ByteWidth = childMesh->numVertices_ * sizeof(SVertexBufferStream2);
			subrData.pSysMem = (const void*)vBuffer2.data();

			hr = deviceD3D11->CreateBuffer(&bufDesc, &subrData, &childMesh->vertexBufferStream2_);
			ASSERT_FAIL(hr == S_OK, "Failed to create vertexbuffer2 for mesh");

			// Create skinning buffer (stream 3)
			if (!vBufferSkinning.empty()) {
				bufDesc.ByteWidth = childMesh->numVertices_ * sizeof(SVertexBufferSkinning);
				subrData.pSysMem = (const void*)vBufferSkinning.data();

				hr = deviceD3D11->CreateBuffer(&bufDesc, &subrData, &childMesh->vertexBufferSkinning_);
			}

			// Create index buffer
			bufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bufDesc.ByteWidth = childMesh->numIndices_ * sizeof(uint32);
			subrData.pSysMem = (const void*)vIndices.data();

			hr = deviceD3D11->CreateBuffer(&bufDesc, &subrData, &childMesh->indexBuffer_);
			ASSERT_FAIL(hr == S_OK, "Failed to create index buffer for mesh");

			bLoadSuccess = true;
		}

		//TODO: Load child submeshes
		if (ASSERT_FAIL(bLoadSuccess, format("Child mesh couldn't be loaded; Path = ", meshPath, "; Submesh: ", i))) {
			delete childMesh;
			return false;
		}
		meshMap.insert({ childMeshIdentifier, childMesh });
	}

	return true;
}

bool framework::FLoader::loadTexture(const std::string& texturePath, std::unordered_map<std::string, FExternalTexture*>& textureMap) {
	FRendererD3D11& renderer = FRendererD3D11::getInstance();
	ID3D11Device* d3d11Device = renderer.getD3D11Device();
	ID3D11DeviceContext* d3d11DevContext = renderer.getD3D11DeviceContext();

	if (nullptr == d3d11Device || nullptr == d3d11DevContext) {
		return false;
	}

	FExternalTexture* texture = new FExternalTexture();

	// When texture loading fails, we are using NULL texture.
	// When sampling NULL texture the result is black color.
	const bool textureLoaded = texture->load(d3d11Device, d3d11DevContext, texturePath);
	textureMap.insert({ texturePath, texture });

	return textureLoaded;
}

void framework::FLoader::fillAnimMap(aiNode* node, const aiScene* scene, std::vector<std::unordered_map<const aiNode*, aiNodeAnim*>>& vectorMap) {
	const std::string nodeName = node->mName.data;
	for (uint32 animIter = 0; animIter < scene->mNumAnimations; ++animIter) {
		vectorMap[animIter].insert({ node, nullptr });
		const aiAnimation* pAnimation = scene->mAnimations[animIter];
		for (uint32 channelIter = 0; channelIter < pAnimation->mNumChannels; ++channelIter) {
			aiNodeAnim* pNodeAnim = pAnimation->mChannels[channelIter];
			if (string(pNodeAnim->mNodeName.data) == nodeName) {
				vectorMap[animIter].at(node) = pNodeAnim;
				break;
			}
		}
	}

	for (uint32 childrenIterator = 0; childrenIterator < node->mNumChildren; ++childrenIterator) {
		fillAnimMap(node->mChildren[childrenIterator], scene, vectorMap);
	}
}
