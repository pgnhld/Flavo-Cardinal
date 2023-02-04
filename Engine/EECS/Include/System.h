#pragma once

#include "Global.h"

namespace eecs
{
	class EntityManager;

	class SystemBase
	{
	public:
		virtual ~SystemBase() = default;

	public:
		virtual void update(EntityManager& entities, double deltaTime) = 0;
		virtual void fixedUpdate(EntityManager& entities, double fixedDeltaTime) = 0;

	protected:
		friend class SystemHandle;
		friend class Core; //TODO: remove this
		static uint32 typeCounter;
	};

	/**
	 * \brief Class all Systems should derive from.
	 * \remarks class NewSystem : public eecs::System<NewSystem>
	 * \remarks Available methods to override: update(...), fixedUpdate(...)
	 */
	template<typename T>
	class System : public SystemBase
	{
	public:
		static uint32 type();

		void update(EntityManager& entities, double deltaTime) override {}
		void fixedUpdate(EntityManager& entities, double fixedDeltaTime) override {}
	};

	template<typename T>
	uint32 System<T>::type() {
		static uint32 type = typeCounter++;
		return type;
	}
}