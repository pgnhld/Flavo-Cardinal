#pragma once

#include "Global.h"
#include <Events/IReceiver.h>
#include <Events/IInvoker.h>
#include <unordered_map>
#include "System.h"
#include "Component.h"
#include "Entity.h"
#include "EntityManager.h"
#include "Json.h"

#define STRINGIFY(x) #x

#define FLAVO_SYSTEM(nmspace, type)
#define FLAVO_COMPONENT(nmspace, type)

using eecs::Entity;
using eecs::EntityManager;
using eecs::ComponentHandle;
