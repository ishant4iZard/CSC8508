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

		protected:


		private:
			PushdownMachine* MenuMachine;

			NetSystem_Steam* steam;
		};

		class MainMenu : public PushdownState
		{
		public:
			PushdownResult OnUpdate(float dt, PushdownState** newState) override;
		protected:

		};
	}
}

