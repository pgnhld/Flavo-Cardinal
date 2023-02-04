#include "SystemHandle.h"
#include "Assertion.h"

eecs::SystemHandle::SystemHandle(std::unique_ptr<SystemBase> system, double period)
	: system_(std::move(system)), period_(period), timer_(0.0) {

}

eecs::SystemHandle::~SystemHandle() {
	system_.reset(nullptr);
}

void eecs::SystemHandle::update(EntityManager& entities, double deltaTime) {
	if (system_ == nullptr)
		return;

	timer_ += deltaTime;
	if (timer_ < period_)
		return;

	const float lastTimer = timer_;
	timer_ = 0.0;
	system_->update(entities, lastTimer);
}

void eecs::SystemHandle::fixedUpdate(EntityManager& entities, double fixedDeltaTime) {
	ASSERT_CRITICAL(system_ != nullptr, "System should not be nullptr");
	system_->fixedUpdate(entities, fixedDeltaTime);
}
