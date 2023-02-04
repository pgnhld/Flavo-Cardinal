#pragma once

#include "Global.h"
#include <chrono>
#include "EECS.h"
#include "EngineEvent.h"

namespace ft_editor
{
	class Core : eecs::IReceiver<Core>, eecs::IInvoker
	{
	public:
		Core(HINSTANCE hInstance, int nCmdShow);
		~Core();

	public:
		bool loop();

	private:
		void onPostSceneCreated(const EventPostSceneCreated& event);
		void onPostSceneLoaded(const EventPostSceneLoaded& event);

		/* Used to prevent physics apocalypse after enormous lag */
		const uint32 FIXED_UPDATE_PER_FRAME_CAP = 10; 

		bool bToExit_;
		std::chrono::time_point<std::chrono::high_resolution_clock> lastLoopTime_;
		float fixedUpdateTimer_;
	};
}
