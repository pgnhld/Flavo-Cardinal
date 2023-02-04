#pragma once

#include "Global.h"
#include "Assertion.h"

namespace eecs
{
	const int MAX_EVENTS = 32;

	class EventBase
	{
	protected:
		static uint32 typeCounter;
	};

	template<typename T>
	class Event : public EventBase
	{
	public:
		static uint32 type();
	};

	template <typename T>
	uint32 Event<T>::type() {
		static uint32 type = typeCounter++;
		ASSERT_CRITICAL(type < MAX_EVENTS - 1, "Too much Events declared! Consider changing MAX_EVENTS value");
		return type;
	}
}