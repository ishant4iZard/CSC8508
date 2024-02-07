#include "steam.h"
#include "Vector3.h"

using namespace NCL;
using namespace CSC8503;

void NetSystem_Steam::CreateLobby()
{
	if (CurrentUserState != NetSystem_Steam::EAvailable) return;

	SteamAPICall_t hCreateLobby = SteamMatchmaking()->CreateLobby(ELobbyType::k_ELobbyTypePublic, 4);

	m_LobbyCreateCallResult.Set(hCreateLobby, this, &NetSystem_Steam::On_LobbyCreated);
}

void NetSystem_Steam::On_LobbyCreated(LobbyCreated_t* pResult, bool bIOFailure)
{
	std::cout << "LobbyCreatedResult: ";
	switch (pResult->m_eResult)
	{
	case EResult::k_EResultOK:
		std::cout << "the lobby was successfully created\n";
		break;
	case EResult::k_EResultNoConnection:
		std::cout << "your Steam client doesn't have a connection to the back-end\n";
		break;
	case EResult::k_EResultTimeout:
		std::cout << "you the message to the Steam servers, but it didn't respond\n";
		break;
	case EResult::k_EResultFail:
		std::cout << "the server responded, but with an unknown internal error\n";
		break;
	case EResult::k_EResultAccessDenied:
		std::cout << "your game isn't set to allow lobbies, or your client does haven't rights to play the game\n";
		break;
	case EResult::k_EResultLimitExceeded:
		std::cout << "your game client has created too many lobbies\n";
		break;
	}
	std::cout << "m_ulSteamIDLobby: " << pResult->m_ulSteamIDLobby << std::endl;

	switch (pResult->m_eResult) {
	case EResult::k_EResultOK:
		CurrentUserState = NetSystem_Steam::ELobbyHolder;
		break;
	}

	steamIDLobby = pResult->m_ulSteamIDLobby;
	if (SetLobbyData(ELobbyDataType::EGameName, GameName.c_str()))
	{
		std::cout << "Set Game Name Success!\n";
	}
	CSteamID ownerID = SteamMatchmaking()->GetLobbyOwner(steamIDLobby);
	string ownerName = SteamFriends()->GetFriendPersonaName(ownerID);
	std::cout << "Lobby Owner: " << ownerName << std::endl;
}

void NetSystem_Steam::SearchLobbies()
{
	ISteamMatchmaking* SteamMatchmakingPtr = SteamMatchmaking();

	SteamMatchmakingPtr->AddRequestLobbyListStringFilter(LobbyDataKey[ELobbyDataType::EGameName].c_str(), GameName.c_str(), ELobbyComparison::k_ELobbyComparisonEqual);

	SteamAPICall_t hRequestLobbyList = SteamMatchmakingPtr->RequestLobbyList();

	numLobbyMatchList = -1;

	CurrentUserState = NetSystem_Steam::ESearching;

	m_LobbyMatchListCallResult.Set(hRequestLobbyList, this, &NetSystem_Steam::On_LobbyMatchList);
}

void NetSystem_Steam::On_LobbyMatchList(LobbyMatchList_t* pLobbyMatchList, bool bIOFailure)
{
	std::cout << "Number Of Lobbies found: " << pLobbyMatchList->m_nLobbiesMatching << std::endl;

	numLobbyMatchList = pLobbyMatchList->m_nLobbiesMatching;

	CurrentUserState = NetSystem_Steam::EAvailable;
}

void NetSystem_Steam::SelectLobbyByIndex(int Index)
{
	if (Index < numLobbyMatchList)
	{
		steamIDLobby = SteamMatchmaking()->GetLobbyByIndex(Index).ConvertToUint64();
	}
}

CSteamID NetSystem_Steam::GetLobbyIDByIndex(int Index)
{	
	if (Index >= numLobbyMatchList) return CSteamID();

	return  SteamMatchmaking()->GetLobbyByIndex(Index);
}

void NetSystem_Steam::JoinLobby()
{
	if (steamIDLobby == 0) return;

	SteamAPICall_t hJoinLobby = SteamMatchmaking()->JoinLobby(steamIDLobby);
}

void NetSystem_Steam::On_LobbyJoined(LobbyEnter_t* pResult, bool bIOFailure)
{
	CurrentUserState = NetSystem_Steam::ELobbyJoiner;
	std::cout << "Join lobby of : " << SteamFriends()->GetFriendPersonaName(SteamMatchmaking()->GetLobbyOwner(pResult->m_ulSteamIDLobby)) << std::endl;
}

void NetSystem_Steam::On_LobbyChatUpdate(LobbyChatUpdate_t* pCallback)
{
	
}

bool NetSystem_Steam::SetLobbyData(ELobbyDataType type, string Value)
{
	if (steamIDLobby == 0) return false;

	return SteamMatchmaking()->SetLobbyData(steamIDLobby, LobbyDataKey[type].c_str(), Value.c_str());
}

string NetSystem_Steam::GetLobbyOwnerNameByLobbyID(CSteamID LobbyID) const
{
	return string(SteamFriends()->GetFriendPersonaName(SteamMatchmaking()->GetLobbyOwner(LobbyID)));
}

void NetSystem_Steam::On_LobbyDataUpdate(LobbyDataUpdate_t* pCallback)
{
}
