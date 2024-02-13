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

void MenuSystem::SetLocalIPv4Address(const std::string& IP)
{
	steam->SetLocalIPv4Address(IP);
}

/** Main Menu Update */
PushdownState::PushdownResult MainMenu::OnUpdate(float dt, PushdownState** newState)
{
	if (game && steam)
	{
		NetSystem_Steam* Steam = (NetSystem_Steam*)steam;

		Debug::Print("Press 1 : Create Lobby", Vector2(5, 23), Debug::YELLOW);
		Debug::Print("Press 2 : Join Lobby", Vector2(5, 33), Debug::YELLOW);

		switch (Steam->GetUserCurrentState())
		{
		case NetSystem_Steam::EUserState::ELobbyHolder:
			*newState = new MultiPlayerLobby();
			return PushdownResult::Push;
			break;
		case NetSystem_Steam::EUserState::EAvailable:
			if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM1))
			{
				Steam->CreateLobby();
			}
			if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM2))
			{
				*newState = new MultiplayerSearchMenu();
				return PushdownResult::Push;
			}
			break;
		}
	}
	return PushdownResult::NoChange;
}

/** Multiplayer Lobby Update */
PushdownState::PushdownResult MultiPlayerLobby::OnUpdate(float dt, PushdownState** newState)
{
	if (game && steam)
	{
		NetSystem_Steam* Steam = (NetSystem_Steam*)steam;
		NetworkedGame* Game = (NetworkedGame*)game;

		Debug::Print("Your Name: " + Steam->GetCurrentUserName(), Vector2(5, 13), Debug::YELLOW);
		Debug::Print("===============================================", Vector2(5, 30), Debug::YELLOW);
		Debug::Print("===============================================", Vector2(5, 65), Debug::YELLOW);
		Debug::Print("Press S: Start Game", Vector2(5, 75), Debug::YELLOW);

		switch (Steam->GetUserCurrentState())
		{
		case NetSystem_Steam::EUserState::ELobbyHolder:
			Debug::Print("You are the holder!", Vector2(5, 23), Debug::YELLOW);
			if (Window::GetKeyboard()->KeyPressed(KeyCodes::S))
			{
				Game->StartAsServer();
				*newState = new PlayingHUD();
				return PushdownResult::Push;
			}
			break;
		case NetSystem_Steam::EUserState::ELobbyJoiner:
			Debug::Print("You are the joiner!", Vector2(5, 23), Debug::YELLOW);
			if (Window::GetKeyboard()->KeyPressed(KeyCodes::S))
			{
				Game->StartAsClient(GetIPnumByIndex(0), GetIPnumByIndex(1), GetIPnumByIndex(2), GetIPnumByIndex(3));
				*newState = new PlayingHUD();
				return PushdownResult::Push;
			}
			break;
		}
	}
	return PushdownResult::NoChange;
}

char MultiPlayerLobby::GetIPnumByIndex(int index)
{
	NetSystem_Steam* Steam = (NetSystem_Steam*)steam;
	string IP = Steam->GetLobbyHolderIPv4Address();
	int PointAccml = 0;
	int IPnum = 0;
	for (auto i : IP)
	{
		if (i == '.')
		{
			++PointAccml;
			continue;
		}
		if (PointAccml > index)
		{
			break;
		}
		if (PointAccml == index)
		{
			IPnum = IPnum * 10 + i - 48;
		}
	}
	return IPnum;
}

/** Multiplayer Search Menu Update */
PushdownState::PushdownResult MultiplayerSearchMenu::OnUpdate(float dt, PushdownState** newState)
{
	if (game && steam)
	{
		NetSystem_Steam* Steam = (NetSystem_Steam*)steam;

		Debug::Print("Your Name: " + Steam->GetCurrentUserName(), Vector2(5, 13), Debug::YELLOW);
		Debug::Print("Lobbies list", Vector2(38, 30), Debug::YELLOW);
		Debug::Print("===============================================", Vector2(5, 35), Debug::YELLOW);
		Debug::Print("===============================================", Vector2(5, 70), Debug::YELLOW);
		Debug::Print("Press S: Join the selected lobby", Vector2(5, 70), Debug::YELLOW);
		
		switch (Steam->GetUserCurrentState())
		{
		case NetSystem_Steam::EUserState::EAvailable:
			Debug::Print("Press 1: Refresh Lobby List", Vector2(5, 23), Debug::YELLOW);
			if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM1))
			{
				Steam->SearchLobbies();
				CurrentSelectLobby = 0;
			}
			DisplaySearchResult(Steam);
			if (Window::GetKeyboard()->KeyPressed(KeyCodes::S))
			{
				Steam->SelectLobbyByIndex(CurrentSelectLobby);
				Steam->JoinLobby();
			}
			break;
		case NetSystem_Steam::EUserState::ESearching:
			Debug::Print("Searching Available Lobbies...", Vector2(5, 23), Debug::YELLOW);
			break;
		case  NetSystem_Steam::EUserState::ELobbyJoiner:
			*newState = new MultiPlayerLobby();
			return PushdownResult::Push;
			break;
		}

	}
	return PushdownResult::NoChange;
}

void MultiplayerSearchMenu::DisplaySearchResult(NetSystem_Steam* Steam)
{
	int num = Steam->GetNumOfLobbyMatchList();
	if (num == -1 || num == 0)
	{
		Debug::Print("There is no lobby existed.", Vector2(5, 42), Debug::YELLOW);
		return;
	}

	Debug::Print("->", Vector2(5, 42), Debug::RED);
	for (int i = 0; i < 4; ++i)
	{
		int CurrentDisplayLobbyIndex = CurrentSelectLobby + i;
		if (CurrentDisplayLobbyIndex < num)
		{
			Debug::Print(DisplayLobbyLine(Steam, CurrentDisplayLobbyIndex), Vector2(12, 42 + i * 7), Debug::YELLOW);
		}
	}
}

string MultiplayerSearchMenu::DisplayLobbyLine(NetSystem_Steam* Steam, int Index)
{
	string Message = std::to_string(Index);
	Message += "       ";
	Message += Steam->GetLobbyOwnerNameByLobbyID(Steam->GetLobbyIDByIndex(Index));
	return Message;
}

/** PlaingHUD update */
PushdownState::PushdownResult PlayingHUD::OnUpdate(float dt, PushdownState** newState)
{
	return PushdownResult::NoChange;
}
