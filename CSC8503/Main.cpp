#include "Window.h"

#include "Debug.h"

#include "NetworkedGame.h"

using namespace NCL;
using namespace CSC8503;

#include <chrono>
#include <thread>
#include <sstream>

#include "ApplicationState.h"

int main() {
	Window*w = Window::CreateGameWindow("CSC8503 Game technology!", 1280, 720 , false);

	if (!w->HasInitialised()) {
		return -1;
	}	
	
	w->ShowOSPointer(true);
	w->LockMouseToWindow(false);

	NetworkedGame* g = new NetworkedGame();
	w->GetTimer().GetTimeDeltaSeconds(); //Clear the timer so we don't get a larget first dt!
	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyCodes::ESCAPE)) {
		float dt = w->GetTimer().GetTimeDeltaSeconds();
		if (dt > 0.2f) {
			std::cout << "Skipping large time delta" << std::endl;
			continue; //must have hit a breakpoint or something to have a 1 second frame time!
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::PRIOR)) {
			w->ShowConsole(true);
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::NEXT)) {
			w->ShowConsole(false);
		}

		if (Window::GetKeyboard()->KeyPressed(KeyCodes::T)) {
			w->SetWindowPosition(0, 0);
		}

		w->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));


		g->UpdateGame(dt);
	}
	
	// Singleton cleanup
	ApplicationState::Destory();
#ifdef _WIN32
	UIWindows::Destroy();
#else //_ORBIS

#endif

	Window::DestroyGameWindow();
}