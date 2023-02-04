#pragma once

#include <Global.h>
#include "EECS.h"
#include "Transform.h"
#include <set>
#include <DxtkString.h>
#include "Collider.h"

namespace ft_engine
{
	struct GridCoords {
		int8_t x;
		int8_t y;
		int8_t z;

		static std::vector<GridCoords> createAllGridCells(const GridCoords& minimum, const GridCoords& maximum);

		GridCoords();
		GridCoords(int8_t x, int8_t y, int8_t z);
		GridCoords(const Vector3& coords);

		void Initialize(const Vector3& coords);
		bool operator<(const GridCoords& rhs) const;
		bool operator==(const GridCoords& rhs) const;
		std::string toString() const;
	};

	class SpatialHashmap {
	public:
		static const int gridSize;

		/// Adds entity (automatically detected whether it's static or dynamic based on presence of Rigidbody or Character Controller component)
		bool tryAddEntity(eecs::Entity entity);

		/// Adds entity with Collider which cannot move
		bool tryAddStaticEntity(eecs::Entity entity);

		/// Adds entity with Collider which can move (eg. Rigidbody or CharacterController)
		bool tryAddDynamicEntity(eecs::Entity entity);

		/// Update positions of certain dynamic entity
		bool tryUpdateDynamicEntity(eecs::Entity entity);

		/// Update positions of all dynamic entities
		void updateDynamicEntities();

		/// Update positions of certain entity
		void updateDynamicEntity(eecs::Entity entity);

		std::set<eecs::Entity> queryEntities(const BoundingBox& boundingVolume);

		std::set<eecs::Entity> queryEntities(const GridCoords& minimum, const GridCoords& maximum);

		std::set<eecs::Entity> queryEntities(const Raycast& raycast);

		std::map< GridCoords, std::vector<eecs::Entity> > hashmap;
		std::map< eecs::Entity, std::pair<GridCoords, GridCoords> > dynamicEntitiesVolumes;
		std::map< eecs::Entity, std::pair<GridCoords, GridCoords> > staticEntitiesVolumes;

	private:
		void addEntity(eecs::Entity entity, bool bStatic);
	};
}