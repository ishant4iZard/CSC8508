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


#ifdef _WIN32
	//using namespace CSC8503;
#endif
using namespace NCL;


std::multimap<EventType, EventListener*> EventEmitter::listeners;

int main() {


#ifndef _WIN32

	NCL::PS5::PS5Window* w  = new NCL::PS5::PS5Window("Hello!", 1920, 1080);

	if (!w->HasInitialised()) {
		return -1;
	}

	NCL::PS5::PS5Controller* c = w->GetController();

	c->MapAxis(0, "LeftX");
	c->MapAxis(1, "LeftY");

	c->MapAxis(2, "RightX");
	c->MapAxis(3, "RightY");

	c->MapAxis(4, "DX");
	c->MapAxis(5, "DY");

	c->MapButton(0, "Triangle");
	c->MapButton(1, "Circle");
	c->MapButton(2, "Cross");
	c->MapButton(3, "Square");

	//These are the axis/button aliases the inbuilt camera class reads from:
	c->MapAxis(0, "XLook");
	c->MapAxis(1, "YLook");

	c->MapAxis(2, "Sidestep");
	c->MapAxis(3, "Forward");

	c->MapButton(0, "Up");
	c->MapButton(2, "Down");

	GameWorld* world = new GameWorld();
	GameTechAGCRenderer* renderer = new GameTechAGCRenderer(*world);
	PhysicsSystem* physics = new PhysicsSystem(*world);

	PS5_Game* g = new PS5_Game(*world, *renderer, *physics);

	while (w->UpdateWindow()) {
		float dt = w->GetTimer().GetTimeDeltaSeconds();
		g->UpdateGame(dt);
		renderer->Update(dt);
		renderer->Render();
	}

	delete physics;
	delete renderer;
	delete world;
#else
	/** Check the NetSubsystem work condition */
	bool bIsNetSystemInitSuccess = false;
#ifdef _WIN32
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
#else
	// Check Playstation subsystem
#endif

	Window*w = Window::CreateGameWindow("CSC8503 Game technology!", 1920, 1080 , true);

	if (!w->HasInitialised()) {
		return -1;
	}	
	

	w->ShowOSPointer(true);
	w->LockMouseToWindow(false);

	NetworkedGame* g = new NetworkedGame();

	std::string IPAdd;
	w->GetLocalIPV4Address(IPAdd);
	g->GetMenuSystem()->SetIsNetsystemInitSuccess(bIsNetSystemInitSuccess);
	g->GetMenuSystem()->SetLocalIPv4Address(IPAdd);

	w->GetTimer().GetTimeDeltaSeconds(); //Clear the timer so we don't get a larget first dt!
	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyCodes::ESCAPE)) {
		float dt = w->GetTimer().GetTimeDeltaSeconds();

		if (Window::GetKeyboard()->KeyPressed(KeyCodes::PRIOR)) {
			w->ShowConsole(true);
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::NEXT)) {
			w->ShowConsole(false);
		}

		if (Window::GetKeyboard()->KeyPressed(KeyCodes::T)) {
			w->SetWindowPosition(0, 0);
		}

		w->SetTitle("Frame Rate : " + std::to_string(1.0f / dt));

/** NetSubsystem Callback Event */
		if (bIsNetSystemInitSuccess)
		{
#ifdef _WIN32
			SteamAPI_RunCallbacks();
#else

#endif
		}

		g->UpdateGame(dt);
	}
	
#ifdef _WIN32
	if (bIsNetSystemInitSuccess) SteamAPI_Shutdown();
	UIWindows::Destroy();
#else //_ORBIS
	UIPlaystation::Destroy();
#endif
	ApplicationState::Destory();
	Window::DestroyGameWindow();
#endif

}