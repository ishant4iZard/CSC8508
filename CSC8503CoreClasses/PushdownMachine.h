#pragma once

#include <stack>

namespace NCL {
	namespace CSC8503 {
		class PushdownState;

		class PushdownMachine
		{
		public:
			PushdownMachine(PushdownState* initialState);
			~PushdownMachine();

			bool Update(float dt);

			void GetGame(void** game) { *game = this->game; }
			void SetGame(void* val) { this->game = val; }

			void SetOnlineSubsystem(void* val) { this->OnlineSubsystem = val; }

		protected:
			PushdownState* activeState;
			PushdownState* initialState;

			std::stack<PushdownState*> stateStack;

			void* game;
			void* OnlineSubsystem;
		};
	}
}

