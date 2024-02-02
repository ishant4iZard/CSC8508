#pragma once
#include "GameObject.h"
#include "NetworkBase.h"
#include "NetworkState.h"

namespace NCL::CSC8503 {
	class GameObject;

	struct PlayerFirePacket : public GamePacket {
		char	PlayerNum;
		int   NetObjectID;

		PlayerFirePacket() {
			type = Player_Fire;
			size = sizeof(PlayerFirePacket);
		}
	};

	struct ClientPacket : public GamePacket {
		int		lastID;
		Vector3 PointerPos;
		bool bIsFireBtnClicked = false;

		ClientPacket() {
			type = Received_State;
			size = sizeof(ClientPacket);
		}
	};

	struct PLayersListPacket : public GamePacket
	{
		char playerList[4];
		PLayersListPacket(std::vector<int>& serverPlayerList)
		{
			type = Message;
			size = sizeof(PLayersListPacket);

			for (int i = 0; i < 4; ++i)
			{
				playerList[i] = serverPlayerList[i] + 48;
			}
		}

		void GetPlayerList(std::vector<int>& clientPlayerList)
		{
			for (int i = 0; i < 4; ++i)
			{
				clientPlayerList[i] = playerList[i] - 48;
			}
		}
	};

	struct FullPacket : public GamePacket {
		int		objectID = -1;
		NetworkState fullState;

		FullPacket() {
			type = Full_State;
			size = sizeof(FullPacket) - sizeof(GamePacket);
		}
	};

	struct DeltaPacket : public GamePacket {
		int		fullID		= -1;
		int		objectID	= -1;
		char	pos[3];
		char	orientation[4];

		DeltaPacket() {
			type = Delta_State;
			size = sizeof(DeltaPacket) - sizeof(GamePacket);
		}
	};

	class NetworkObject		{
	public:
		NetworkObject(GameObject& o, int id);
		virtual ~NetworkObject();

		//Called by clients
		virtual bool ReadPacket(GamePacket& p);
		//Called by servers
		virtual bool WritePacket(GamePacket** p, bool deltaFrame, int stateID);

		void UpdateStateHistory(int minID);
		NetworkState& GetLatestNetworkState();

	protected:
		bool GetNetworkState(int frameID, NetworkState& state);

		virtual bool ReadDeltaPacket(DeltaPacket &p);
		virtual bool ReadFullPacket(FullPacket &p);

		virtual bool WriteDeltaPacket(GamePacket**p, int stateID);
		virtual bool WriteFullPacket(GamePacket**p);

		GameObject& object;

		NetworkState lastFullState;

		std::vector<NetworkState> stateHistory;

		int deltaErrors;
		int fullErrors;

		int networkID;
	};
}