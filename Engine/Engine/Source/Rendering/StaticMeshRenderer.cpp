#include "Rendering/StaticMeshRenderer.h"
#include "FRendererD3D11.h"
#include "FResourceManager.h"
#include "FLoader.h"

ft_render::StaticMeshRenderer::StaticMeshRenderer()
	: mesh_(nullptr), material_(FMaterial()), bEnabledOwn(true), bEnabledOther(true) {

}

ft_render::StaticMeshRenderer::~StaticMeshRenderer() {
	// todo: it's never called.
}

void ft_render::StaticMeshRenderer::reloadMesh(const framework::FMeshIdentifier& newIdentifier) {
	FResourceManager& resourceManager = FResourceManager::getInstance();
	MeshTypeLoadInfo meshInfo;
	mesh_ = resourceManager.getMesh(newIdentifier, meshInfo);
}

void ft_render::StaticMeshRenderer::replaceMaterial(const framework::FMaterial& newMaterial) {
	material_ = FMaterial(newMaterial);
}

framework::FMaterial& ft_render::StaticMeshRenderer::getMaterial() {
	return material_;
}

const framework::FMesh* ft_render::StaticMeshRenderer::getMesh() const {
	return mesh_;
}

void ft_render::StaticMeshRenderer::setEnabledBoth(bool bEnabled) {
	bEnabledOther = bEnabled;
	bEnabledOwn = bEnabled;
}

nlohmann::json ft_render::StaticMeshRenderer::serialize() {
	return {
		{ "#meshPath", (mesh_ != nullptr) ? mesh_->identifier.filePath : "-" },
		{ "meshIndex", (mesh_ != nullptr) ? mesh_->identifier.submeshIndex : 0 },
		{ "material", material_ },
		{ "bEnabledOwn", bEnabledOwn },
		{ "bEnabledOther", bEnabledOther }
	};
}

void ft_render::StaticMeshRenderer::deserialize(const nlohmann::json& json) {
	FResourceManager& resourceManager = FResourceManager::getInstance();
	const framework::FMeshIdentifier meshId = framework::FMeshIdentifier(json.at("#meshPath").get<string>(), json.at("meshIndex").get<uint32>());
	reloadMesh(meshId);
	replaceMaterial(json.at("material").get<FMaterial>());
	bEnabledOwn = json.at("bEnabledOwn").get<bool>();
	bEnabledOther = json.at("bEnabledOther").get<bool>();
}

void ft_render::StaticMeshRenderer::render() {
	framework::FRendererD3D11& rendererInstance = framework::FRendererD3D11::getInstance();
	ID3D11DeviceContext* deviceContext_ = rendererInstance.getD3D11DeviceContext();
	if (mesh_ == nullptr) return;

	// set vertex & index buffers
	ID3D11Buffer* vertexBuffers[2] = { mesh_->vertexBufferStream1_, mesh_->vertexBufferStream2_ };
	UINT strides[2] = { sizeof(framework::SVertexBufferStream1), sizeof(framework::SVertexBufferStream2) };
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
