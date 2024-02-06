#include "steam.h"
#include "Vector3.h"

using namespace NCL;
using namespace CSC8503;

void NetSystem_Steam::CreateLobby()
{
	SteamAPICall_t hCreateLobby = SteamMatchmaking()->CreateLobby(ELobbyType::k_ELobbyTypePublic, 4);

	m_LobbyCreateCallResult.Set(hCreateLobby, this, &NetSystem_Steam::On_LobbyCreated);
}

void NetSystem_Steam::On_LobbyCreated(LobbyCreated_t* pResult, bool bIOFailure)
{
	std::cout << "bIOFailure: " << bIOFailure << std::endl;
	std::cout << "LobbyCreatedResult: " << pResult->m_eResult << std::endl;
	std::cout << "m_ulSteamIDLobby: " << pResult->m_ulSteamIDLobby << std::endl;
}

void NetSystem_Steam::SearchLobbies()
{
	SteamAPICall_t hRequestLobbyList = SteamMatchmaking()->RequestLobbyList();

	m_LobbyMatchListCallResult.Set(hRequestLobbyList, this, &NetSystem_Steam::On_LobbyMatchList);
}

void NetSystem_Steam::On_LobbyMatchList(LobbyMatchList_t* pLobbyMatchList, bool bIOFailure)
{
	std::cout << "bIOFailure: " << bIOFailure << std::endl;
	std::cout << "Number Of Lobbies found: " << pLobbyMatchList->m_nLobbiesMatching << std::endl;
}