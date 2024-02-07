#pragma once

#include <steam_api.h>
#include <vector>
#include <string>

using std::vector;
using std::string;

namespace NCL {
	namespace  CSC8503 {

		class NetSystem_Steam
		{
		public:
			enum ELobbyDataType {
				EGameName = 0,
				ETypeMax
			};

			vector<string> LobbyDataKey = {
				"GameName"
			};

			enum EUserState {
				EAvailable = 0,
				ELobbyHolder,
				ESearching,
				ELobbyJoiner,
				EStateMax
			};

			NetSystem_Steam() : 
				m_LobbyChatUpdateCallback(this, &NetSystem_Steam::On_LobbyChatUpdate),
				m_LobbyDataUpdateCallback(this, &NetSystem_Steam::On_LobbyDataUpdate)
			{
				CurrentUserName = SteamFriends()->GetPersonaName();
			}

		public:
			void CreateLobby();

			void SearchLobbies();

			void SelectLobbyByIndex(int Index);
			CSteamID GetLobbyIDByIndex(int Index);

			void JoinLobby();

			bool SetLobbyData(ELobbyDataType DataType, string Value);

			string GetLobbyOwnerNameByLobbyID(CSteamID LobbyID) const;

			virtual void On_LobbyCreated(LobbyCreated_t* pResult, bool bIOFailure);
			virtual void On_LobbyMatchList(LobbyMatchList_t* pLobbyMatchList, bool bIOFailure);
			virtual void On_LobbyJoined(LobbyEnter_t* pResult, bool bIOFailure);
			virtual void On_LobbyChatUpdate(LobbyChatUpdate_t* pCallback);
			virtual void On_LobbyDataUpdate(LobbyDataUpdate_t* pCallback);

			/** public get function */
			inline EUserState GetUserCurrentState() const { return CurrentUserState; }
			inline string GetCurrentUserName() const { return CurrentUserName; }
			inline int32 GetNumOfLobbyMatchList() const { return numLobbyMatchList; }

		protected:
			string GameName = "ProjetT";

		private:
			CCallResult<NetSystem_Steam, LobbyCreated_t> m_LobbyCreateCallResult;
			CCallResult<NetSystem_Steam, LobbyMatchList_t> m_LobbyMatchListCallResult;
			CCallResult<NetSystem_Steam, LobbyEnter_t> m_JoinLobbyCallResult;

			CCallback<NetSystem_Steam, LobbyChatUpdate_t> m_LobbyChatUpdateCallback;
			CCallback<NetSystem_Steam, LobbyDataUpdate_t> m_LobbyDataUpdateCallback;

			string CurrentUserName;

			/** the steam ID of the lobby which you just create or you just choose */
			uint64 steamIDLobby = 0;

			/** the states you are in */
			EUserState CurrentUserState = EUserState::EAvailable;

			/**  */
			int32 numLobbyMatchList = -1;
		};
	}
}