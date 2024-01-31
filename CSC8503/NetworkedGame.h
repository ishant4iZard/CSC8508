#pragma once
#include "TutorialGame.h"
#include "NetworkBase.h"
#include "PushdownState.h"

namespace NCL {
	namespace CSC8503 {
		class GameServer;
		class GameClient;
		class NetworkPlayer;
		class PushdownMachine;

		class NetworkedGame : public TutorialGame, public PacketReceiver {
		public:
			NetworkedGame();
			~NetworkedGame();

			void StartAsServer();
			void StartAsClient(char a, char b, char c, char d);

			void UpdateGame(float dt) override;

			void SpawnPlayer();

			void StartLevel();

			void ReceivePacket(int type, GamePacket* payload, int source) override;

			void OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b);

		protected:
			void UpdateAsServer(float dt);
			void UpdateAsClient(float dt);

			void BroadcastSnapshot(bool deltaFrame);
			void UpdateMinimumState();
			std::map<int, int> stateIDs;

			GameServer* thisServer;
			GameClient* thisClient;
			float timeToNextPacket;
			int packetsToSnapshot;

			std::vector<NetworkObject*> networkObjects;

			std::map<int, GameObject*> serverPlayers;
			GameObject* localPlayer;

			PushdownMachine* TestMenu;
		};

		class MainMenu : public PushdownState
		{
			PushdownResult OnUpdate(float dt, PushdownState** pushFunc) override
			{
				Debug::Print("Press 1 : Start As Server", Vector2(5, 23), Debug::YELLOW);
				Debug::Print("Press 2 : Start As Client", Vector2(5, 33), Debug::YELLOW);
				Debug::Print("Press Esc : Game Over", Vector2(5, 43), Debug::YELLOW);

				if (game)
				{
					NetworkedGame* thisGame = (NetworkedGame*)game;
					if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM1))
					{
						thisGame->StartAsServer();
					}
					if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM2))
					{
						thisGame->StartAsClient(127, 0, 0, 1);
					}
				}

				return PushdownResult::NoChange;
			}
		};
	}
}

