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

		ui->DrawButton(
			"Create Lobby",
			Vector2(5, 23),
			[]() {
				EventEmitter* emitter = EventEmitter::GetInstance();
				emitter->EmitEvent(CREATE_LOBBY);
			},
			UIBase::WHITE,
			KeyCodes::NUM1 // Only for PS
		);

		ui->DrawButton(
			"Join Lobby",
			Vector2(5, 33),
			[]() {
				EventEmitter* emitter = EventEmitter::GetInstance();
				emitter->EmitEvent(JOIN_LOBBY);
			},
			UIBase::WHITE,
			KeyCodes::NUM2 // Only for PS
		);

		switch (Steam->GetUserCurrentState())
		{
		case NetSystem_Steam::EUserState::ELobbyHolder:
			*newState = new MultiPlayerLobby();
			return PushdownResult::Push;
			break;
		case NetSystem_Steam::EUserState::EAvailable:
			if (isCreatingLobby)
			{
				Steam->CreateLobby();
			}
			if (isJoiningLobby)
			{
				*newState = new MultiplayerSearchMenu();
				return PushdownResult::Push;
			}
			break;
		}
	}
	return PushdownResult::NoChange;
}

void MainMenu::ReceiveEvent(const EventType eventType) {
	switch (eventType)
	{
	case JOIN_LOBBY :
		isJoiningLobby = true;
		break;

	case CREATE_LOBBY :
		isCreatingLobby = true;
		break;

	default:
		break;
	}
}

/** Multiplayer Lobby Update */
PushdownState::PushdownResult MultiPlayerLobby::OnUpdate(float dt, PushdownState** newState)
{
	if (game && steam)
	{
		NetSystem_Steam* Steam = (NetSystem_Steam*)steam;
		NetworkedGame* Game = (NetworkedGame*)game;

		ui->DrawStringText("UserName: " + Steam->GetCurrentUserName(), Vector2(5, 13), UIBase::WHITE);
		ui->DrawStringText("______________________________________________________________________________________________", Vector2(5, 30), UIBase::BLACK);
		ui->DrawStringText("______________________________________________________________________________________________", Vector2(5, 65), UIBase::BLACK);

		switch (Steam->GetUserCurrentState())
		{
		case NetSystem_Steam::EUserState::ELobbyHolder:
			ui->DrawStringText("You are the holder!", Vector2(5, 18), UIBase::WHITE);
			ui->DrawButton(
				"Start Game",
				Vector2(5, 75),
				[]() {
					EventEmitter* emitter = EventEmitter::GetInstance();
					emitter->EmitEvent(START_AS_SERVER);
				},
				UIBase::WHITE,
				KeyCodes::S // Only for PS
				);
			if (appState->GetIsServer())
			{
				Game->StartAsServer();
				*newState = new PlayingHUD();
				return PushdownResult::Push;
			}
			break;
		case NetSystem_Steam::EUserState::ELobbyJoiner:
			ui->DrawStringText("You are the joiner!", Vector2(5, 23), UIBase::WHITE);
			ui->DrawButton(
				"Start Game",
				Vector2(5, 75),
				[]() {
					EventEmitter* emitter = EventEmitter::GetInstance();
					emitter->EmitEvent(START_AS_CLIENT);
				},
				UIBase::WHITE,
				KeyCodes::S // Only for PS
				);
			if (appState->GetIsClient())
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

void MultiPlayerLobby::ReceiveEvent(const EventType eventType) {
	switch (eventType) {
	case START_AS_SERVER :
		appState->SetIsServer(true);
		break;

	case START_AS_CLIENT :
		appState->SetIsClient(true);
		break;

	default :
		break;
	}
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

		ui->DrawStringText("UserName: " + Steam->GetCurrentUserName(), Vector2(5, 13), UIBase::WHITE);
		ui->DrawStringText("Available Lobbies :", Vector2(5, 30), UIBase::WHITE);
		ui->DrawStringText("______________________________________________________________________________________________", Vector2(5, 35), UIBase::BLACK);
		ui->DrawStringText("______________________________________________________________________________________________", Vector2(5, 70), UIBase::BLACK);
		ui->DrawButton(
			"Join Selected Lobby",
			Vector2(5, 75),
			[]() {
				EventEmitter* emitter = EventEmitter::GetInstance();
				emitter->EmitEvent(JOIN_CURRENT_LOBBY);
			},
			UIBase::WHITE,
			KeyCodes::S // Only for PS
		);
		
		switch (Steam->GetUserCurrentState())
		{
		case NetSystem_Steam::EUserState::EAvailable:
			ui->DrawButton(
				"Refresh Lobby List",
				Vector2(5, 83),
				[]() {
					EventEmitter* emitter = EventEmitter::GetInstance();
					emitter->EmitEvent(REFRESH_LOBBY);
				},
				UIBase::WHITE,
				KeyCodes::NUM1 // Only for PS
				);
			if (shouldRefreshLobbyList)
			{
				Steam->SearchLobbies();
				CurrentSelectLobby = 0;
				shouldRefreshLobbyList = false;
			}
			DisplaySearchResult(Steam);
			if (shouldJoinSelectedLobby)
			{
				if (CurrentSelectLobby == 0) {

				}

				Steam->SelectLobbyByIndex(CurrentSelectLobby);
				Steam->JoinLobby();
				shouldJoinSelectedLobby = false;
			}
			break;
		case NetSystem_Steam::EUserState::ESearching:
			ui->DrawStringText("Searching For Lobbies...", Vector2(5, 18), UIBase::WHITE);
			break;
		case  NetSystem_Steam::EUserState::ELobbyJoiner:
			*newState = new MultiPlayerLobby();
			return PushdownResult::Push;
			break;
		}

	}
	return PushdownResult::NoChange;
}

void MultiplayerSearchMenu::ReceiveEvent(const EventType eventType) {
	switch (eventType) {
		case JOIN_CURRENT_LOBBY:
			shouldJoinSelectedLobby = true;
			break;

		case REFRESH_LOBBY :
			shouldRefreshLobbyList = true;
			break;

		default : break;
	}
}

void MultiplayerSearchMenu::DisplaySearchResult(NetSystem_Steam* Steam)
{
	int num = Steam->GetNumOfLobbyMatchList();
	if (num == -1 || num == 0)
	{
		//Debug::Print("There is no lobby existed. :-(", Vector2(5, 42), Debug::YELLOW);
		ui->DrawStringText("No Lobbies Found :-(", Vector2(5, 42), UIBase::RED);
		return;
	}

	//Debug::Print("->", Vector2(5, 42), Debug::RED);
	ui->DrawStringText("->", Vector2(5, 42), UIBase::RED);
	for (int i = 0; i < 4; ++i)
	{
		int CurrentDisplayLobbyIndex = CurrentSelectLobby + i;
		if (CurrentDisplayLobbyIndex < num)
		{
			//Debug::Print(DisplayLobbyLine(Steam, CurrentDisplayLobbyIndex), Vector2(12, 42 + i * 7), Debug::YELLOW);
			ui->DrawStringText(DisplayLobbyLine(Steam, CurrentDisplayLobbyIndex), Vector2(12, 42 + i * 7), UIBase::RED);
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
