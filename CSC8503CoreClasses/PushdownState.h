#pragma once

namespace NCL {
	namespace CSC8503 {
		//class NetworkedGame;
		class PushdownState
		{
		public:
			enum PushdownResult {
				Push, Pop, NoChange
			};
			PushdownState()  {
			}
			virtual ~PushdownState() {}

			virtual PushdownResult OnUpdate(float dt, PushdownState** newState) = 0;
			virtual void OnAwake() {}
			virtual void OnSleep() {}

			void GetGame(void** game) { *game = this->game; }
			void SetGame(void* val) { this->game = val; }

			void SetOnlineSubsystem(void* val) { this->OnlineSubsystem = val; }

		protected:
			void* game = nullptr;
			void* OnlineSubsystem = nullptr;
		};
	}
}