#pragma once

#include "Global.h"
#include <unordered_map>
#include <functional>
#include "EventManager.h"
#include "Assertion.h"

namespace eecs
{
	/*
	 * Classes which want to easily subscribe to events should inherit from this
	 */
	template<typename T>
	class IReceiver
	{
	protected:
		/* Subscribe to certain event type e.g.
		 * subscribe<SomeEventDerivedClass>(this, &OurClass::callbackMethod);
		 */
		template<typename E>
		void subscribe(T* object, void(T::*callback)(const E&));

		template<typename E>
		void subscribeNonConst(T* object, void(T::*callback)(E&));

		/* Unsubscribe to certain event type. Must have been subscribed earlier e.g.
		 * unsubscribe<SomeEventDerivedClass>();
		 */
		template<typename E>
		void unsubscribe();

		template<typename E>
		void unsubscribeNonConst();

	private:
		std::unordered_map<uint32, size_t> eventMap_;
		std::unordered_map<uint32, size_t> eventMapNonConst_;
	};

	template <typename T>
	template <typename E>
	void IReceiver<T>::subscribe(T* object, void(T::*callback)(const E&)) {
		const uint32 type = E::type();
		const size_t index = EventManager::getInstance().subscribe<E>(std::bind(callback, object, std::placeholders::_1));

		ASSERT_CRITICAL(eventMap_.find(type) == eventMap_.end(), "Cannot subscribe; You have already subscribed to this event type");
		eventMap_.insert(std::pair<uint32, size_t>(type, index));
	}

	template <typename T>
	template <typename E>
	void IReceiver<T>::subscribeNonConst(T* object, void(T::* callback)(E&)) {
		const uint32 type = E::type();
		const size_t index = EventManager::getInstance().subscribeNonConst<E>(std::bind(callback, object, std::placeholders::_1));

		ASSERT_CRITICAL(eventMapNonConst_.find(type) == eventMapNonConst_.end(), "Cannot subscribe; You have already subscribed to this event type");
		eventMapNonConst_.insert(std::pair<uint32, size_t>(type, index));
	}

	template <typename T>
	template <typename E>
	void IReceiver<T>::unsubscribe() {
		const uint32 type = E::type();
		auto it = eventMap_.find(type);
		ASSERT_CRITICAL(it != eventMap_.end(), "Cannot unsubscribe; You have not sunscribed to this event type");
		EventManager::getInstance().unsubscribe<E>(it->second);
		eventMap_.erase(it);
	}

	template <typename T>
	template <typename E>
	void IReceiver<T>::unsubscribeNonConst() {
		const uint32 type = E::type();
		auto it = eventMapNonConst_.find(type);
		ASSERT_CRITICAL(it != eventMapNonConst_.end(), "Cannot unsubscribe; You have not sunscribed to this event type");
		EventManager::getInstance().unsubscribeNonConst<E>(it->second);
		eventMapNonConst_.erase(it);
	}
}
