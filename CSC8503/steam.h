#pragma once

#include "OnlineSubsystemBase.h"
#include <steam_api.h>
#include <vector>
#include <string>

using std::vector;
using std::string;

namespace NCL {
	namespace  CSC8503 {

		class NetSystem_Steam : public OnlineSubsystemBase
		{
		public:
			static NetSystem_Steam* GetInstance();

			enum ELobbyDataType {
				EGameName = 0,
				EOwnerName,
				ELocalIPv4Address,
				ETypeMax
			};

			vector<string> LobbyDataKey = {
				"GameName",
				"OwnerName",
				"LocalIPv4Address"
			};

			NetSystem_Steam() : 
				m_LobbyChatUpdateCallback(this, &NetSystem_Steam::On_LobbyChatUpdate),
				m_LobbyDataUpdateCallback(this, &NetSystem_Steam::On_LobbyDataUpdate)
			{ }

		public:
			void CreateLobby() override;

			void SearchLobbies() override;

			void SelectLobbyByIndex(int Index) override;

			void JoinLobby() override;

			void LeaveLobby() override;

			bool SetLobbyData(ELobbyDataType DataType, string Value);

			string GetLobbyOwnerNameByIndex(int Index) const override;

			int GetNumCurrentLobbyMembers() const override;

			int GetNumLobbyMembersByIndex(int Index) const override;

			string GetLobbyMemberNameByIndex(int Index) const override;

			int GetCurrentUserLobbyIndex() const override;

			void SetCurrentUserName() override;

			void On_LobbyCreated(LobbyCreated_t* pResult, bool bIOFailure);
			void On_LobbyMatchList(LobbyMatchList_t* pLobbyMatchList, bool bIOFailure);
			void On_LobbyJoined(LobbyEnter_t* pResult, bool bIOFailure);
			void On_LobbyChatUpdate(LobbyChatUpdate_t* pCallback);
			void On_LobbyDataUpdate(LobbyDataUpdate_t* pCallback);

			void SetLocalIPv4Address(const string& IPAddress) override { LocalIPv4Address = IPAddress; }

			/** public get function */
			inline string GetLobbyHolderIPv4Address() const { return LobbyHolderIPv4Address; }

		protected:
			string GameName = "ProjetT";
			string LocalIPv4Address;
			string LobbyHolderIPv4Address;

			void OnCreateLobbySuccess() override;
			void OnCreateLobbyFailed() override;
			void OnLobbyMatchList() override;
			void OnJoinLobbySuccess() override;
			void OnJoinLobbyFailed() override;

		private:
			static NetSystem_Steam* steam;

			CCallResult<NetSystem_Steam, LobbyCreated_t> m_LobbyCreateCallResult;
			CCallResult<NetSystem_Steam, LobbyMatchList_t> m_LobbyMatchListCallResult;
			CCallResult<NetSystem_Steam, LobbyEnter_t> m_JoinLobbyCallResult;

			CCallback<NetSystem_Steam, LobbyChatUpdate_t> m_LobbyChatUpdateCallback;
			CCallback<NetSystem_Steam, LobbyDataUpdate_t> m_LobbyDataUpdateCallback;

			/** the steam ID of the lobby which you just create or you just choose */
			uint64 steamIDLobby = 0;
		};
	}
}