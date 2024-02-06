#include "steam.h"

using namespace NCL;
using namespace CSC8503;

void Net_Steam::CreateLobby()
{
	SteamAPICall_t hCreateLobby = SteamMatchmaking()->CreateLobby(ELobbyType::k_ELobbyTypePublic, 4);

	m_LobbyCreateCallResult.Set(hCreateLobby, this, &Net_Steam::On_LobbyCreated);
}

void Net_Steam::On_LobbyCreated(LobbyCreated_t* pResult, bool bIOFailure)
{
	
}
