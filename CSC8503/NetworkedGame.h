#pragma once
#include "TutorialGame.h"
#include "NetworkBase.h"
#include "PushdownState.h"
#include "Projectile.h"

#include "GameClient.h"
#include "ApplicationState.h"

#include "../CSC8503/UIBase.h"

#ifdef _WIN32
#include "../CSC8503/UIWindows.h"
#else //_ORBIS
#include "../CSC8503/UIPlaystation.h"
#endif

#include "AudioEngine.h"
#include "DebugHUD.h"

#include <chrono>
using namespace std::chrono;

namespace NCL {
	namespace CSC8503 {
		class GameServer;
		class GameClient;
		class NetworkPlayer;
		class PushdownMachine;
		class DeltaPacket;
		class FullPacket;
		class ClientPacket;
		class MenuSystem;

		class NetworkedGame : public TutorialGame, public PacketReceiver {
		public:
			NetworkedGame();
			~NetworkedGame();

			bool StartAsServer();
			bool StartAsClient(char a, char b, char c, char d);
			void DestroyServer();
			void DestroyClient();

			void UpdateGame(float dt) override;

			void SpawnPlayer();
			void SpawnProjectile(NetworkPlayer* owner, Vector3 firePos, Vector3 fireDir);
			void OnRep_SpawnProjectile(int PlayerNum, int objectID);
			void OnRep_DeactiveProjectile(int objectID);

			void StartLevel();
			void EndLevel();

			void ReceivePacket(int type, GamePacket* payload, int source) override;

			void OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b);

			std::vector<Projectile*> ProjectileList;

			
			/** Network public function */
			int GetConnectedClientsNum();
			int GetLocalPlayerNumber() const;
			int GetPlayerNumberFromNetID(const int NetID) const;
			int GetClientState();
			void SetPlayerNameByIndexInList(const std::string& Name, int Index);
			void ServerSendRoundOverMsg();
			// for develop mode
			bool isDevMode = false;

		protected:
			void UpdateAsServer(float dt);
			void UpdateAsClient(float dt);

			void HandleInputAsServer();
			void HandleInputAsClient();

			void UpdatePlayerState(float dt);
			void UpdateProjectiles(float dt);

			void BroadcastSnapshot(bool deltaFrame);
			void UpdateMinimumState();

			void ServerUpdatePlayersList();
			void ServerUpdateScoreList();
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
			ApplicationState* appState;

			std::map<int, NetworkObject*> networkObjects;

			/*std::map<int, GameObject*> serverPlayers;
			GameObject* localPlayer;*/

			//PushdownMachine* TestMenu;
			MenuSystem* Menu;

			/** List for Networking Players*/
			std::vector<int> PlayersList;
			std::vector<NetworkPlayer*> ControledPlayersList;
			std::vector<std::string> PlayersNameList;
			std::vector<int> PlayersScoreList;
			int localPlayerIndex;

			void SpawnAI();
			AiStatemachineObject* AddAiStateObjectToWorld(const Vector3& position);
			AiStatemachineObject* AIStateObject;

			bool ServerFired;
			bool ClientFired;

			NetworkPlayer* LocalPlayer;

			GameObject* projectileToChase;

			AudioEngine* audioEngine;
			int backGroundMusic = -1;
			int fireSFX = -1;
			bool UpdatePhysics = false;

			/** Round going on state */
			powerUpType CurrentPowerUpType;

			void PhysicsUpdate(float dt);
			void NonPhysicsUpdate(float dt);

			DebugHUD* debugHUD;
			bool isDebuHUDActive;

		public:
			inline bool IsServer() { return thisServer != nullptr ? true : false; }
			inline bool IsClient() { return thisClient != nullptr ? true : false; }
			inline GameServer* GetServer() const { return thisServer; }
			inline GameClient* GetClient() const { return thisClient; } 
			inline MenuSystem* GetMenuSystem() const { return Menu; }
			inline int GetLocalPlayerIndex() const { return localPlayerIndex; }
			inline void SetLocalPlayerIndex(int val) { localPlayerIndex = val; }
			inline std::string GetPlayerNameByIndex(int index) { return PlayersNameList[index]; }
			inline int GetPlayerScoreByIndex(int index) { return PlayersScoreList[index]; }
			inline float GetRoundTimer() const { return timer; }
			inline int GetRoundTimeLimit() const { return TIME_LIMIT; }
			inline powerUpType GetCurrentPowerUpType() const { return CurrentPowerUpType; }
		};


	}
}

