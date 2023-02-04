#include "CoroutineManager.h"
#include <utility>
#include "Assertion.h"
#include "Logger.h"

ft_engine::CoroutineManager& ft_engine::CoroutineManager::getInstance() {
	static CoroutineManager instance;
	return instance;
}

bool ft_engine::CoroutineManager::isRunning(Coroutine coroutine) {
	CoroutineManager& manager = getInstance();
	return (coroutine.index < manager.coroutines_.size()) 
		&& (coroutine.counter == manager.coroutineCounters_[coroutine.index]);
}

Coroutine ft_engine::CoroutineManager::startCoroutine(std::function<IEnumerator(CoroutineArg)> coroutine, const std::function<void(CoroutineArg)>& deleter, CoroutineArg data) {
	Coroutine retCoroutine;
	CoroutineManager& manager = getInstance();
	if (manager.emptyCoroutineSlots_.empty()) {
		manager.coroutines_.push_back(new CoroutineContainer(coroutine, deleter, data));
		manager.coroutineCounters_.push_back(1); //starting from 1, 0 is always invalid counter
		retCoroutine.init(manager.coroutines_.size() - 1, manager.coroutineCounters_.back());
		return retCoroutine;
	}

	const size_t slot = manager.emptyCoroutineSlots_.back();
	manager.coroutines_[slot] = new CoroutineContainer(coroutine, deleter, data);
	manager.emptyCoroutineSlots_.pop_back();
	retCoroutine.init(slot, manager.coroutineCounters_[slot]);
	return retCoroutine;
}

void ft_engine::CoroutineManager::stopCoroutine(Coroutine& coroutine) {
	if (!isRunning(coroutine))
		return;

	CoroutineManager& manager = getInstance();
	ASSERT_CRITICAL(manager.coroutines_[coroutine.index] != nullptr, "Coroutine has already been removed");

	manager.emptyCoroutineSlots_.push_back(coroutine.index);
	delete manager.coroutines_[coroutine.index];
	manager.coroutines_[coroutine.index] = nullptr;
	manager.coroutineCounters_[coroutine.index]++;
}

void ft_engine::CoroutineManager::stopAllCoroutines() {
	CoroutineManager& manager = getInstance();
	for (size_t i = 0; i < manager.coroutines_.size(); ++i) {
		if (manager.coroutines_[i] == nullptr)
			continue;

		manager.emptyCoroutineSlots_.push_back(i);
		delete manager.coroutines_[i];
		manager.coroutines_[i] = nullptr;
		manager.coroutineCounters_[i]++;
	}
}

void ft_engine::CoroutineManager::update() {
	for (size_t i = 0; i < coroutines_.size(); ++i) {
		Coroutine coro; coro.init(i, coroutineCounters_[i]);
		CoroutineContainer* container = coroutines_[i];
		if (container == nullptr) { //stopped earlier
			continue;
		}

		if (!container->bStarted) { //recently added
			container->iterator = container->generator.begin();
			container->bStarted = true;
			continue;
		}

		if (container->iterator == container->generator.end()) { //already finished
			stopCoroutine(coro);
			continue;
		}

		++container->iterator;
		if (container->iterator == container->generator.end()) { //already finished
			stopCoroutine(coro);
			continue;
		}

		if (!*container->iterator) { //yield break
			stopCoroutine(coro);
		}
	}
}

ft_engine::CoroutineManager::CoroutineContainer::CoroutineContainer(std::function<IEnumerator(CoroutineArg)> function, std::function<void(CoroutineArg)> deleter, CoroutineArg data)
	: generator(function(data)), iterator(generator.end()), bStarted(false), deleter(std::move(deleter)), pointer(data) {

}

ft_engine::CoroutineManager::CoroutineContainer::~CoroutineContainer() {
	deleter(pointer);
}

Coroutine::Coroutine() : index(0), counter(0) {

}

void Coroutine::init(size_t index, size_t counter) {
	this->index = index;
	this->counter = counter;
}

