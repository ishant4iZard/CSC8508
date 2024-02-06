#pragma once

#include <steam_api.h>

namespace NCL {
	namespace  CSC8503 {

		class Net_Steam
		{
		public:
			void CreateLobby();

			virtual void On_LobbyCreated(LobbyCreated_t* pResult, bool bIOFailure);

		protected:

		private:
			CCallResult<Net_Steam, LobbyCreated_t> m_LobbyCreateCallResult;

		};
	}
}