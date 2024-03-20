#include "MenuSystem.h"
#include "TutorialGame.h"
#include "NetworkedGame.h"
#include "PushdownMachine.h"
#include "NetworkPlayer.h"

#include "OnlineSubsystemBase.h"
#include "steam.h"

// TODO sit with T and change everything to tutorial game and non network related

using namespace NCL;
using namespace CSC8503;

using std::string;

MenuSystem::MenuSystem(TutorialGame* Game)
{
#ifdef _WIN32
	OnlineSubsystem = NetSystem_Steam::GetInstance();
#else
	OnlineSubsystem = nullptr;
#endif
	MenuMachine = new PushdownMachine(new MainMenu());
	MenuMachine->SetGame(Game);
	MenuMachine->SetOnlineSubsystem(OnlineSubsystem);
}

MenuSystem::~MenuSystem()
{
	delete MenuMachine;
}

void MenuSystem::Update(float dt)
{
	MenuMachine->Update(dt);
}

void MenuSystem::SetIsNetsystemInitSuccess(bool result)
{
	if (OnlineSubsystem)
	{
		OnlineSubsystem->SetIsOnlineSubsystemInitSuccess(result);
	}
}

void MenuSystem::SetLocalIPv4Address(const std::string& IP)
{
	if (OnlineSubsystem)
	{
		OnlineSubsystem->SetLocalIPv4Address(IP);
	}
}  

/** Main Menu Update */
PushdownState::PushdownResult MainMenu::OnUpdate(float dt, PushdownState** newState)
{
	if (game && OnlineSubsystem)
	{
#ifdef _WIN32
		OnlineSubsystemBase* Subsystem = (OnlineSubsystemBase*)OnlineSubsystem;
		NetworkedGame* Game = (NetworkedGame*)game;
#else
#endif

		if (Subsystem && !Subsystem->GetIsOnlineSubsystemInitSuccess())
		{
			ui->DrawStringText("NetSystem init failed! Please check your steam or playstation 5 network state and restart game!", Vector2(5, 40), UIBase::RED);
			return PushdownResult::NoChange;
		}
		if(Subsystem)
		{
			Subsystem->SetCurrentUserName();
		}

		if (isSoloGameBtnPressed)
		{
			isSoloGameBtnPressed = false;
			Game->SetLocalPlayerIndex(0);
			appState->SetIsServer(true);
			appState->SetIsSolo(true);
		}
		if (isLobbyCreated)
		{
			Game->isDevMode = false;
			appState->SetIsSolo(false);
			*newState = new MultiPlayerLobby();
			isCreatingLobby = false;
			isLobbyCreated = false;
			appState->SetIsLobbyHolder(true);
			return Push;
		}
		if (isSearchLobbyBtnPressed)
		{
			Game->isDevMode = false;
			appState->SetIsSolo(false);
			isSearchLobbyBtnPressed = false;
			*newState = new MultiplayerSearchMenu();
			return Push;
		}

		if (appState->GetIsServer())
		{
			Game->StartAsServer();
			*newState = new PlayingHUD();
			return PushdownResult::Push;
		}
		ui->DrawButton(
			"Solo Game",
			Vector2(5, 23),
			[&]() {
				isSoloGameBtnPressed = true;
			},
			UIBase::WHITE,
			KeyCodes::NUM1 // Only for PS
		);

#ifdef _WIN32
		ui->DrawButton(
			"Create Lobby",
			Vector2(5, 33),
			[&, Subsystem]() {
				if (!isCreatingLobby)
				{
					Subsystem->CreateLobby();
					isCreatingLobby = true;
				}
			},
			UIBase::WHITE,
			KeyCodes::NUM1 // Only for PS
		);

		ui->DrawButton(
			"Search Lobby",
			Vector2(5, 43),
			[&]() {
				isSearchLobbyBtnPressed = true;
			},
			UIBase::WHITE,
			KeyCodes::NUM2 // Only for PS
		);

		ui->DrawButton(
			"Quit Game",
			Vector2(5, 53),
			[Game]() {
				Game->CloseGame = true;
			},
			UIBase::WHITE,
			KeyCodes::NUM2 // Only for PS
		);
#endif

		/** for devlop only */
		/*if (appState->GetIsServer())
		{
			appState->SetIsGameOver(false);
			Game->StartAsServer();
			*newState = new PlayingHUD();
			return PushdownResult::Push;
		}*/
		//if (appState->GetIsClient())
		//{
		//	appState->SetIsGameOver(false);
		//	Game->StartAsClient(127, 0, 0, 1);
		//	*newState = new PlayingHUD();
		//	return PushdownResult::Push;
		//}
		//ui->DrawButton(
		//	"Local: Play As Server",
		//	Vector2(5, 53),
		//	[&, Game]() {
		//		Game->SetLocalPlayerIndex(0);
		//		appState->SetIsServer(true);
		//		Game->isDevMode = true;
		//	},
		//	UIBase::WHITE,
		//	KeyCodes::NUM1 // Only for PS
		//);
		//ui->DrawButton(
		//	"Local: Play As Client",
		//	Vector2(5, 63),
		//	[&, Game]() {
		//		appState->SetIsClient(true);
		//	},
		//	UIBase::WHITE,
		//	KeyCodes::NUM1 // Only for PS
		//);
	}
	return PushdownResult::NoChange;
}

void MainMenu::ReceiveEvent(const EventType eventType) {
	switch (eventType)
	{
	case LOBBY_CREATED :
		isLobbyCreated = true;
		break;

	case LOBBY_CREATEFAILED :
		isCreatingLobby = false;
		break;

	default:
		break;
	}
}

/** Multiplayer Lobby Update */
PushdownState::PushdownResult MultiPlayerLobby::OnUpdate(float dt, PushdownState** newState)
{
	if (game && OnlineSubsystem)
	{
		OnlineSubsystemBase* Subsystem = (OnlineSubsystemBase*)OnlineSubsystem;
		NetworkedGame* Game = (NetworkedGame*)game;

		ui->DrawStringText("UserName: " + Subsystem->GetCurrentUserName(), Vector2(5, 13), UIBase::WHITE);
		ui->DrawStringText("______________________________________________________________________________________________", Vector2(5, 30), UIBase::BLACK);
		ui->DrawStringText("______________________________________________________________________________________________", Vector2(5, 65), UIBase::BLACK);

		CheckAndDisplayLobbyMembersList(Subsystem, Game);
		Game->SetLocalPlayerIndex(Subsystem->GetCurrentUserLobbyIndex());
		if (Subsystem->GetCurrentUserLobbyIndex() == 0)
		{
			appState->SetIsLobbyHolder(true);
		}

		if (isLeaveLobbyPressed)
		{
			isLeaveLobbyPressed = false;
			appState->SetIsLobbyHolder(false);
			EventEmitter::RemoveListner(this);
			return PushdownResult::Pop;
		}
		ui->DrawButton(
			"Leave Lobby",
			Vector2(47, 75),
			[Subsystem]() {
				Subsystem->LeaveLobby();
				EventEmitter::EmitEvent(LOBBY_LEAVE);
			},
			UIBase::WHITE,
			KeyCodes::L
		);
		if (appState->GetIsLobbyHolder())
		{
			if (appState->GetIsServer())
			{
				appState->SetIsGameOver(false);
				Game->StartAsServer();
				*newState = new PlayingHUD();
				return PushdownResult::Push;
			}
			ui->DrawButton(
				"Start Game",
				Vector2(5, 75),
				[&, Subsystem]() {
					Subsystem->SendGameRoundStartSignal();
					appState->SetIsServer(true);
				},
				UIBase::WHITE,
				KeyCodes::S // Only for PS
			);
		}
		else
		{
			ui->DrawStringText("You are the joiner!", Vector2(5, 23), UIBase::WHITE);
			ui->DrawStringText("Wait for holder start!", Vector2(5, 75), UIBase::WHITE);
			//ui->DrawButton(
			//	"Start Game",
			//	Vector2(5, 75),
			//	[]() {
			//		EventEmitter::EmitEvent(START_AS_CLIENT);
			//	},
			//	UIBase::WHITE,
			//	KeyCodes::S // Only for PS
			//);
			if (appState->GetIsClient())
			{
				appState->SetIsGameOver(false);
				Game->StartAsClient(GetIPnumByIndex(0), GetIPnumByIndex(1), GetIPnumByIndex(2), GetIPnumByIndex(3));
				*newState = new PlayingHUD();
				return PushdownResult::Push;
			}
		}
		
	}
	return PushdownResult::NoChange;
}

void MultiPlayerLobby::ReceiveEvent(const EventType eventType) {
	switch (eventType) {
	case LOBBY_GAMESTART:
		if (!appState->GetIsLobbyHolder())
		{
			appState->SetIsClient(true);
		}
		break;

	case LOBBY_LEAVE:
		isLeaveLobbyPressed = true;
		break;

	default :
		break;
	}
}

void MultiPlayerLobby::CheckAndDisplayLobbyMembersList(OnlineSubsystemBase* Subsystem, NetworkedGame* Game)
{
	for (int i = 0; i < Subsystem->GetNumCurrentLobbyMembers(); ++i)
	{
		if (i == 0) { ui->DrawStringText(("Holder"), Vector2(5, 37), UIBase::WHITE); }
		ui->DrawStringText(DisplayMemberLineByIndex(Subsystem, i), Vector2(15, 37 + i * 7), UIBase::WHITE);

		Game->SetPlayerNameByIndexInList(Subsystem->GetLobbyMemberNameByIndex(i), i);
	}
}

std::string MultiPlayerLobby::DisplayMemberLineByIndex(OnlineSubsystemBase* Subsystem, int Index)
{
	std::string MemberInfo = "      ";

	MemberInfo += Subsystem->GetLobbyMemberNameByIndex(Index);

	return MemberInfo;
}

char MultiPlayerLobby::GetIPnumByIndex(int index)
{
	// this need to be modified
	NetSystem_Steam* Steam = (NetSystem_Steam*)OnlineSubsystem;
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
	if (game && OnlineSubsystem)
	{
		OnlineSubsystemBase* Subsystem = (OnlineSubsystemBase*)OnlineSubsystem;

		ui->DrawStringText("UserName: " + Subsystem->GetCurrentUserName(), Vector2(5, 13), UIBase::WHITE);
		ui->DrawStringText("Available Lobbies :", Vector2(5, 23), UIBase::WHITE);
		ui->DrawStringText("______________________________________________________________________________________________", Vector2(5, 33), UIBase::BLACK);
		ui->DrawStringText("______________________________________________________________________________________________", Vector2(5, 68), UIBase::BLACK);

		if (isSearchingLobbies)
		{
			ui->DrawStringText("Searching For Lobbies...", Vector2(5, 42), UIBase::WHITE);
		}
		if (isLobbyJoined)
		{
			*newState = new MultiPlayerLobby();
			isLobbyJoined = false;
			return PushdownResult::Push;
		}
		if (isMainMenuPressed)
		{
			isMainMenuPressed = false;
			EventEmitter::RemoveListner(this);
			return PushdownResult::Pop;
		}

		CheckAndDisplaySearchResult(Subsystem);

		ui->DrawButton(
			"Main Menu",
			Vector2(48,75),
			[&]() {
				isMainMenuPressed = true;
			},
			UIBase::WHITE,
			KeyCodes::ESCAPE // Only for PS
			);

		ui->DrawButton(
			"Refresh Lobby List",
			Vector2(48, 13),
			[&, Subsystem]() {
				if (!isSearchingLobbies)
				{
					Subsystem->SearchLobbies();
					isSearchingLobbies = true;
				}
			},
			UIBase::WHITE,
			KeyCodes::NUM1 // Only for PS
			);

		ui->DrawButton(
			"<=",
			Vector2(5, 28),
			[&, Subsystem]() {
				ChangeCurrentSelectLobbyByAmount(Subsystem, -1);
			},
			UIBase::WHITE,
			KeyCodes::UP // Only for PS
		);

		ui->DrawButton(
			"=>",
			Vector2(48, 28),
			[&, Subsystem]() {
				ChangeCurrentSelectLobbyByAmount(Subsystem, 1);
			},
			UIBase::WHITE,
			KeyCodes::DOWN // Only for PS
		);

		if (isJoiningLobby)
		{
			ui->DrawStringText("Joining Select Lobby...", Vector2(5, 75), UIBase::WHITE);
		}
		else
		{
			ui->DrawButton(
				"Join Selected Lobby",
				Vector2(5, 75),
				[&, Subsystem]() {
					if (isCanJoinSelectLobby && !isJoiningLobby)
					{
						Subsystem->SelectLobbyByIndex(CurrentSelectLobby);
						Subsystem->JoinLobby();
						isJoiningLobby = true;
					}
				},
				UIBase::WHITE,
				KeyCodes::S // Only for PS
			);
		}
	}
	return PushdownResult::NoChange;
}

void MultiplayerSearchMenu::ReceiveEvent(const EventType eventType) {
	switch (eventType) {
		case LOBBY_SEARCHCOMPLETED:
			isSearchingLobbies = false;
			break;

		case LOBBY_JOINED:
			isJoiningLobby = false;
			isLobbyJoined = true;
			break;

		case LOBBY_JOINFAILED:
			isJoiningLobby = false;
			break;

		default : break;
	}
}

void MultiplayerSearchMenu::CheckAndDisplaySearchResult(OnlineSubsystemBase* Subsystem)
{
	int num = Subsystem->GetNumOfLobbyMatchList();

	if (num == 0)
	{
		ui->DrawStringText("No Lobbies Found :-(", Vector2(5, 42), UIBase::RED);
		isCanJoinSelectLobby = false;
		return;
	}
	if (num == -1)
	{
		isCanJoinSelectLobby = false;
		return;
	}

	isCanJoinSelectLobby = true;

	FirstDisplayedLobbyIndex = FirstDisplayedLobbyIndex > CurrentSelectLobby ? CurrentSelectLobby :
		(FirstDisplayedLobbyIndex + 3) < CurrentSelectLobby ? CurrentSelectLobby - 3 : FirstDisplayedLobbyIndex;
	for (int i = 0; i < 4; ++i)
	{
		int CurrentDisplayLobbyIndex = FirstDisplayedLobbyIndex + i;
		if (CurrentDisplayLobbyIndex == CurrentSelectLobby)
		{
			ui->DrawStringText("->", Vector2(5, 42 + i * 7), UIBase::RED);
		}

		if (CurrentDisplayLobbyIndex < num)
		{
			ui->DrawStringText(DisplayLobbyLine(Subsystem, CurrentDisplayLobbyIndex), Vector2(12, 42 + i * 7), UIBase::RED);
		}
	}
}

string MultiplayerSearchMenu::DisplayLobbyLine(OnlineSubsystemBase* Subsystem, int Index)
{
	string Message = std::to_string(Index);
	Message += "                  ";
	Message += Subsystem->GetLobbyOwnerNameByIndex(Index);
	Message += "                  ";
	Message += std::to_string(Subsystem->GetNumLobbyMembersByIndex(Index));
	return Message;
}

void MultiplayerSearchMenu::ChangeCurrentSelectLobbyByAmount(OnlineSubsystemBase* Subsystem, int num)
{
	int targetLobbyIndex = CurrentSelectLobby + num;
	if (targetLobbyIndex >= 0 && targetLobbyIndex < Subsystem->GetNumOfLobbyMatchList())
	{
		CurrentSelectLobby = targetLobbyIndex;
	}
}

/** PlaingHUD update */
PushdownState::PushdownResult PlayingHUD::OnUpdate(float dt, PushdownState** newState)
{
	if (game && OnlineSubsystem)
	{
#ifdef _WIN32
		OnlineSubsystemBase* Subsystem = (OnlineSubsystemBase*)OnlineSubsystem;
		NetworkedGame* Game = (NetworkedGame*)game;
#else
#endif

		CheckRoundTime(Game);

		/** Round Over Event */
		if (appState->GetIsGameOver())
		{
			if (appState->GetIsServer())
			{
				appState->SetIsServer(false);
				Game->DestroyServer();
			}
			if (appState->GetIsClient())
			{
				appState->SetIsClient(false);
				Game->DestroyClient();
			}
			ShowGameResult(Game);
			if (Window::GetKeyboard()->KeyPressed(KeyCodes::Type::RETURN))
			{
				EventEmitter::RemoveListner(this);
				return  PushdownResult::Pop;
			}
		}

		/** Game going HUD */
		if (!appState->GetIsGameOver())
		{
			ShowTimeLeft(Game);
			//ui->DrawStringText("Player    " + Game->GetPlayerNameByIndex(Game->GetLocalPlayerIndex()), Vector2(83, 30), UIBase::WHITE);
			ui->DrawStringText("Player    " + Subsystem->GetCurrentUserName(), Vector2(83, 25), UIBase::WHITE);
			ui->DrawStringText("Your Score:    " + std::to_string(Game->GetPlayerScoreByIndex(Game->GetLocalPlayerIndex())), Vector2(83, 35), UIBase::WHITE);
			ui->DrawStringText("Bullet Num:    " + std::to_string(Game->GetPlayerBulletNumByIndex(Game->GetLocalPlayerIndex())), Vector2(83, 45), UIBase::WHITE);
			ui->DrawStringText("PowerUp State: " + GetRoundPowerUpState(Game), Vector2(83, 55), UIBase::WHITE);		
			ui->DrawStringText("Hold TAB To", Vector2(83, 65), UIBase::WHITE);
			ui->DrawStringText("Show Score Table", Vector2(83, 70), UIBase::WHITE);

			if (Window::GetKeyboard()->KeyHeld(KeyCodes::Type::TAB))
			{
				ShowScoreTable(Game);
			}
		}

		if (appState->GetIsServer())
		{
			ui->DrawButton(
				"Round Over",
				Vector2(85, 85),
				[&, Game]() {
					Game->ServerSendRoundOverMsg();
					appState->SetIsGamePaused(false);
					EventEmitter::EmitEvent(EventType::ROUND_OVER);
				},
				UIBase::WHITE,
				KeyCodes::S, // Only for PS
				Vector2(150, 50)
			);
		}

		if (!appState->GetIsGameOver() && appState->GetIsGamePaused())
		{
			ui->DrawStringText("Round Paused !!!", Vector2(40, 50), UIBase::BLUE);
		}

		if (appState->GetIsSolo() && !appState->GetIsGameOver())
		{
			ui->DrawButton(
				appState->GetIsGamePaused() ? "Resume" : "Round Pause",
				Vector2(85, 80),
				[&, Game]() {
					appState->SetIsGamePaused(!appState->GetIsGamePaused());
				},
				UIBase::WHITE,
				KeyCodes::S, // Only for PS
				Vector2(150, 50)
			);
		}
	}
	return PushdownResult::NoChange;
}

void PlayingHUD::ReceiveEvent(const EventType eventType)
{
	switch (eventType) {
	case ROUND_OVER:
		appState->SetIsGameOver(true);
		break;

	default: break;
	}
}

void PlayingHUD::ShowScoreTable(TutorialGame* Game)
{
	//TODO dynamic cast game
	NetworkedGame* tempGame = dynamic_cast<NetworkedGame*> (Game);
	for (int i = 0; i < 4; ++i)
	{
		if (tempGame->GetPlayerScoreByIndex(i) != -1)
		{
			ui->DrawStringText("Player " + std::to_string(i + 1), Vector2(25, 30 + i * 7), UIBase::BLUE);
			ui->DrawStringText(tempGame->GetPlayerNameByIndex(i), Vector2(45, 30 + i * 7), UIBase::BLUE);
			ui->DrawStringText("Your Score: " + std::to_string(tempGame->GetPlayerScoreByIndex(i)), Vector2(65, 30 + i * 7), UIBase::BLUE);
		}
	}
}

void PlayingHUD::ShowTimeLeft(TutorialGame* Game)
{
	//TODO dynamic cast game
	NetworkedGame* tempGame = dynamic_cast<NetworkedGame*> (Game);

	ui->DrawStringText("Timeleft:", Vector2(83, 15), UIBase::WHITE);

	float timeLeft = tempGame->GetRoundTimeLimit() - tempGame->GetRoundTimer();
	int time = (int)timeLeft + 1;
	
	ui->DrawStringText(std::to_string(time), Vector2(90, 15), UIBase::WHITE);
}

void PlayingHUD::CheckRoundTime(TutorialGame* Game)
{
	//TODO dynamic cast game
	NetworkedGame* tempGame = dynamic_cast<NetworkedGame*> (Game);

	if (tempGame->GetRoundTimer() > tempGame->GetRoundTimeLimit())
	{
		EventEmitter::EmitEvent(EventType::ROUND_OVER);
	}
}

void PlayingHUD::ShowGameResult(TutorialGame* Game)
{
	//TODO dynamic cast game
	NetworkedGame* tempGame = dynamic_cast<NetworkedGame*> (Game);

	int LoaclPlayerScore = tempGame->GetPlayerScoreByIndex(tempGame->GetLocalPlayerIndex());
	int Rank = 1;
	for (int i = 0; i < 4; ++i)
	{
		if (tempGame->GetPlayerScoreByIndex(i) != -1)
		{
			ui->DrawStringText("Player " + std::to_string(i + 1), Vector2(25, 30 + i * 7), UIBase::BLUE);
			ui->DrawStringText(tempGame->GetPlayerNameByIndex(i), Vector2(45, 30 + i * 7), UIBase::BLUE);
			ui->DrawStringText("Score: " + std::to_string(tempGame->GetPlayerScoreByIndex(i)), Vector2(65, 30 + i * 7), UIBase::BLUE);
			if (LoaclPlayerScore < tempGame->GetPlayerScoreByIndex(i))
			{
				++Rank;
			}
		}
	}
	std::string RoundResult;
	switch (Rank)
	{
	case 1:
		RoundResult = std::string("You are the Winner ! ! !");
		break;

	case 2:
		RoundResult = std::string("Sorry! You are 2nd ! ! !");
		break;

	case 3:
		RoundResult = std::string("Sorry! You are 3rd ! ! !");
		break;

	case 4:
		RoundResult = std::string("Sorry! You are 4th ! ! !");
		break;

	default:
		break;
	}
	ui->DrawStringText(RoundResult, Vector2(40, 15), UIBase::RED);
	ui->DrawStringText("Press Enter To Continue!...", Vector2(25, 60), UIBase::YELLOW);
}

std::string PlayingHUD::GetRoundPowerUpState(TutorialGame* Game)
{
	//TODO dynamic cast game
	NetworkedGame* tempGame = dynamic_cast<NetworkedGame*> (Game);

	std::string state;
	switch (tempGame->GetCurrentPowerUpType())
	{
	case powerUpType::none:
		state = std::string("No powerup");
		break;

	case powerUpType::ice:
		state = std::string("Ice");
		break;

	case powerUpType::sand:
		state = std::string("Sand");
		break;

	case powerUpType::wind:
		state = std::string("Wind");
		break;

	default:
		break;
	}
	return state;
}

