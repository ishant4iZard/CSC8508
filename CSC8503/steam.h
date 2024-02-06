#pragma once

#include <steam_api.h>

namespace NCL {
	namespace  CSC8503 {

		class NetSystem_Steam
		{
		public:
			void CreateLobby();

			void SearchLobbies();

			virtual void On_LobbyCreated(LobbyCreated_t* pResult, bool bIOFailure);
			virtual void On_LobbyMatchList(LobbyMatchList_t* pLobbyMatchList, bool bIOFailure);

		protected:

		private:
			CCallResult<NetSystem_Steam, LobbyCreated_t> m_LobbyCreateCallResult;
			CCallResult<NetSystem_Steam, LobbyMatchList_t> m_LobbyMatchListCallResult;
		};
	}
}