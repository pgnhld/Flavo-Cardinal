#pragma once

#include "Global.h"
#include <chrono>
#include "FWindow.h"
#include "Network/NetworkManager.h"
#include "FRendererD3D11.h"
#include "EECS.h"

namespace ft_game
{
	class Core : public eecs::IInvoker
	{
	public:
		Core(HINSTANCE hInstance, int nCmdShow);

	public:
		bool loop();

		void cleanup();

	private:
		/* Inverse of average frequency of fixedUpdate() */
		const double FIXED_UPDATE_PERIOD = 1.0 / 50.0;
		/* Used to prevent physics apocalypse after enormous lag */
		const uint32 FIXED_UPDATE_PER_FRAME_CAP = 8; 

		bool bToExit_;
		std::chrono::time_point<std::chrono::high_resolution_clock> lastLoopTime_;
		double fixedUpdateTimer_;

		framework::FWindow* window_;
		ft_engine::NetworkManager* networkManager_;
		framework::FRendererD3D11*	rendererD3D11_;
	};
}
