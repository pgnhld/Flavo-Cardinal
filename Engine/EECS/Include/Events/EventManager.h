#pragma once

#include "Global.h"

#include <utility>
#include <vector>
#include <functional>
#include <memory>

#include "Event.h"
#include "IReceiver.h"
#include "Assertion.h"

namespace eecs
{
	/*
	 * Wrapper responsible for casting event-function type to void*
	 */
	template <typename T>
	struct EventCallbackWrapper
	{
		EventCallbackWrapper(std::function<void(const T&)> callback) : callback(std::move(callback)) {}
		void operator()(const void* event) { callback(*(static_cast<const T*>(event))); }
		std::function<void(const T&)> callback;
	};

	template <typename T>
	struct EventCallbackWrapperNonConst
	{
		EventCallbackWrapperNonConst(std::function<void(T&)> callback) : callback(std::move(callback)) {}
		void operator()(void* event) { callback(*(static_cast<T*>(event))); }
		std::function<void(T&)> callback;
	};

	/* 
	 * Event managing singleton class
	 */
	class EventManager
	{
	public:
		static EventManager& getInstance();

	public:
		EventManager(const EventManager& other) = delete;
		EventManager(EventManager&& other) = delete;
		EventManager& operator=(const EventManager& other) = delete;
		EventManager& operator=(EventManager&& other) = delete;
		~EventManager();

	private:
		EventManager(); //EventManager should never be created outside

	public:
		template<typename T>
		size_t subscribe(std::function<void(const T&)> callback);
		template<typename T>
		size_t subscribeNonConst(std::function<void(T&)> callback);

		template<typename T>
		void unsubscribe(size_t index);
		template<typename T>
		void unsubscribeNonConst(size_t index);

		template<typename T>
		void invoke(T* data);
		template<typename T>
		void invokeNonConst(T* data);

	private:
		std::vector<std::function<void(const void*)>>* listenersByEvent_;
		std::vector<std::function<void(void*)>>* listenersByEventNonConst_;
		std::vector<size_t>* availableListenerIndexes_;
		std::vector<size_t>* availableListenerIndexesNonConst_;
	};

	template <typename T>
	size_t EventManager::subscribe(std::function<void(const T&)> callback) {
		const uint32 type = T::type();
		ASSERT_CRITICAL(type < MAX_EVENTS, "There is no such event type");

		EventCallbackWrapper<T> wrapper(callback);
		std::vector<size_t>& indexVector = availableListenerIndexes_[type];
		std::vector<std::function<void(const void*)>>& callbackVector = listenersByEvent_[type];
		if (!indexVector.empty()) {
			const size_t index = indexVector.back();
			indexVector.pop_back();
			callbackVector[index] = wrapper;
			return index;
		}

		callbackVector.push_back(wrapper);
		return callbackVector.size() - 1;
	}

	template <typename T>
	size_t EventManager::subscribeNonConst(std::function<void(T&)> callback) {
		const uint32 type = T::type();
		ASSERT_CRITICAL(type < MAX_EVENTS, "There is no such event type");

		EventCallbackWrapperNonConst<T> wrapper(callback);
		std::vector<size_t>& indexVector = availableListenerIndexesNonConst_[type];
		std::vector<std::function<void(void*)>>& callbackVector = listenersByEventNonConst_[type];
		if (!indexVector.empty()) {
			const size_t index = indexVector.back();
			indexVector.pop_back();
			callbackVector[index] = wrapper;
			return index;
		}

		callbackVector.push_back(wrapper);
		return callbackVector.size() - 1;
	}

	template <typename T>
	void EventManager::unsubscribe(const size_t index) {
		if (listenersByEvent_ == nullptr) //EventManager already destroyed
			return;

		const uint32 type = T::type();
		ASSERT_CRITICAL(type < MAX_EVENTS, "There is no such event type");

		std::vector<size_t>& indexVector = availableListenerIndexes_[type];
		std::vector<std::function<void(const void*)>>& callbackVector = listenersByEvent_[type];
		ASSERT_CRITICAL(index < callbackVector.size(), "Incorrect index");

		callbackVector[index] = nullptr;
		indexVector.push_back(index);
	}

	template <typename T>
	void EventManager::unsubscribeNonConst(size_t index) {
		if (listenersByEventNonConst_ == nullptr) //EventManager already destroyed
			return;

		const uint32 type = T::type();
		ASSERT_CRITICAL(type < MAX_EVENTS, "There is no such event type");

		std::vector<size_t>& indexVector = availableListenerIndexesNonConst_[type];
		std::vector<std::function<void(void*)>>& callbackVector = listenersByEventNonConst_[type];
		ASSERT_CRITICAL(index < callbackVector.size(), "Incorrect index");

		callbackVector[index] = nullptr;
		indexVector.push_back(index);
	}

	template <typename T>
	void EventManager::invoke(T* data) {
		const uint32 type = T::type();
		ASSERT_CRITICAL(type < MAX_EVENTS, "There is no such event type");

		const std::vector<std::function<void(const void*)>>& callbackVector = listenersByEvent_[type];
		for (const auto& it : callbackVector) {
			if (!it)
				continue;
			it(data);
		}

		delete data;
	}

	template <typename T>
	void EventManager::invokeNonConst(T* data) {
		const uint32 type = T::type();
		ASSERT_CRITICAL(type < MAX_EVENTS, "There is no such event type");

		const std::vector<std::function<void(void*)>>& callbackVector = listenersByEventNonConst_[type];
		for (const auto& it : callbackVector) {
			if (!it)
				continue;
			it(data);
		}

		//no deleting of pointer, user is responsible for it
	}
}
