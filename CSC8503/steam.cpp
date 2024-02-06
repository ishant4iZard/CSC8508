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

	steamIDLobby = pResult->m_ulSteamIDLobby;
	if (SetLobbyData(ELobbyDataType::EGameName, GameName.c_str()))
	{
		std::cout << "Set Game Name Success!\n";
	}
	
}

void NetSystem_Steam::SearchLobbies()
{
	ISteamMatchmaking* SteamMatchmakingPtr = SteamMatchmaking();

	SteamMatchmakingPtr->AddRequestLobbyListStringFilter(LobbyDataKey[ELobbyDataType::EGameName].c_str(), GameName.c_str(), ELobbyComparison::k_ELobbyComparisonEqual);

	SteamAPICall_t hRequestLobbyList = SteamMatchmakingPtr->RequestLobbyList();

	m_LobbyMatchListCallResult.Set(hRequestLobbyList, this, &NetSystem_Steam::On_LobbyMatchList);
}

void NetSystem_Steam::On_LobbyMatchList(LobbyMatchList_t* pLobbyMatchList, bool bIOFailure)
{
	std::cout << "bIOFailure: " << bIOFailure << std::endl;
	std::cout << "Number Of Lobbies found: " << pLobbyMatchList->m_nLobbiesMatching << std::endl;
}

bool NetSystem_Steam::SetLobbyData(ELobbyDataType type, string Value)
{
	if (steamIDLobby == 0) return false;

	return SteamMatchmaking()->SetLobbyData(steamIDLobby, LobbyDataKey[type].c_str(), Value.c_str());
}