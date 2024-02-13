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

			NetSystem_Steam* steam;
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
				emitter = EventEmitter::GetInstance();
				emitter->RegisterForEvent(REFRESH_LOBBY, this);
				emitter->RegisterForEvent(JOIN_CURRENT_LOBBY, this);
			}

			PushdownResult OnUpdate(float dt, PushdownState** newState) override;
			void ReceiveEvent(const EventType eventType) override;

		protected:
			void DisplaySearchResult(NetSystem_Steam* Steam);
			std::string DisplayLobbyLine(NetSystem_Steam* Steam, int Index);

			int CurrentSelectLobby = 0;
			UIBase* ui;
			EventEmitter* emitter;

			// Local states
			bool shouldRefreshLobbyList = false;
			bool shouldJoinSelectedLobby = false;
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

				emitter = EventEmitter::GetInstance();
				emitter->RegisterForEvent(START_AS_SERVER, this);
				emitter->RegisterForEvent(START_AS_CLIENT, this);
			}
			PushdownResult OnUpdate(float dt, PushdownState** newState) override;
			void ReceiveEvent(const EventType eventType) override;

		protected:
			char GetIPnumByIndex(int index);
			UIBase* ui;
			EventEmitter* emitter;
			ApplicationState* appState;
		};

		class MainMenu : public PushdownState, public EventListener
		{
		public:
			MainMenu() {
#ifdef _WIN32
				ui = UIWindows::GetInstance();
#else //_ORBIS
				ui = UIPlaystation::GetInstance();
#endif
				emitter = EventEmitter::GetInstance();
				emitter->RegisterForEvent(JOIN_LOBBY, this);
				emitter->RegisterForEvent(CREATE_LOBBY, this);
			}

			PushdownResult OnUpdate(float dt, PushdownState** newState) override;
			void ReceiveEvent(const EventType eventType) override;

		protected:
			UIBase* ui;
			EventEmitter* emitter;

			// Local States 
			bool isCreatingLobby = false;
			bool isJoiningLobby = false;
		};

		class PlayingHUD : public PushdownState
		{
		public:
			PushdownResult OnUpdate(float dt, PushdownState** newState) override;
		};
	}
}

