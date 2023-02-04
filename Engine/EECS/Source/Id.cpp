#include "Id.h"

eecs::Id::Id() : index_(9999999), counter_(0) {

}

eecs::Id::Id(uint64 index, uint32 counter) : index_(index), counter_(counter) {

}

bool eecs::Id::operator==(const Id& another) const {
	return (index_ == another.index_) && (counter_ == another.counter_);
}

bool eecs::Id::operator==(const uint64& value) const {
	return index_ == value;
}

bool eecs::Id::operator!=(const Id& another) const {
	return !(*this == another);
}

bool eecs::Id::operator!=(const uint64& value) const {
	return !(index_ == value);
}

bool eecs::Id::operator<(const Id& another) const {
	return index_ < another.index_;
}

bool eecs::Id::operator<(const uint64& value) const {
	return index_ < value;
}
