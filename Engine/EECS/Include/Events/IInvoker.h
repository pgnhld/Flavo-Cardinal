#pragma once

#include "Global.h"
#include "EventManager.h"

namespace eecs
{
	/*
	 * Classes which want to invoke events easily should inherit from this
	 */
	class IInvoker
	{
	protected:
		/*
		 * Invoke user-defined event from arguments e.g. 
		 * class SomeEventDerivedClass : public eecs::Event<SomeEventDerivedClass> {
		 *     SomeEventDerivedClass(int x, float y) : x(x), y(y) { }
		 *     int x;
		 *     float y;
		 * }
		 * 
		 * invoke<SomeEventDerivedClass>(12, 4.7f);
		 */
		template<typename E, typename... Args>
		void invoke(Args... args) const;

		/*
		* Invoke user-defined event from existing pointer e.g.
		* class SomeEventDerivedClass : public eecs::Event<SomeEventDerivedClass> {
		*     SomeEventDerivedClass(int x, float y) : x(x), y(y) { }
		*     int x;
		*     float y;
		* }
		* 
		* SomeEventDerivedClass* eventPtr = new SomeEventDerivedClass();
		* eventPtr->x = 5;
		* eventPtr->y = 14.0f;
		* invoke<SomeEventDerivedClass>(eventPtr);
		*/
		template<typename E>
		void invoke(E* ptr) const;

		/*
		 * Invoke user-defined event with argument which can be modified by subscribents
		 * USE WITH CAUTION. Try not to have 2 or more subscribents to one event type to avoid debugging hell
		 */
		template<typename E>
		void invokeNonConst(E* ptr) const;
	};

	template <typename E, typename... Args>
	void IInvoker::invoke(Args... args) const {
		EventManager::getInstance().invoke(new E(std::forward<Args>(args)...));
	}

	template <typename E>
	void IInvoker::invoke(E* ptr) const {
		EventManager::getInstance().invoke(ptr);
	}

	template <typename E>
	void IInvoker::invokeNonConst(E* ptr) const {
		EventManager::getInstance().invokeNonConst(ptr);
	}
}
