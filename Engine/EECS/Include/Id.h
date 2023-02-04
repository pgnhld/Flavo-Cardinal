#pragma once

#include "Global.h"

namespace eecs
{
	struct Id
	{
	public:
		Id();
		Id(uint64 index, uint32 counter);

		bool operator==(const Id& another) const;
		bool operator==(const uint64& value) const;
		bool operator!=(const Id& another) const;
		bool operator!=(const uint64& value) const;
		bool operator<(const Id& another) const;
		bool operator<(const uint64& value) const;

		uint64 index_;

	private:
		friend class EntityManager;
		uint32 counter_;
	};
}