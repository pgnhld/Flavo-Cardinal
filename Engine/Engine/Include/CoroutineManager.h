#pragma once

#include "Global.h"
#include <vector>
#include <memory>
#include "FTime.h"
#include "Generator.h"
#include <functional>

/* Starts new coroutine e.g.
 * Coroutine coroutineIndex = START_COROUTINE(&SomeClass:someMethod, SomeDataStruct*, new SomeDataStruct());
 * arg can be also nullptr if it is not necessary
 * someMethod signature has to be: IEnumerator someMethod(CoroutineArg data)
 * return value can be used to STOP_COROUTINE
 * arg does not need to be manually removed
 */
#define START_COROUTINE(func, T, arg) \
ft_engine::CoroutineManager::startCoroutine(std::bind(func, this, std::placeholders::_1), [](CoroutineArg data){ delete static_cast<T>(data); }, arg)

/* Stops existing coroutine e.g.
 * Coroutine coroutineIndex = START_COROUTINE(&SomeClass:someMethod, new SomeDataStruct());
 * //... some code ...
 * STOP_COROUTINE(coroutineIndex);
 * coroutineIndex has to be valid and returned by START_COROUTINE
 */
#define STOP_COROUTINE(coroutine) \
ft_engine::CoroutineManager::stopCoroutine(coroutine)

/* Wait with coroutine execution for t seconds e.g.
 * doSth();
 * YIELD_WAIT_SECONDS(2.0);
 * doAnotherThing(); //will be invoked after 2 seconds or a little bit more (depends on framerate)
 */
#define YIELD_WAIT_SECONDS(t) \
{ double timer = 0.0; while (timer < t) { co_yield true; timer += framework::FTime::deltaTime; } }

/* Wait with coroutine execution till next frame
 * Useful when interpolating some values
 */
#define YIELD_RETURN_NULL()\
{ co_yield true; }

/* Stops coroutine execution and remove it from active coroutine list
 * CoroutineArg pointer is automatically removed
 */
#define YIELD_BREAK()\
{ co_yield false; }

/* Required return type of all Coroutine-type functions
 * Not really IEnumerator but it makes functions similar to Unity
 */
typedef ft_engine::generator<bool> IEnumerator;

/* Coroutine only argument of any type
 * Needs to be casted back from CoroutineArg to T via static_cast<T>(...)
 * Can be nullptr
 */
typedef void* CoroutineArg;

/*
 * Index used to stopCoroutine() if it needs to be removed early
 */
struct Coroutine
{
	Coroutine();
	Coroutine(const Coroutine& another) = default;
	Coroutine(Coroutine&& another) = default;

	Coroutine& operator=(const Coroutine& another) = default;
	Coroutine& operator=(Coroutine&& another) = default;

	void init(size_t index, size_t counter);

	size_t index;
	size_t counter;
};

namespace ft_engine
{
	class CoroutineManager
	{
	public:
		static CoroutineManager& getInstance();
		static bool isRunning(Coroutine coroutine);
		static Coroutine startCoroutine(std::function<IEnumerator(CoroutineArg)> coroutine, const std::function<void(CoroutineArg)>& deleter, CoroutineArg data);
		static void stopCoroutine(Coroutine& coroutine);
		/* Used by SceneManager while relaoding scene */
		static void stopAllCoroutines();

		void update();

	private:
		struct CoroutineContainer;

		std::vector<CoroutineContainer*> coroutines_;
		std::vector<size_t> coroutineCounters_;
		std::vector<size_t> emptyCoroutineSlots_;
	};

	struct CoroutineManager::CoroutineContainer
	{
		CoroutineContainer(std::function<IEnumerator(CoroutineArg)> function, std::function<void(CoroutineArg)> deleter, CoroutineArg data);
		~CoroutineContainer();

		IEnumerator generator;
		IEnumerator::iterator iterator;
		bool bStarted;
		std::function<void(CoroutineArg)> deleter;
		CoroutineArg pointer;
	};
}
