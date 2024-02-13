#pragma once
#include "PushdownState.h"

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

		class MultiplayerSearchMenu : public PushdownState
		{
		public:
			PushdownResult OnUpdate(float dt, PushdownState** newState) override;

		protected:
			void DisplaySearchResult(NetSystem_Steam* Steam);
			std::string DisplayLobbyLine(NetSystem_Steam* Steam, int Index);

			int CurrentSelectLobby = 0;
		};

		class MultiPlayerLobby : public PushdownState
		{
		public:
			PushdownResult OnUpdate(float dt, PushdownState** newState) override;

		protected:
			char GetIPnumByIndex(int index);
		};

		class MainMenu : public PushdownState
		{
		public:
			PushdownResult OnUpdate(float dt, PushdownState** newState) override;
		protected:

		};

		class PlayingHUD : public PushdownState
		{
		public:
			PushdownResult OnUpdate(float dt, PushdownState** newState) override;
		};
	}
}

