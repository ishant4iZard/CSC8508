#include "Window.h"

#include "Debug.h"

#include "NetworkedGame.h"
#include "MenuSystem.h"

using namespace NCL;
using namespace CSC8503;

#include <chrono>
#include <thread>
#include <sstream>

#include "ApplicationState.h"
#ifdef _WIN32
#include "UIWindows.h"
#else //_ORBIS
#include "UIPlaystation.h"
#endif
#include <steam_api.h>

#include "Event.h"
std::multimap<EventType, EventListener*> EventEmitter::listeners;

int main() {
	if (SteamAPI_Init())
	{
		std::cout << "Steam API initialized successfully.\n";
	}
	else
	{
		std::cout << "Steam API failed to initialize.\n";
		std::cout << "You may need run the steam app.\n";
	}

	Window*w = Window::CreateGameWindow("CSC8503 Game technology!", 1280, 720 , false);

	if (!w->HasInitialised()) {
		return -1;
	}	
	
	w->ShowOSPointer(true);
	w->LockMouseToWindow(false);

	NetworkedGame* g = new NetworkedGame();

	std::string IPAdd;
	w->GetLocalIPV4Address(IPAdd);
	g->GetMenuSystem()->SetLocalIPv4Address(IPAdd);

	w->GetTimer().GetTimeDeltaSeconds(); //Clear the timer so we don't get a larget first dt!
	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyCodes::ESCAPE)) {
		float dt = w->GetTimer().GetTimeDeltaSeconds();
		//if (dt > 0.2f) {
		//	std::cout << "Skipping large time delta" << std::endl;
		//	continue; //must have hit a breakpoint or something to have a 1 second frame time!
		//}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::PRIOR)) {
			w->ShowConsole(true);
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::NEXT)) {
			w->ShowConsole(false);
		}

		if (Window::GetKeyboard()->KeyPressed(KeyCodes::T)) {
			w->SetWindowPosition(0, 0);
		}

		//w->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));
		w->SetTitle("Frame Rate : " + std::to_string(1.0f / dt));

		SteamAPI_RunCallbacks();

		g->UpdateGame(dt);
	}
	
	// Singleton cleanup
	ApplicationState::Destory();
#ifdef _WIN32
	UIWindows::Destroy();
#else //_ORBIS
	UIPlaystation::Destroy();
#endif

	SteamAPI_Shutdown();

	Window::DestroyGameWindow();
}