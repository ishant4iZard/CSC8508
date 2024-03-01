#include "MenuSystem.h"
#include "NetworkedGame.h"
#include "PushdownMachine.h"

#include "OnlineSubsystemBase.h"
#include "steam.h"

using namespace NCL;
using namespace CSC8503;

MenuSystem::MenuSystem(NetworkedGame* Game)
{
#ifdef _WIN32
	OnlineSubsystem = NetSystem_Steam::GetInstance();
#else

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
	OnlineSubsystem->SetIsOnlineSubsystemInitSuccess(result);
}

void MenuSystem::SetLocalIPv4Address(const std::string& IP)
{
	OnlineSubsystem->SetLocalIPv4Address(IP);
}  

/** Main Menu Update */
PushdownState::PushdownResult MainMenu::OnUpdate(float dt, PushdownState** newState)
{
	if (game && OnlineSubsystem)
	{
		OnlineSubsystemBase* Subsystem = (OnlineSubsystemBase*)OnlineSubsystem;

		if (!Subsystem->GetIsOnlineSubsystemInitSuccess())
		{
			ui->DrawStringText("NetSystem init failed! Please check your steam or playstation 5 network state and restart game!", Vector2(5, 40), UIBase::RED);
			return PushdownResult::NoChange;
		}
		else
		{
			Subsystem->SetCurrentUserName();
		}

		if (isLobbyCreated)
		{
			*newState = new MultiPlayerLobby();
			isCreatingLobby = false;
			isLobbyCreated = false;
			appState->SetIsLobbyHolder(true);
			return Push;
		}
		if (isSearchLobbyBtnPressed)
		{
			isSearchLobbyBtnPressed = false;
			*newState = new MultiplayerSearchMenu();
			return Push;
		}

		ui->DrawButton(
			"Create Lobby",
			Vector2(5, 23),
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
			Vector2(5, 33),
			[&]() {
				isSearchLobbyBtnPressed = true;
			},
			UIBase::WHITE,
			KeyCodes::NUM2 // Only for PS
		);

		/** for devlop only */

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

		CheckAndDisplayLobbyMembersList(Subsystem);
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
				EventEmitter::EmitEvent(LEAVE_CURRENT_LOBBY);
			},
			UIBase::WHITE,
			KeyCodes::L
		);
		if (appState->GetIsLobbyHolder())
		{
			if (appState->GetIsServer())
			{
				Game->StartAsServer();
				*newState = new PlayingHUD();
				return PushdownResult::Push;
			}
			ui->DrawButton(
				"Start Game",
				Vector2(5, 75),
				[]() {
					EventEmitter::EmitEvent(START_AS_SERVER);
				},
				UIBase::WHITE,
				KeyCodes::S // Only for PS
			);
		}
		else
		{
			ui->DrawStringText("You are the joiner!", Vector2(5, 23), UIBase::WHITE);
			ui->DrawButton(
				"Start Game",
				Vector2(5, 75),
				[]() {
					EventEmitter::EmitEvent(START_AS_CLIENT);
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

	case LEAVE_CURRENT_LOBBY:
		isLeaveLobbyPressed = true;
		break;

	default :
		break;
	}
}

void MultiPlayerLobby::CheckAndDisplayLobbyMembersList(OnlineSubsystemBase* Subsystem)
{
	for (int i = 0; i < Subsystem->GetNumCurrentLobbyMembers(); ++i)
	{
		if (i == 0) { ui->DrawStringText(("Holder"), Vector2(5, 37), UIBase::WHITE); }
		ui->DrawStringText(DisplayMemberLineByIndex(Subsystem, i), Vector2(15, 37 + i * 7), UIBase::WHITE);
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
	return PushdownResult::NoChange;
}


/*#pragma region Menu
		class PlayerMenu : public PushdownState
		{
		public :
			PlayerMenu() {
#ifdef _WIN32
				ui = UIWindows::GetInstance();
#else //_ORBIS
				ui = UIPlaystation::GetInstance();
#endif
			}


		protected :
			PushdownResult OnUpdate(float dt, PushdownState** newState) override
			{
				if (game)
				{
					NetworkedGame* thisGame = (NetworkedGame*)game;
					if (thisGame->IsServer())
					{
						int ClientsConnectedNum = thisGame->GetConnectedClientsNum();
						std::string num = std::to_string(ClientsConnectedNum);
						//Debug::Print("Connected Clients : " + num, Vector2(5, 3), Debug::YELLOW);
						ui->DrawStringText("Connected Clients : " + num, Vector2(5, 3), UIBase::YELLOW);
					}
					if (thisGame->IsClient())
					{

					}
				}
				return PushdownResult::NoChange;
			}
#pragma region UI
			UIBase* ui;
#pragma endregion
		};

		class MainMenu : public PushdownState
		{
		public :
			MainMenu() {
				appState = ApplicationState::GetInstance();
#ifdef _WIN32
				ui = UIWindows::GetInstance();
#else //_ORBIS
				ui = UIPlaystation::GetInstance();
#endif
			}

		protected :
			PushdownResult OnUpdate(float dt, PushdownState** newState) override
			{
				if (game)
				{
					//Debug::Print("Press 1 : Start As Server", Vector2(5, 23), Debug::YELLOW);
					//Debug::Print("Press 2 : Start As Client", Vector2(5, 33), Debug::YELLOW);
					//Debug::Print("Press Esc : Game Over", Vector2(5, 43), Debug::YELLOW);
					//ui->DrawStringText("Press 1 : Start As Server", Vector2(5, 23), UIBase::YELLOW);
					//ui->DrawStringText("Press 2 : Start As Client", Vector2(5, 33), UIBase::YELLOW);
					//ui->DrawStringText("Press Esc : Game Over", Vector2(5, 43), UIBase::YELLOW);

					if (StartAsServerDisplayTime > 0.0f)
					{
						//Debug::Print("Server Existed! Start as Client please!", Vector2(15, 53), Debug::RED);
						ui->DrawStringText("Server Existed! Start as Client please!", Vector2(15, 53), UIBase::RED);
						StartAsServerDisplayTime -= dt;
					}

					NetworkedGame* thisGame = (NetworkedGame*)game;
					if (thisGame->IsClient())
					{
						switch (thisGame->GetClient()->GetClientState())
						{
						case EClientState::CLIENT_STATE_CONNECTING:
							//Debug::Print("Client Is Serching!....", Vector2(15, 53), Debug::RED);
							ui->DrawStringText("Client Is Searching!....", Vector2(15, 53), UIBase::RED);
							break;
						case EClientState::CLIENT_STATE_DISCONNECTED:
							//Debug::Print("Failed to connect server, please Press 1!", Vector2(10, 53), Debug::RED);
							ui->DrawStringText("Failed to connect server, please Press 1!", Vector2(10, 53), UIBase::RED);
							break;
						case EClientState::CLIENT_STATE_CONNECTED:
							*newState = new PlayerMenu();
							return PushdownResult::Push;
							break;
						}
						return PushdownResult::NoChange;
					}

					// React based on app state_____________________________________________________________
					if (appState->GetIsServer())
					{
						if (thisGame->StartAsServer())
						{
							*newState = new PlayerMenu();
							return PushdownResult::Push;
						}
						else
						{
							StartAsServerDisplayTime = 3.0f;
						}
					}
					if (appState->GetIsClient())
					{
						if (!thisGame->StartAsClient(127, 0, 0, 1))
						{

						}
					}

					// UI Menu______________________________________________________________________________
					// To do : Find center location based on text size

					ui->DrawStringText("Main Menu", Vector2(45, 30), UIBase::YELLOW);

					ui->DrawButton(
						"Start As Server",
						Vector2(35, 35),
						[]() {
							ApplicationState* appState = ApplicationState::GetInstance();
							appState->SetIsServer(true);
						},
						UIBase::WHITE,
						KeyCodes::NUM1 // Only for PS
						);

					ui->DrawButton(
						"Start As Client",
						Vector2(35, 45),
						[]() {
							ApplicationState* appState = ApplicationState::GetInstance();
							appState->SetIsClient(true);
						},
						UIBase::WHITE,
						KeyCodes::NUM2 // Only for PS
						);

				}

				return PushdownResult::NoChange;
			}

		protected:
			float StartAsServerDisplayTime = 0.0f;
#pragma region UI
			UIBase* ui;
#pragma endregion
			ApplicationState* appState;
		};
#pragma endregion
*/
//#pragma region Menu
//		class PlayerMenu : public PushdownState
//		{
//			PushdownResult OnUpdate(float dt, PushdownState** newState) override
//			{
//				if (game)
//				{
//					NetworkedGame* thisGame = (NetworkedGame*)game;
//					if (thisGame->IsServer())
//					{
//						int ClientsConnectedNum = thisGame->GetConnectedClientsNum();
//						std::string num = std::to_string(ClientsConnectedNum);
//						Debug::Print("Connected Clients : " + num, Vector2(5, 3), Debug::YELLOW);
//					}
//					if (thisGame->IsClient())
//					{
//
//					}
//				}
//				return PushdownResult::NoChange;
//			}
//		};
//
//		class MainMenu : public PushdownState
//		{
//			PushdownResult OnUpdate(float dt, PushdownState** newState) override
//			{
//				if (game)
//				{
//					Debug::Print("Press 1 : Start As Server", Vector2(5, 23), Debug::YELLOW);
//					Debug::Print("Press 2 : Start As Client", Vector2(5, 33), Debug::YELLOW);
//					Debug::Print("Press Esc : Game Over", Vector2(5, 43), Debug::YELLOW);
//					if (StartAsServerDisplayTime > 0.0f)
//					{
//						Debug::Print("Server Existed! Start as Client please!", Vector2(15, 53), Debug::RED);
//						StartAsServerDisplayTime -= dt;
//					}
//
//					NetworkedGame* thisGame = (NetworkedGame*)game;
//					if (thisGame->IsClient())
//					{
//						switch (thisGame->GetClient()->GetClientState())
//						{
//						case EClientState::CLIENT_STATE_CONNECTING:
//							Debug::Print("Client Is Serching!....", Vector2(15, 53), Debug::RED);
//							break;
//						case EClientState::CLIENT_STATE_DISCONNECTED:
//							Debug::Print("Failed to connect server, please Press 1!", Vector2(10, 53), Debug::RED);
//							break;
//						case EClientState::CLIENT_STATE_CONNECTED:
//							*newState = new PlayerMenu();
//							return PushdownResult::Push;
//							break;
//						}
//						return PushdownResult::NoChange;
//					}
//					if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM1))
//					{
//						if (thisGame->StartAsServer())
//						{
//							*newState = new PlayerMenu();
//							return PushdownResult::Push;
//						}
//						else
//						{
//							StartAsServerDisplayTime = 3.0f;
//						}
//					}
//					if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM2))
//					{
//						if (!thisGame->StartAsClient(127, 0, 0, 1))
//						{
//
//						}
//					}
//				}
//
//				return PushdownResult::NoChange;
//			}
//
//		protected:
//			float StartAsServerDisplayTime = 0.0f;
//		};
//#pragma endregion