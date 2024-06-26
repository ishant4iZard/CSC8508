#pragma once
#include "TutorialGame.h"
#include "NetworkBase.h"
#include "PushdownState.h"
#include "Projectile.h"

#include "ThreadPool.h"

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

#define POWER_UP_INIT_NETID 2000;

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

			const int PROJECTILE_POOL_SIZE = 20;
			std::vector<Projectile*> ProjectileList;
			void InitializeProjectilePool(NetworkPlayer* player);

			/** Network public function */
			int GetConnectedClientsNum();
			int GetLocalPlayerNumber() const;
			int GetPlayerNumberFromNetID(const int NetID) const;
			int GetClientState();
			void SetPlayerNameByIndexInList(const std::string& Name, int Index);
			void ServerSendRoundOverMsg();
			void DeactiveNetObject(GameObject* TargetObject);

			// for develop mode
			bool isDevMode = false;

		protected:
			void UpdateAsServer(float dt);
			void UpdateAsClient(float dt);

			void HandleInputAsServer();
			void HandleInputAsClient();

			void UpdatePlayerState(float dt);
			void UpdateProjectiles(float dt);
			void UpdatePowerUpSpawnTimer(float dt);

			void BroadcastSnapshot(bool deltaFrame);
			void UpdateMinimumState();

			void ServerUpdatePlayersList();
			void ServerUpdateScoreList();
			void ServerUpdateBulletNumList();
			void CheckPlayerListAndSpawnPlayers();

			void UpdateAnimations(float dt);

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

			/** Network footprint */
			int TotalSizeOutgoingPacket = 0;
			int TotalSizeIncomingPakcet = 0;
			

			std::map<int, NetworkObject*> networkObjects;

			MenuSystem* Menu;

			/** List for Networking Players*/
			std::vector<int> PlayersList;
			std::vector<NetworkPlayer*> ControledPlayersList;
			std::vector<std::string> PlayersNameList;
			std::vector<int> PlayersScoreList;
			std::vector<int> PlayersBulletNumList;
			int localPlayerIndex;

			void SpawnAI();
			AiStatemachineObject* AddAiStateObjectToWorld(const Vector3& position);
			AiStatemachineObject* AIStateObject;
			int AIInitialID = 10;

			/** Power Up Spawn */
			void SpawnPowerUp(int NetID);
			int PowerUpSpawnNetID = POWER_UP_INIT_NETID;

			bool ServerFired;
			bool ClientFired;

			NetworkPlayer* LocalPlayer;

			GameObject* projectileToChase;

			AudioEngine* audioEngine;
			int backGroundMusic = -1;
			int fireSFX = -1;
			bool UpdatePhysics = false;


			powerUpType CurrentPowerUpType;
			ThreadPool* poolPTR;

			DebugHUD* debugHUD;

			std::mutex PhysicsMutex;
			std::mutex NonPhysicsMutex;

			std::optional<microseconds> physicsTimeCost;

		public:
			void NonPhysicsUpdate(float dt);
			void PhysicsUpdate(float dt);
			inline bool IsServer() { return thisServer != nullptr ? true : false; }
			inline bool IsClient() { return thisClient != nullptr ? true : false; }
			inline GameServer* GetServer() const { return thisServer; }
			inline GameClient* GetClient() const { return thisClient; } 
			inline MenuSystem* GetMenuSystem() const { return Menu; }
			inline int GetLocalPlayerIndex() const { return localPlayerIndex; }
			inline void SetLocalPlayerIndex(int val) { localPlayerIndex = val; }
			inline std::string GetPlayerNameByIndex(int index) { return PlayersNameList[index]; }
			inline int GetPlayerScoreByIndex(int index) { return PlayersScoreList[index]; }
			inline int GetPlayerBulletNumByIndex(int index) { return PlayersBulletNumList[index]; }
			inline float GetRoundTimer() const { return timer; }
			inline int GetRoundTimeLimit() const { return TIME_LIMIT; }
			inline powerUpType GetCurrentPowerUpType() const { return CurrentPowerUpType; }
			inline NetworkPlayer* GetLocallyControlPlayer() const { return LocalPlayer; }
			int GetLocalPlayerBulletNum() const;
			float GetOutgoingPacketSizePerSecond() const;
			float GetInComingPacketSizePerSecond() const;
		};
	}
}

