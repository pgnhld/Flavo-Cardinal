#include "ByteArray.h"
#include "Memory.h"
#include "Assertion.h"

using namespace utils;

ByteArrayBase::ByteArrayBase(uint32 elementSize, uint32 noElementsInBlock) : elementSize_(elementSize), noElementsInBlock_(noElementsInBlock), size_(0), capacity_(0) {

}

uint32 ByteArrayBase::size() const {
	return size_;
}

void ByteArrayBase::reserve(uint32 newCount) {
	if (newCount <= size_)
		return;

	while (capacity_ < newCount) {
		byte* newBlock = new byte[elementSize_ * noElementsInBlock_];
		blocks_.push_back(newBlock);
		capacity_ += noElementsInBlock_;
	}

	size_ = newCount;
}

void* ByteArrayBase::getPtr(uint32 index) {
	ASSERT_CRITICAL(size_ > index, "Out of ByteArray range!");
	return blocks_[index / noElementsInBlock_] + (index % noElementsInBlock_) * elementSize_;
}

void utils::ByteArrayBase::assign(uint32 index, void* data) {
	ASSERT_CRITICAL(capacity_ > index, "Out of ByteArray range!");
	memcpy((blocks_[index / noElementsInBlock_] + (index % noElementsInBlock_) * elementSize_), data, elementSize_);
}

void* ByteArrayBase::operator[](uint32 index) {
	return getPtr(index);
}