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

			bool SendSinglePacket(GamePacket& packet, int clientNum);

			bool GetNetPeer(int peerNum, int& peerID);

			virtual void UpdateServer();

		protected:
			void clearPeerArray();
			void AddPeer(int val);
			void DeletPeer(int val);

			void DebugNetPeer();

			int			port;
			int			clientMax;
			int			clientCount;
			int*        netPeers;
			GameWorld*	gameWorld;

			int incomingDataRate;
			int outgoingDataRate;
		};
	}
}
