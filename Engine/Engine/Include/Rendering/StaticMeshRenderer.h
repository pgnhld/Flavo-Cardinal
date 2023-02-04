#pragma once

#include "Global.h"
#include "EECS.h"
#include <d3d11.h>
#include "FMesh.h"
#include "FMaterial.h"

FLAVO_COMPONENT(ft_render, StaticMeshRenderer)
namespace ft_render
{
	using namespace framework;
	class StaticMeshRenderer : public eecs::Component<StaticMeshRenderer>
	{
	public:
		StaticMeshRenderer();
		~StaticMeshRenderer();

		void reloadMesh(const FMeshIdentifier& newIdentifier);
		void replaceMaterial(const FMaterial& newMaterial);
		FMaterial& getMaterial();
		const FMesh* getMesh() const;
		void setEnabledBoth(bool bEnabled);

		nlohmann::json serialize() override;
		void deserialize(const nlohmann::json& json) override;

		void render();

		bool bEnabledOwn;
		bool bEnabledOther;
		
	private:
		friend class RenderSystem;

		FMesh* mesh_;
		FMaterial material_;
	};
}