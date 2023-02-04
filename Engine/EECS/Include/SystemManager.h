#pragma once

#include "Global.h"
#include "System.h"
#include "EntityManager.h"
#include "SystemHandle.h"

namespace eecs
{
	class SystemManager
	{
	public:
		SystemManager() {}
		~SystemManager() {
			bDeleted = true;
		}

		void update(EntityManager& entities, double deltaTime);
		void fixedUpdate(EntityManager& entities, double deltaTime);
		
		void add(SystemBase* system, double period = 0.0);

	private:
		std::vector<SystemHandle> systems_;
		bool bDeleted;
	};
}
