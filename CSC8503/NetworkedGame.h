#pragma once
#include "TutorialGame.h"
#include "NetworkBase.h"
#include "PushdownState.h"

#include "GameClient.h"

namespace NCL {
	namespace CSC8503 {
		class GameServer;
		class GameClient;
		class NetworkPlayer;
		class PushdownMachine;
		class DeltaPacket;
		class FullPacket;
		class ClientPacket;

		class NetworkedGame : public TutorialGame, public PacketReceiver {
		public:
			NetworkedGame();
			~NetworkedGame();

			bool StartAsServer();
			bool StartAsClient(char a, char b, char c, char d);

			void UpdateGame(float dt) override;

			void SpawnPlayer();
			void SpawnProjectile(NetworkPlayer* owner, Vector3 firePos, Vector3 fireDir);
			void OnRep_SpawnProjectile(int PlayerNum, int objectID);

			void StartLevel();

			void ReceivePacket(int type, GamePacket* payload, int source) override;

			void OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b);

			/** Network public function */
			int GetConnectedClientsNum();
			int GetLocalPlayerNumber() const;
			int GetPlayerNumberFromNetID(const int NetID) const;
			int GetClientState();

		protected:
			void UpdateAsServer(float dt);
			void UpdateAsClient(float dt);

			void UpdatePlayerPositions(float dt);

			void BroadcastSnapshot(bool deltaFrame);
			void UpdateMinimumState();

			void ServerUpdatePlayersList();
			void CheckPlayerListAndSpawnPlayers();

			NetworkPlayer* AddNetworkPlayerToWorld(const Vector3& position, int playerNum);

			void findOSpointerWorldPosition(Vector3& position);

			/** function used to process network package */
			bool serverProcessClientPacket(ClientPacket* cp, int source);
			bool clientProcessFullPacket(FullPacket* fp);
			bool clientProcessDeltaPacket(DeltaPacket* dp);

			std::map<int, int> stateIDs;
			int GlobalStateID;

			GameServer* thisServer;
			GameClient* thisClient;
			float timeToNextPacket;
			int packetsToSnapshot;

			std::map<int, NetworkObject*> networkObjects;

			/*std::map<int, GameObject*> serverPlayers;
			GameObject* localPlayer;*/

			PushdownMachine* TestMenu;

			/** List for Networking Players*/
			std::vector<int> PlayersList;
			std::vector<NetworkPlayer*> ControledPlayersList;

			NetworkPlayer* LocalPlayer;

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
					if (StartAsServerDisplayTime > 0.0f)
					{
						Debug::Print("Server Existed! Start as Client please!", Vector2(15, 53), Debug::RED);
						StartAsServerDisplayTime -= dt;
					}

					NetworkedGame* thisGame = (NetworkedGame*)game;
					if (thisGame->IsClient())
					{
						switch (thisGame->GetClient()->GetClientState())
						{
						case EClientState::CLIENT_STATE_CONNECTING:
							Debug::Print("Client Is Serching!....", Vector2(15, 53), Debug::RED);
							break;
						case EClientState::CLIENT_STATE_DISCONNECTED:
							Debug::Print("Failed to connect server, please Press 1!", Vector2(10, 53), Debug::RED);
							break;
						case EClientState::CLIENT_STATE_CONNECTED:
							*newState = new PlayerMenu();
							return PushdownResult::Push;
							break;
						}
						return PushdownResult::NoChange;
					}
					if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM1))
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
					if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM2))
					{
						if (!thisGame->StartAsClient(127, 0, 0, 1))
						{

						}
					}
				}

				return PushdownResult::NoChange;
			}

		protected:
			float StartAsServerDisplayTime = 0.0f;
		};
	}
}

