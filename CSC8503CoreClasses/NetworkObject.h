#pragma once
#include "GameObject.h"
#include "NetworkBase.h"
#include "NetworkState.h"

namespace NCL::CSC8503 {
	class GameObject;

	struct ServerRoundOverPacket : public GamePacket {
		bool isRoundOver = true;

		ServerRoundOverPacket() {
			type = Round_Over;
			size = sizeof(ServerRoundOverPacket);
		}
	};

	struct ClientHelloPacket : public GamePacket {
		char PlayerListIndex;
		char PeerID;

		ClientHelloPacket() {
			type = Client_Hello;
			size = sizeof(ClientHelloPacket);
		}
	};

	struct DeactivateProjectilePacket : public GamePacket {
		int   NetObjectID;

		DeactivateProjectilePacket() {
			type = Projectile_Deactivate;
			size = sizeof(DeactivateProjectilePacket);
		}
	};

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
		float timer;
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

	struct PlayersScorePacket : public GamePacket
	{
		int ScoreList[4];
		char PowerUpState;

		PlayersScorePacket(std::vector<int>& ServerScoreList)
		{
			type = Player_Score;
			size = sizeof(PlayersScorePacket);

			for (int i = 0; i < 4; ++i)
			{
				ScoreList[i] = ServerScoreList[i];
			}
		}

		void GetPlayerScore(std::vector<int>& ClientScoreList)
		{
			for (int i = 0; i < 4; ++i)
			{
				ClientScoreList[i] = ScoreList[i];
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

		GameObject* GetGameObject() const { return &object; }
		inline int GetNetworkID() const { return networkID; }

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