#pragma once
#include "PushdownState.h"
#include "ApplicationState.h"

#include "../CSC8503/UIBase.h"
#ifdef _WIN32
#include "../CSC8503/UIWindows.h"
#else //_ORBIS
#include "../CSC8503/UIPlaystation.h"
#endif
#include "Event.h"

namespace NCL {
	namespace  CSC8503 {
		class PushdownMachine;
		class NetworkedGame;
		class OnlineSubsystemBase;
		class NetSystem_Steam;

		class MenuSystem
		{
		public:
			MenuSystem(NetworkedGame* Game);
			~MenuSystem();

			void Update(float dt);

			void SetLocalIPv4Address(const std::string& IP);

		protected:


		private:
			PushdownMachine* MenuMachine;

			OnlineSubsystemBase* OnlineSubsystem;
		};

		/** Main Menu */
		class MainMenu : public PushdownState, public EventListener
		{
		public:
			MainMenu() {
#ifdef _WIN32
				ui = UIWindows::GetInstance();
#else //_ORBIS
				ui = UIPlaystation::GetInstance();
#endif
				appState = ApplicationState::GetInstance();

				EventEmitter::RegisterForEvent(LOBBY_CREATED, this);
				EventEmitter::RegisterForEvent(LOBBY_CREATEFAILED, this);
			}

			PushdownResult OnUpdate(float dt, PushdownState** newState) override;
			void ReceiveEvent(const EventType eventType) override;

		protected:
			UIBase* ui;
			ApplicationState* appState;

			// Local States 
			bool isCreatingLobby = false;
			bool isLobbyCreated = false;
			bool isSearchLobbyBtnPressed = false;
			bool isJoiningLobby = false;
		};

		class MultiplayerSearchMenu : public PushdownState, public EventListener
		{
		public:
			MultiplayerSearchMenu() {
#ifdef _WIN32
				ui = UIWindows::GetInstance();
#else //_ORBIS
				ui = UIPlaystation::GetInstance();
#endif
				EventEmitter::RegisterForEvent(LOBBY_SEARCHCOMPLETED, this);
				EventEmitter::RegisterForEvent(LOBBY_JOINED, this);
				EventEmitter::RegisterForEvent(LOBBY_JOINFAILED, this);
			}

			PushdownResult OnUpdate(float dt, PushdownState** newState) override;
			void ReceiveEvent(const EventType eventType) override;

		protected:
			void CheckAndDisplaySearchResult(OnlineSubsystemBase* Subsystem);
			std::string DisplayLobbyLine(OnlineSubsystemBase* Subsystem, int Index);

			int CurrentSelectLobby = 0;
			void ChangeCurrentSelectLobbyByAmount(OnlineSubsystemBase* Subsystem, int num);

			int FirstDisplayedLobbyIndex = 0;

			UIBase* ui;

			// Local states
			bool isSearchingLobbies = false;
			bool isCanJoinSelectLobby = false;
			bool isJoiningLobby = false;
			bool isLobbyJoined = false;
		};

		class MultiPlayerLobby : public PushdownState, public EventListener
		{
		public:
			MultiPlayerLobby() {
#ifdef _WIN32
				ui = UIWindows::GetInstance();
#else //_ORBIS
				ui = UIPlaystation::GetInstance();
#endif
				appState = ApplicationState::GetInstance();

				EventEmitter::RegisterForEvent(START_AS_SERVER, this);
				EventEmitter::RegisterForEvent(START_AS_CLIENT, this);
			}
			PushdownResult OnUpdate(float dt, PushdownState** newState) override;
			void ReceiveEvent(const EventType eventType) override;

		protected:
			void CheckAndDisplayLobbyMembersList(OnlineSubsystemBase* Subsystem);
			std::string DisplayMemberLineByIndex(OnlineSubsystemBase* Subsystem, int Index);

			char GetIPnumByIndex(int index);

			UIBase* ui;
			ApplicationState* appState;
		};

		class PlayingHUD : public PushdownState
		{
		public:
			PushdownResult OnUpdate(float dt, PushdownState** newState) override;
		};
	}
}
