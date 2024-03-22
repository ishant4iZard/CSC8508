#include <chrono>
#include <thread>
#include <sstream>

#include "ApplicationState.h"
#include "Debug.h"
#include "Event.h"
#include "Window.h"

#ifdef _WIN32
#include <steam_api.h>
#include "UIWindows.h"
#include "NetworkedGame.h"
#include "MenuSystem.h"

#else // PROSPERO
#include "UIPlaystation.h"
#include "PS5_Game.h"
#include "PS5Window.h"
#include "PS5Controller.h"
#include "GameTechAGCRenderer.h"

size_t sceUserMainThreadStackSize = 2 * 1024 * 1024;
extern const char sceUserMainThreadName[] = "TeamProjectGameMain";
int sceUserMainThreadPriority = SCE_KERNEL_PRIO_FIFO_DEFAULT;
size_t sceLibcHeapSize = 256 * 1024 * 1024;
#endif

using namespace NCL;

std::multimap<EventType, EventListener*> EventEmitter::listeners;

int main() {

	Window* w = Window::CreateGameWindow("CSC8503 Game technology!", 1920, 1080, true);
	if (!w->HasInitialised()) return -1;

	bool bIsNetSystemInitSuccess = false;
#ifdef _WIN32
	/** Check the NetSubsystem work condition */
	if (SteamAPI_Init())
	{
		bIsNetSystemInitSuccess = true;
		std::cout << "Steam API initialized successfully.\n";
	}
	else
	{
		std::cout << "Steam API failed to initialize.\n";
		std::cout << "You may need run the steam app.\n";
	}

	w->ShowOSPointer(true);
	w->LockMouseToWindow(false);

	NetworkedGame* g = new NetworkedGame();
	
	std::string IPAdd;
	w->GetLocalIPV4Address(IPAdd);
	g->GetMenuSystem()->SetIsNetsystemInitSuccess(bIsNetSystemInitSuccess);
	g->GetMenuSystem()->SetLocalIPv4Address(IPAdd);
#else
	PS5_Game* g = new PS5_Game();
#endif

	w->GetTimer().GetTimeDeltaSeconds(); //Clear the timer so we don't get a larget first dt!
	while (w->UpdateWindow() && !g->CloseGame) {
		float dt = w->GetTimer().GetTimeDeltaSeconds();

#ifdef _WIN32
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::PRIOR)) {
			w->ShowConsole(true);
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::NEXT)) {
			w->ShowConsole(false);
		}

		if (Window::GetKeyboard()->KeyPressed(KeyCodes::T)) {
			w->SetWindowPosition(0, 0);
		}

		if (Window::GetKeyboard()->KeyDown(KeyCodes::ESCAPE))
			break;

		w->SetTitle("Frame Rate : " + std::to_string(1.0f / dt));

		/** NetSubsystem Callback Event */
		if (bIsNetSystemInitSuccess)
			SteamAPI_RunCallbacks();
#endif
		g->UpdateGame(dt);
	}

#ifdef _WIN32
	if (bIsNetSystemInitSuccess) SteamAPI_Shutdown();
	UIWindows::Destroy();
#else // PROSPERO
	UIPlaystation::Destroy();
#endif

	ApplicationState::Destory();
	Window::DestroyGameWindow();
}