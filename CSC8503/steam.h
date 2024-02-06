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

			void CreateLobby();

			void SearchLobbies();

			bool SetLobbyData(ELobbyDataType DataType, string Value);

			virtual void On_LobbyCreated(LobbyCreated_t* pResult, bool bIOFailure);
			virtual void On_LobbyMatchList(LobbyMatchList_t* pLobbyMatchList, bool bIOFailure);

		protected:
			string GameName = "ProjetT";

		private:
			CCallResult<NetSystem_Steam, LobbyCreated_t> m_LobbyCreateCallResult;
			CCallResult<NetSystem_Steam, LobbyMatchList_t> m_LobbyMatchListCallResult;

			uint64 steamIDLobby = 0;
		};
	}
}