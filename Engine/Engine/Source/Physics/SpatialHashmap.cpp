#include "Physics/SpatialHashmap.h"
#include "Physics/Rigidbody.h"
#include "Physics/CharacterController.h"
#include "Physics/Collider.h"
#include "Assertion.h"
#include "Logger.h"
#include <numeric>

std::vector<ft_engine::GridCoords> Bresenham3D(int8_t x1, int8_t y1, int8_t z1, const int8_t x2, const int8_t y2, const int8_t z2) {
	int8_t i, dx, dy, dz, l, m, n, x_inc, y_inc, z_inc, err_1, err_2, dx2, dy2, dz2;
	int8_t point[3];

	std::vector<ft_engine::GridCoords> result;

	point[0] = x1;
	point[1] = y1;
	point[2] = z1;
	dx = x2 - x1;
	dy = y2 - y1;
	dz = z2 - z1;
	x_inc = (dx < 0) ? -1 : 1;
	l = abs(dx);
	y_inc = (dy < 0) ? -1 : 1;
	m = abs(dy);
	z_inc = (dz < 0) ? -1 : 1;
	n = abs(dz);
	dx2 = l << 1;
	dy2 = m << 1;
	dz2 = n << 1;

	if ((l >= m) && (l >= n)) {
		err_1 = dy2 - l;
		err_2 = dz2 - l;
		for (i = 0; i < l; i++) {
			result.push_back(ft_engine::GridCoords(point[0], point[1], point[2]));
			if (err_1 > 0) {
				point[1] += y_inc;
				err_1 -= dx2;
			}
			if (err_2 > 0) {
				point[2] += z_inc;
				err_2 -= dx2;
			}
			err_1 += dy2;
			err_2 += dz2;
			point[0] += x_inc;
		}
	} else if ((m >= l) && (m >= n)) {
		err_1 = dx2 - m;
		err_2 = dz2 - m;
		for (i = 0; i < m; i++) {
			result.push_back(ft_engine::GridCoords(point[0], point[1], point[2]));
			if (err_1 > 0) {
				point[0] += x_inc;
				err_1 -= dy2;
			}
			if (err_2 > 0) {
				point[2] += z_inc;
				err_2 -= dy2;
			}
			err_1 += dx2;
			err_2 += dz2;
			point[1] += y_inc;
		}
	} else {
		err_1 = dy2 - n;
		err_2 = dx2 - n;
		for (i = 0; i < n; i++) {
			result.push_back(ft_engine::GridCoords(point[0], point[1], point[2]));
			if (err_1 > 0) {
				point[1] += y_inc;
				err_1 -= dz2;
			}
			if (err_2 > 0) {
				point[0] += x_inc;
				err_2 -= dz2;
			}
			err_1 += dy2;
			err_2 += dx2;
			point[2] += z_inc;
		}
	}
	result.push_back(ft_engine::GridCoords(point[0], point[1], point[2]));
	return result;
}

ft_engine::GridCoords::GridCoords() {
}

ft_engine::GridCoords::GridCoords(int8_t x, int8_t y, int8_t z) {
	this->x = x;
	this->y = y;
	this->z = z;
}

ft_engine::GridCoords::GridCoords(const Vector3& coords) {
	x = static_cast<int8_t>(MathsHelper::Clamp(static_cast<int8_t>(coords.x / SpatialHashmap::gridSize), INT8_MIN, INT8_MAX));
	y = static_cast<int8_t>(MathsHelper::Clamp(static_cast<int8_t>(coords.y / SpatialHashmap::gridSize), INT8_MIN, INT8_MAX));
	z = static_cast<int8_t>(MathsHelper::Clamp(static_cast<int8_t>(coords.z / SpatialHashmap::gridSize), INT8_MIN, INT8_MAX));
}

void ft_engine::GridCoords::Initialize(const Vector3& coords) {
	x = static_cast<int8_t>(MathsHelper::Clamp(static_cast<int8_t>(coords.x / SpatialHashmap::gridSize), INT8_MIN, INT8_MAX));
	y = static_cast<int8_t>(MathsHelper::Clamp(static_cast<int8_t>(coords.y / SpatialHashmap::gridSize), INT8_MIN, INT8_MAX));
	z = static_cast<int8_t>(MathsHelper::Clamp(static_cast<int8_t>(coords.z / SpatialHashmap::gridSize), INT8_MIN, INT8_MAX));
}

bool ft_engine::GridCoords::operator<(const GridCoords & rhs) const {
	if (x < rhs.x) { return true; }
	if (x > rhs.x) { return false; }
	if (y < rhs.y) { return true; }
	if (y > rhs.y) { return false; }
	if (z < rhs.z) { return true; }
	return false;
}

bool ft_engine::GridCoords::operator==(const GridCoords & rhs) const {
	return (x == rhs.x) && (y == rhs.y) && (z == rhs.z);
}

std::string ft_engine::GridCoords::toString() const {
	return std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z) + " ";
}

std::vector<ft_engine::GridCoords> ft_engine::GridCoords::createAllGridCells(const GridCoords & minimum, const GridCoords & maximum) {
	if (ASSERT_FAIL(minimum < maximum || minimum == maximum, "Minimum is not less or equal to maximum")) {
		return std::vector<GridCoords>();
	}

	if (minimum.x == INT8_MIN || minimum.x == INT8_MAX ||
		minimum.y == INT8_MIN || minimum.y == INT8_MAX ||
		minimum.z == INT8_MIN || minimum.z == INT8_MAX ||
		maximum.x == INT8_MIN || maximum.x == INT8_MAX ||
		maximum.y == INT8_MIN || maximum.y == INT8_MAX ||
		maximum.z == INT8_MIN || maximum.z == INT8_MAX
		) {
		return std::vector<GridCoords>();
	}

	//CLOG(minimum.toString(), maximum.toString(), '\n');
	std::vector<GridCoords> result;
	result.reserve((1 + maximum.x - minimum.x) * (1 + maximum.y - minimum.y) * (1 + maximum.z - minimum.z));


	for (int8_t x = minimum.x; x <= maximum.x; x++) {
		for (int8_t y = minimum.y; y <= maximum.y; y++) {
			for (int8_t z = minimum.z; z <= maximum.z; z++) {
				GridCoords currCoords(x, y, z);
				result.push_back(currCoords);
			}
		}
	}

	return result;
}


const int ft_engine::SpatialHashmap::gridSize = 3;

bool ft_engine::SpatialHashmap::tryAddEntity(eecs::Entity entity) {
	Transform* transform = entity.getComponent<Transform>().get();
	Collider* collider = entity.getComponent<Collider>().get();

	if (ASSERT_FAIL(transform, "Entity with invalid Transform") || ASSERT_FAIL(collider, "Entity with invalid Collider")) {
		return false;
	}

	if (entity.hasComponent<Rigidbody>() || entity.hasComponent<CharacterController>()) {
		// If dynamic entity was added previously due to having collider move it from static entities to dynamic
		if (staticEntitiesVolumes.find(entity) != staticEntitiesVolumes.end()) {
			const auto bounds = staticEntitiesVolumes[entity];
			staticEntitiesVolumes.erase(entity);
			dynamicEntitiesVolumes[entity] = bounds;
		}
		return tryAddDynamicEntity(entity);
	} else {
		return tryAddStaticEntity(entity);
	}
}

bool ft_engine::SpatialHashmap::tryAddStaticEntity(eecs::Entity entity) {
	if (ASSERT_FAIL(entity.hasComponent<Transform>(), "Entity with invalid Transform")) {
		return false;
	}
	if (ASSERT_FAIL(staticEntitiesVolumes.find(entity) == staticEntitiesVolumes.end(), "Static Entity already exists")) {
		return false;
	}
	addEntity(entity, true);
	return true;
}

bool ft_engine::SpatialHashmap::tryAddDynamicEntity(eecs::Entity entity) {
	if (ASSERT_FAIL(entity.hasComponent<Transform>(), "Entity with invalid Transform")) {
		return false;
	}
	if (ASSERT_FAIL(dynamicEntitiesVolumes.find(entity) == dynamicEntitiesVolumes.end(), "Dynamic Entity already exists")) {
		return false;
	}
	addEntity(entity, false);
	return true;
}

void ft_engine::SpatialHashmap::addEntity(eecs::Entity entity, bool bStatic) {
	Transform* transform = entity.getComponent<Transform>().get();
	BoundingBox boundingVolume = transform->getTransformedBoundingBox();

	//CLOG("BOUNDING VOLUME: ");
	//CLOG(std::to_string(boundingVolume.Center.x), " ", std::to_string(boundingVolume.Center.y), " ", std::to_string(boundingVolume.Center.z));

	GridCoords minimum(static_cast<Vector3>(boundingVolume.Center) - static_cast<Vector3>(boundingVolume.Extents));
	GridCoords maximum(static_cast<Vector3>(boundingVolume.Center) + static_cast<Vector3>(boundingVolume.Extents));

	//CLOG("\nADD ENTITY: ", entity.toString(), " min: ", minimum.toString(), " max: ", maximum.toString(), '\n');

	std::vector<GridCoords> coords = GridCoords::createAllGridCells(minimum, maximum);
	for (const auto& currCoords : coords) {
		if (hashmap.find(currCoords) == hashmap.end()) {
			std::vector<eecs::Entity> dummyVector;
			hashmap[currCoords] = dummyVector;
		}
		hashmap[currCoords].push_back(entity);
	}

	if (bStatic) {
		staticEntitiesVolumes[entity] = std::make_pair(minimum, maximum);
	} else {
		dynamicEntitiesVolumes[entity] = std::make_pair(minimum, maximum);
	}
}

bool ft_engine::SpatialHashmap::tryUpdateDynamicEntity(eecs::Entity entity) {
	if (ASSERT_FAIL(dynamicEntitiesVolumes.find(entity) != dynamicEntitiesVolumes.end(), "Dynamic Entity does not exist")) {
		return false;
	}

	updateDynamicEntity(entity);
	return true;
}

void ft_engine::SpatialHashmap::updateDynamicEntity(eecs::Entity entity) {
	Transform* transform = entity.getComponent<Transform>().get();
	BoundingBox boundingVolume = transform->getTransformedBoundingBox();

	GridCoords minimum(static_cast<Vector3>(boundingVolume.Center) - static_cast<Vector3>(boundingVolume.Extents));
	GridCoords maximum(static_cast<Vector3>(boundingVolume.Center) + static_cast<Vector3>(boundingVolume.Extents));

	//CLOG("======== INB4 =========", '\n');
	//CLOG(std::to_string(boundingVolume.Center.x), " ", std::to_string(boundingVolume.Center.y), " ", std::to_string(boundingVolume.Center.z), '\n');
	//CLOG(std::to_string(boundingVolume.Extents.x), " ", std::to_string(boundingVolume.Extents.y), " ", std::to_string(boundingVolume.Extents.z), '\n');

	std::pair<GridCoords, GridCoords> previousBounds = dynamicEntitiesVolumes[entity];
	if (previousBounds == std::make_pair(minimum, maximum)) {
		return;
	}

	std::vector<GridCoords> previousGridCells = GridCoords::createAllGridCells(previousBounds.first, previousBounds.second);
	std::vector<GridCoords> currentGridCells = GridCoords::createAllGridCells(minimum, maximum);

	// Remove entity from old cells where it does not longer exist
	std::vector<GridCoords> cellsToRemove;
	std::set_difference(previousGridCells.begin(), previousGridCells.end(), currentGridCells.begin(), currentGridCells.end(), std::inserter(cellsToRemove, cellsToRemove.begin()));

	for (const auto& currCoords : cellsToRemove) {
		auto& entities = hashmap[currCoords];
		const auto endOfRangeIterator = std::remove(entities.begin(), entities.end(), entity);
		if (endOfRangeIterator != entities.end()) {
			entities.erase(endOfRangeIterator, entities.end());
		}
	}

	// Add entity tp new cells where it appeared
	std::vector<GridCoords> cellsToAdd;
	std::set_difference(currentGridCells.begin(), currentGridCells.end(), previousGridCells.begin(), previousGridCells.end(), std::inserter(cellsToAdd, cellsToAdd.begin()));

	for (const auto& currCoords : cellsToAdd) {
		if (hashmap.find(currCoords) == hashmap.end()) {
			std::vector<eecs::Entity> dummyVector;
			hashmap[currCoords] = dummyVector;
		}
		hashmap[currCoords].push_back(entity);
	}

	dynamicEntitiesVolumes[entity] = std::make_pair(minimum, maximum);
}

void ft_engine::SpatialHashmap::updateDynamicEntities() {
	for (auto it = dynamicEntitiesVolumes.begin(); it != dynamicEntitiesVolumes.end(); ++it) {
		updateDynamicEntity(it->first);
	}

	//CLOG("HASHMAP\n");
	//for (auto it = hashmap.begin(); it != hashmap.end(); it++) {
	//	CLOG("\nCoords: ", it->first.toString(), '\n');
	//	for (const auto& entity : it->second) {
	//		CLOG(entity.toString(), ' ');
	//	}
	//}

	//CLOG("\nDYNAMIC\n");
	//for (auto it = dynamicEntitiesVolumes.begin(); it != dynamicEntitiesVolumes.end(); it++) {
	//	CLOG(it->first.toString(), " ", it->second.first.toString(), " ", it->second.second.toString(), '\n');
	//}

	//CLOG("\nSTATIC\n");
	//for (auto it = staticEntitiesVolumes.begin(); it != staticEntitiesVolumes.end(); it++) {
	//	CLOG(it->first.toString(), " ", it->second.first.toString(), " ", it->second.second.toString(), '\n');
	//}
}

std::set<eecs::Entity> ft_engine::SpatialHashmap::queryEntities(const BoundingBox& boundingVolume) {
	GridCoords minimum(static_cast<Vector3>(boundingVolume.Center) - static_cast<Vector3>(boundingVolume.Extents));
	GridCoords maximum(static_cast<Vector3>(boundingVolume.Center) + static_cast<Vector3>(boundingVolume.Extents));
	return queryEntities(minimum, maximum);
}

std::set<eecs::Entity> ft_engine::SpatialHashmap::queryEntities(const GridCoords& minimum, const GridCoords& maximum) {
	std::set<eecs::Entity> uniqueEntities;
	std::vector<GridCoords> cellsToCheck = GridCoords::createAllGridCells(minimum, maximum);
	for (const auto& currCoords : cellsToCheck) {
		if (hashmap.find(currCoords) != hashmap.end()) {
			uniqueEntities.insert(hashmap[currCoords].begin(), hashmap[currCoords].end());
		}
	}
	return uniqueEntities;
}

std::set<eecs::Entity> ft_engine::SpatialHashmap::queryEntities(const Raycast& raycast) {
	GridCoords raycastBegin(raycast.origin);
	GridCoords raycastEnd(raycast.origin + raycast.direction * raycast.maxLength);

	std::set<eecs::Entity> potentialColliders;
	std::vector<GridCoords> gridCells = Bresenham3D(raycastBegin.x, raycastBegin.y, raycastBegin.z, raycastEnd.x, raycastEnd.y, raycastEnd.z);

	//CLOG("QUERY RAYCAST\n");
	//CLOG(raycast.origin, " ", raycast.direction, " ", raycast.maxLength, '\n');

	for (int i = 0; i < gridCells.size(); i++) {
		//CLOG(gridCells[i].toString(), '\n');
		const int currCellEntitySize = hashmap[gridCells[i]].size();
		for (int j = 0; j < currCellEntitySize; j++) {
			potentialColliders.insert(hashmap[gridCells[i]][j]);
		}
	}

	return potentialColliders;
}
