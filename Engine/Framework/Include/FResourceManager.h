#pragma once

#include "Global.h"
#include <unordered_map>
#include "FMesh.h"
#include "FExternalTexture.h"

namespace framework
{
	struct MeshTypeLoadInfo;

	class FResourceManager
	{
	public:
		static FResourceManager& getInstance();

		/* Used by editor */
		bool loadMesh(const std::string& filePath, OUT MeshTypeLoadInfo& meshInfo);
		FMesh* getMesh(const FMeshIdentifier& identifier, OUT MeshTypeLoadInfo& meshInfo);
		FExternalTexture* getTexture(const std::string& path);
		FExternalTexture* getTextureDigit(int digit);

		void releaseResources();


	private:
		void releaseAllTextures();
		void releaseAllMeshes();

		std::unordered_map<FMeshIdentifier, FMesh*> meshMap_;
		std::unordered_map<std::string, FExternalTexture*> textureMap_;
		std::unordered_map<std::string, MeshTypeLoadInfo> meshChildrenCountMap_;
	};
}

#define IMAGE(path) (framework::FResourceManager::getInstance().getTexture(std::string("../Data/Images/") + path)->getSRV())
#define IMAGE_DIGIT(digit) (framework::FResourceManager::getInstance().getTextureDigit(digit)->getSRV())
