#include "FResourceManager.h"
#include "FLoader.h"
#include "Assertion.h"

framework::FResourceManager& framework::FResourceManager::getInstance() {
	static FResourceManager rm;
	return rm;
}

bool framework::FResourceManager::loadMesh(const std::string& filePath, MeshTypeLoadInfo& meshInfo) {
	if (meshChildrenCountMap_.find(filePath) != meshChildrenCountMap_.end()) {
		meshInfo = meshChildrenCountMap_[filePath];
		return true;
	}

	const bool bMeshLoaded = FLoader::loadMesh(filePath, meshMap_, meshInfo);
	if (bMeshLoaded)
		meshChildrenCountMap_.insert({ filePath, meshInfo });

	return bMeshLoaded;
}

framework::FMesh* framework::FResourceManager::getMesh(const FMeshIdentifier& identifier, MeshTypeLoadInfo& meshInfo) {
	auto meshMapIt = meshMap_.find(identifier);
	if (meshMapIt != meshMap_.end()) {
		return meshMapIt->second;
	}

	const bool meshLoaded = loadMesh(identifier.filePath, meshInfo);
	ASSERT_CRITICAL(meshLoaded, "Couldn't load mesh");

	meshMapIt = meshMap_.find(identifier);
	ASSERT_CRITICAL(meshMapIt != meshMap_.end(), "Couldn't find loaded mesh in map");
	return meshMapIt->second;
}

framework::FExternalTexture* framework::FResourceManager::getTexture(const std::string& path) {
	auto textureMapIt = textureMap_.find(path);
	if (textureMapIt != textureMap_.end()) {
		return textureMapIt->second;
	}

	const bool textureLoaded = FLoader::loadTexture(path, textureMap_);
	if (!textureLoaded)
		return nullptr;

	// texture successfully loaded from external file.
	textureMapIt = textureMap_.find(path);
	ASSERT_CRITICAL(textureMapIt != textureMap_.end(), "Couldn't find loaded mesh in map");
	return textureMapIt->second;
}

void framework::FResourceManager::releaseResources() {
	releaseAllTextures();
	releaseAllMeshes();
}

void framework::FResourceManager::releaseAllTextures() {
	if (!textureMap_.empty()) {
		for (const auto tex : textureMap_) {
			framework::FExternalTexture* d3dtex = tex.second;

			if (d3dtex) {
				d3dtex->cleanup();

				delete d3dtex;
				d3dtex = nullptr;
			}
		}
		textureMap_.clear();
	}	
}

void framework::FResourceManager::releaseAllMeshes() {
	if (!meshMap_.empty()) {
		for (const auto mesh : meshMap_) {
			framework::FMesh* d3dmesh = mesh.second;

			if (d3dmesh) {
				delete d3dmesh;
				d3dmesh = nullptr;
			}
		}
		meshMap_.clear();
	}

	if (!meshChildrenCountMap_.empty())
		meshChildrenCountMap_.clear();
}
