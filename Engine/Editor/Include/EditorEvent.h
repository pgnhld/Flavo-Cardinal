#pragma once

#include "Global.h"
#include "EECS.h"

struct EventSerializeSystem : eecs::Event<EventSerializeSystem>
{
	std::string filePath;
};