#pragma once


namespace NCL {
	namespace  CSC8503 {
		class PushdownMachine;
		class NetworkedGame;

		class MenuSystem
		{
		public:
			MenuSystem(NetworkedGame* Game);

		protected:


		private:
			PushdownMachine* MenuMachine;
		};
	}
}

