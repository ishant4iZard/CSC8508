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

			NetSystem_Steam();

		public:
			void CreateLobby();

			void SearchLobbies();

			void JoinLobby();

			bool SetLobbyData(ELobbyDataType DataType, string Value);

			virtual void On_LobbyCreated(LobbyCreated_t* pResult, bool bIOFailure);
			virtual void On_LobbyMatchList(LobbyMatchList_t* pLobbyMatchList, bool bIOFailure);
			virtual void On_LobbyJoined(LobbyEnter_t* pResult, bool bIOFailure);
		protected:
			string GameName = "ProjetT";

		private:
			CCallResult<NetSystem_Steam, LobbyCreated_t> m_LobbyCreateCallResult;
			CCallResult<NetSystem_Steam, LobbyMatchList_t> m_LobbyMatchListCallResult;
			CCallResult<NetSystem_Steam, LobbyEnter_t> m_JoinLobbyCallResult;

			//CCallback<NetSystem_Steam, LobbyChatUpdate_t> m_LobbyChatUpdateCallback;

			uint64 steamIDLobby = 0;
		};
	}
}