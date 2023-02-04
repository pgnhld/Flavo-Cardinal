#include "FTime.h"

using namespace framework;

float FTime::deltaTime = 0.0f;

const float FTime::defaultFixedDeltaTime = 1.0f / 50.0f;
float FTime::fixedDeltaTime = FTime::defaultFixedDeltaTime;

const float FTime::defaultTimeScale = 1.0f;
float FTime::timeScale = FTime::defaultTimeScale;

float FTime::timeSinceStart = 0.0f;

bool FTime::bSceneStarted = false;