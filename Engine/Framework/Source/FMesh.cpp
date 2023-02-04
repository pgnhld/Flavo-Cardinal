#include "FMesh.h"
#include <d3d11.h>

framework::FMeshIdentifier::FMeshIdentifier(std::string filePath, uint32 submeshIndex)
	: filePath(filePath), submeshIndex(submeshIndex) {
}

bool framework::FMeshIdentifier::operator==(const FMeshIdentifier& another) const {
	return filePath == another.filePath && submeshIndex == another.submeshIndex;
}

framework::FMesh::FMesh()
	: identifier("", 0)
	, vertexBufferStream1_(nullptr)
	, vertexBufferStream2_(nullptr)
	, vertexBufferSkinning_(nullptr)
	, indexBuffer_(nullptr)
	, numIndices_(0)
	, numVertices_(0)
	, numFaces_(0)
	, numBones_(0)
	, assimpImporter(nullptr)
{
}

framework::FMesh::FMesh(const FMesh& another)
	: identifier("", 0) {

}

framework::FMesh::~FMesh() {
	SAFE_RELEASE(indexBuffer_);
	SAFE_RELEASE(vertexBufferStream1_);
	SAFE_RELEASE(vertexBufferStream2_);
	SAFE_RELEASE(vertexBufferSkinning_);

	if (assimpImporter) {
		//delete assimpImporter;
		assimpImporter = nullptr;
	}
}

void framework::SVertexBufferSkinning::reset() {
	memset(this, 0, sizeof(framework::SVertexBufferSkinning));
}

void framework::SVertexBufferSkinning::addBoneData( uint32 boneID, float weight ) {
	for (uint32 i=0; i < 4; i++) {
		if (blendWeights_[i] == 0.0f) {
			blendIndices_[i] = boneID;
			blendWeights_[i] = weight;
			return;
		}
	}
}
