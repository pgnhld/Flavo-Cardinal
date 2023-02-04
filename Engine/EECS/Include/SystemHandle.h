#pragma once

#include "Global.h"
#include <memory>
#include "System.h"

namespace eecs
{
	class SystemHandle
	{
	public:
		SystemHandle(std::unique_ptr<SystemBase> system, double period);
		SystemHandle(const SystemHandle& another) = delete;
		SystemHandle(SystemHandle&& another) = default;
		~SystemHandle();

		SystemHandle& operator=(const SystemHandle& another) = delete;
		SystemHandle& operator=(SystemHandle&& another) = default;

	private:
		void update(EntityManager& entities, double deltaTime);
		void fixedUpdate(EntityManager& entities, double fixedDeltaTime);

	private:
		friend class SystemManager;

		std::unique_ptr<SystemBase> system_;
		double period_;
		double timer_;
	};
}
