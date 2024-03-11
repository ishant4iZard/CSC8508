#pragma once
#include "NetworkBase.h"
#include <stdint.h>
#include <thread>
#include <atomic>

namespace NCL {
	namespace CSC8503 {
		class GameObject;

		enum EClientState
		{	
			CLIENT_STATE_NONE = 0,
			CLIENT_STATE_DISCONNECTED = 1,
			CLIENT_STATE_CONNECTING = 2,
			CLIENT_STATE_CONNECTED = 3,
			CLIENT_STATE_MAX = 4,
		};

		class GameClient : public NetworkBase {
		public:
			GameClient();
			~GameClient();

			bool Connect(uint8_t a, uint8_t b, uint8_t c, uint8_t d, int portNum);

			void SendPacket(GamePacket&  payload);

			void UpdateClient(float dt);

		protected:	
			_ENetPeer*	netPeer;
			int CurrentConnetNetID;
				
			int PlayerIndex;

			EClientState clientState;

			float timeGap = 0.0f;

		public:
			inline int GetClientNetID() const { return CurrentConnetNetID; }
			inline EClientState GetClientState() const { return clientState; }
			void SetLocalPlayerIndex(int val) { PlayerIndex = val; }
		};
	}
}

