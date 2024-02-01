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
			bool StartAsClient(char a, char b, char c, char d);

			void UpdateGame(float dt) override;

			void SpawnPlayer();

			void StartLevel();

			void ReceivePacket(int type, GamePacket* payload, int source) override;

			void OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b);

			int GetConnectedClientsNum();

		protected:
			void UpdateAsServer(float dt);
			void UpdateAsClient(float dt);

			void BroadcastSnapshot(bool deltaFrame);
			void UpdateMinimumState();

			void ServerUpdatePlayersList();

			std::map<int, int> stateIDs;

			GameServer* thisServer;
			GameClient* thisClient;
			float timeToNextPacket;
			int packetsToSnapshot;

			std::vector<NetworkObject*> networkObjects;

			std::map<int, GameObject*> serverPlayers;
			GameObject* localPlayer;

			PushdownMachine* TestMenu;

			/** List for Networking Players*/
			std::vector<int> PlayersList;
			std::vector<NetworkPlayer*> ControledPlayersList;

		public:
			inline bool IsServer() { return thisServer != nullptr ? true : false; }
			inline bool IsClient() { return thisClient != nullptr ? true : false; }
			inline GameServer* GetServer() const { return thisServer; }
			inline GameClient* GetClient() const { return thisClient; }
		};

		class PlayerMenu : public PushdownState
		{
			PushdownResult OnUpdate(float dt, PushdownState** newState) override
			{
				if (game)
				{
					NetworkedGame* thisGame = (NetworkedGame*)game;
					if (thisGame->IsServer())
					{
						int ClientsConnectedNum = thisGame->GetConnectedClientsNum();
						std::string num = std::to_string(ClientsConnectedNum);
						Debug::Print("Connected Clients : " + num, Vector2(5, 3), Debug::YELLOW);
					}
					if (thisGame->IsClient())
					{

					}
				}
				return PushdownResult::NoChange;
			}
		};

		class MainMenu : public PushdownState
		{
			PushdownResult OnUpdate(float dt, PushdownState** newState) override
			{
				if (game)
				{
					Debug::Print("Press 1 : Start As Server", Vector2(5, 23), Debug::YELLOW);
					Debug::Print("Press 2 : Start As Client", Vector2(5, 33), Debug::YELLOW);
					Debug::Print("Press Esc : Game Over", Vector2(5, 43), Debug::YELLOW);

					NetworkedGame* thisGame = (NetworkedGame*)game;
					if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM1))
					{
						thisGame->StartAsServer();
						*newState = new PlayerMenu();
						return PushdownResult::Push;
					}
					if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM2))
					{
						if (thisGame->StartAsClient(127, 0, 0, 1))
						{
							*newState = new PlayerMenu();
							return PushdownResult::Push;
						}
					}
				}

				return PushdownResult::NoChange;
			}
		};
	}
}

