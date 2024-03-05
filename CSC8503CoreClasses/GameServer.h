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

			void SetClientList(int index, int ID);
			void AddConnectClient(int peerID);

		protected:
			void DeleteClient(int peerID);
			void ClearClientsArray();

		protected:
			int			port;
			int			clientMax;
			int			clientCount;
			GameWorld*	gameWorld;

			bool isValid;

			int incomingDataRate;
			int outgoingDataRate;

			int* Clients;

		public:
			inline int GetConnectedClientsNum() const { return clientCount; }
			inline bool IsValid() const { return isValid; }
			/** -1 for no this Client */
			int GetClientNetID(int Index) const;
		};
	}
}
