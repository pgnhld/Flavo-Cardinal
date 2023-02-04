#pragma once

#include "Global.h"
#include <unordered_map>
#include "FMesh.h"
#include "FExternalTexture.h"

namespace framework
{
	enum class MeshSkinnedType
	{
		NONE,
		STATIC,
		SKINNED
	};

	struct MeshTypeLoadInfo
	{
		uint32 childrenCount;
		std::vector<MeshSkinnedType> skinnedVector;
		std::vector<DirectX::BoundingBox> boundingBoxes;
		std::vector<std::string> names;
	};

	class FLoader
	{
	public:
		static bool loadMesh(const std::string& meshPath, REF std::unordered_map<FMeshIdentifier, FMesh*>& meshMap, OUT MeshTypeLoadInfo& meshInfo);
		static bool loadTexture(const std::string& texturePath, REF std::unordered_map<std::string, FExternalTexture*>& textureMap);

	private:
		static void fillAnimMap(aiNode* node, const aiScene* scene, REF std::vector<std::unordered_map<const aiNode*, aiNodeAnim*>>& vectorMap);
	};
}
