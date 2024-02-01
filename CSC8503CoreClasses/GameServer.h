#pragma once
#include "NetworkBase.h"

namespace NCL {
	namespace CSC8503 {
		class GameWorld;
		class GameServer : public NetworkBase {
		public:
			GameServer(int onPort, int maxClients);
			~GameServer();

			bool Initialise();
			void Shutdown();

			void SetGameWorld(GameWorld &g);

			bool SendGlobalPacket(int msgID);
			bool SendGlobalPacket(GamePacket& packet);

			virtual void UpdateServer();

		protected:
			void AddConnectClient(int peerID);
			void DeleteClient(int peerID);
			void ClearClientsArray();

		protected:
			int			port;
			int			clientMax;
			int			clientCount;
			GameWorld*	gameWorld;

			int incomingDataRate;
			int outgoingDataRate;

			int* Clients;

		public:
			inline int GetConnectedClientsNum() const { return clientCount; }

			/** -1 for no this Client */
			int GetClientNetID(int Index) const;
		};
	}
}
