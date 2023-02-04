#include "Entity.h"
#include "EntityManager.h"

eecs::Entity::Entity() : id_(), manager_(nullptr) {

}

eecs::Entity::Entity(EntityManager* manager, Id id) : id_(id), manager_(manager) {

}

bool eecs::Entity::isValid() const {
	return (manager_ != nullptr) && (manager_->isValid(id_));
}

eecs::Id eecs::Entity::getId() const {
	return id_;
}

bool eecs::Entity::operator==(const Entity& another) const {
	return (manager_ == another.manager_) && (id_ == another.id_);
}

bool eecs::Entity::operator!=(const Entity& another) const {
	return !(*this == another);
}

bool eecs::Entity::operator<(const Entity& another) const {
	return (manager_ == another.manager_) && (id_ < another.id_);
}

string eecs::Entity::toString() const {
	return std::to_string(id_.index_);
}
