#include "Events/EventManager.h"

eecs::EventManager& eecs::EventManager::getInstance() {
	static EventManager manager;
	return manager;
}

eecs::EventManager::EventManager()
	: listenersByEvent_(new std::vector<std::function<void(const void*)>>[MAX_EVENTS])
	, listenersByEventNonConst_(new std::vector<std::function<void(void*)>>[MAX_EVENTS]) 
	, availableListenerIndexes_(new std::vector<size_t>[MAX_EVENTS])
	, availableListenerIndexesNonConst_(new std::vector<size_t>[MAX_EVENTS]) {

}

eecs::EventManager::~EventManager() {
	delete[] listenersByEvent_; listenersByEvent_ = nullptr;
	delete[] availableListenerIndexes_; availableListenerIndexes_ = nullptr;
	delete[] listenersByEventNonConst_; listenersByEventNonConst_ = nullptr;
	delete[] availableListenerIndexesNonConst_; availableListenerIndexesNonConst_ = nullptr;
}
