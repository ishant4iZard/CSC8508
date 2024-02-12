#include "Window.h"

#include "Debug.h"

#include "NetworkedGame.h"
#include "MenuSystem.h"

using namespace NCL;
using namespace CSC8503;

#include <chrono>
#include <thread>
#include <sstream>
#include <steam_api.h>


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

		w->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));

		SteamAPI_RunCallbacks();

		g->UpdateGame(dt);
	}

	SteamAPI_Shutdown();

	Window::DestroyGameWindow();

}