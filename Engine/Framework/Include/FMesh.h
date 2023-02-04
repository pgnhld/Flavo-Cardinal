#pragma once

#include "Global.h"
#include "Maths/Maths.h"
#include <map>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include "Json.h"
#include <unordered_map>

// forward decls
struct ID3D11Buffer;

namespace framework
{
	using DirectX::SimpleMath::Vector2;
	using DirectX::SimpleMath::Vector3;

	struct SVertexBufferStream1
	{
		Vector3 position_;
		Vector2 texcoords_;
	};

	struct SVertexBufferStream2
	{
		Vector3 normal_;
		Vector3 tangent_;
		Vector3 bitangent_;
	};

	// max 4 bones per vertex
	// todo: optimize memory usage
	struct SVertexBufferSkinning
	{
		uint32 blendIndices_[4];
		float blendWeights_[4];

		void reset();
		void addBoneData(uint32 boneID, float weight);
	};

	struct FBoneInfo
	{
		Matrix boneOffset_;
		Matrix finalTransform_;
	};

	struct FMeshIdentifier
	{
		FMeshIdentifier(std::string filePath, uint32 submeshIndex);
		bool operator==(const FMeshIdentifier& another) const;

		std::string filePath;
		uint32 submeshIndex; //"parent" mesh = 0	"children" submeshes = index + 1
	};

	struct FAssimpImporter
	{
		Assimp::Importer importer;
		const aiScene* pScene;
	};

	struct FMesh
	{
		FMesh();
		FMesh(const FMesh& another); //TODO: memcpy all buffers
		~FMesh();

		FMeshIdentifier identifier;

		ID3D11Buffer* indexBuffer_;
		ID3D11Buffer* vertexBufferStream1_;		// stream 0
		ID3D11Buffer* vertexBufferStream2_;		// stream 1
		ID3D11Buffer* vertexBufferSkinning_;	// stream 2

		Vector3 min_;
		Vector3 max_;

		uint32 numIndices_;
		uint32 numVertices_;
		uint32 numFaces_;

		std::map<string, uint32> boneMapping_;
		uint32 numBones_;
		std::vector<FBoneInfo> boneInfo_;
		std::vector<std::unordered_map<const aiNode*, aiNodeAnim*>> nodeToAnimMapVectorByAnimationIndex;

		FAssimpImporter* assimpImporter;
	};
}

namespace std
{
	template<> struct hash<framework::FMeshIdentifier>
	{
		size_t operator()(const framework::FMeshIdentifier& identifier) const {
			const size_t stringHash = std::hash<string>()(identifier.filePath);
			return  std::hash<uint32>()(identifier.submeshIndex) + 0x9e3779b9 + (stringHash << 6) + (stringHash >> 2);
		}
	};
}