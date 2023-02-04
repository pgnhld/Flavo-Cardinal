#pragma once

#include "Global.h"
#include <vector>
#include "Assertion.h"

namespace utils
{
	class ByteArrayBase
	{
	public:
		ByteArrayBase(uint32 dataSize, uint32 blockSize);
		virtual ~ByteArrayBase() {}

		uint32 size() const;
		void reserve(uint32 newCount);
		void* getPtr(uint32 index);
		virtual void destroy(uint32 index) = 0;

		void assign(uint32 index, void* data);
		void* operator[](uint32 index);

	private:
		uint32 size_;
		uint32 capacity_;
		uint32 noElementsInBlock_;
		uint32 elementSize_;
		std::vector<byte*> blocks_;
	};

	template<typename T, uint64 blockSize = 1024>
	class ByteArray : public ByteArrayBase
	{
	public:
		ByteArray() : ByteArrayBase(sizeof(T), blockSize) { }

		T* get(uint32 index) {
			return static_cast<T*>(getPtr(index));
		}

		void destroy(uint32 index) override {
			ASSERT_CRITICAL(index <= size(), "Index to destroy out of bounds");
			T* pointer = static_cast<T*>(getPtr(index));
			pointer->~T();
		}
	};
}