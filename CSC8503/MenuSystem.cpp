#include "MenuSystem.h"
#include "NetworkedGame.h"
#include "PushdownMachine.h"

#include "steam.h"

using namespace NCL;
using namespace CSC8503;

MenuSystem::MenuSystem(NetworkedGame* Game)
{
	steam = new NetSystem_Steam();

	MenuMachine = new PushdownMachine(new MainMenu());
	MenuMachine->SetGame(Game);
	MenuMachine->SetSteam(steam);
}

MenuSystem::~MenuSystem()
{
	delete MenuMachine;
	delete steam;
}

void MenuSystem::Update(float dt)
{
	MenuMachine->Update(dt);
}


PushdownState::PushdownResult MainMenu::OnUpdate(float dt, PushdownState** newState)
{
	if (game && steam)
	{
		Debug::Print("Press 1 : Create Lobby", Vector2(5, 23), Debug::YELLOW);
		Debug::Print("Press 2 : Search Lobbies", Vector2(5, 33), Debug::YELLOW);

		NetSystem_Steam* Steam = (NetSystem_Steam*)steam;
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM1))
		{
			Steam->CreateLobby();
		}
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM2))
		{
			Steam->SearchLobbies();
		}
	}
	return PushdownResult::NoChange;
}
