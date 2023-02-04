#pragma once

#include "Events/Event.h"

struct EventComponentAdded : eecs::Event<EventComponentAdded>
{
	EventComponentAdded(uint32 componentType, eecs::Entity entity) : componentType(componentType), entity(entity) {}

	uint32 componentType;
	eecs::Entity entity;
};
