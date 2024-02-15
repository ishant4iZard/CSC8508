#include "steam.h"
#include "Vector3.h"
#include "Event.h"

using namespace NCL;
using namespace CSC8503;

NetSystem_Steam* NetSystem_Steam::steam = nullptr;

NetSystem_Steam* NetSystem_Steam::GetInstance()
{
	if (steam == nullptr)
	{
		steam = new NetSystem_Steam();
	}
	return steam;
}

void NetSystem_Steam::CreateLobby()
{
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
		OnCreateLobbySuccess();
		break;

	case EResult::k_EResultNoConnection:
		std::cout << "your Steam client doesn't have a connection to the back-end\n";

	case EResult::k_EResultTimeout:
		std::cout << "you the message to the Steam servers, but it didn't respond\n";

	case EResult::k_EResultFail:
		std::cout << "the server responded, but with an unknown internal error\n";

	case EResult::k_EResultAccessDenied:
		std::cout << "your game isn't set to allow lobbies, or your client does haven't rights to play the game\n";

	case EResult::k_EResultLimitExceeded:
		std::cout << "your game client has created too many lobbies\n";

	default:
		OnCreateLobbyFailed();
	}

	std::cout << "m_ulSteamIDLobby: " << pResult->m_ulSteamIDLobby << std::endl;
	steamIDLobby = pResult->m_ulSteamIDLobby;

	if (SetLobbyData(ELobbyDataType::EGameName, GameName.c_str()))
	{
		std::cout << "Set Game Name Success!\n";
	}
	string OwnerName = SteamFriends()->GetPersonaName();
	if (SetLobbyData(ELobbyDataType::EOwnerName, OwnerName.c_str()))
	{
		std::cout << "Set Owner Name Success!\n";
	}
	if (SetLobbyData(ELobbyDataType::ELocalIPv4Address, LocalIPv4Address))
	{
		std::cout << "Set Loacl IPv4 Address Success!\n";
	}

	std::cout << "Lobby Owner: " << OwnerName << std::endl;
}

void NetSystem_Steam::OnCreateLobbySuccess()
{
	EventEmitter::EmitEvent(EventType::LOBBY_CREATED);
}

void NetSystem_Steam::OnCreateLobbyFailed()
{
	EventEmitter::EmitEvent(EventType::LOBBY_CREATEFAILED);
}

void NetSystem_Steam::SearchLobbies()
{
	ISteamMatchmaking* SteamMatchmakingPtr = SteamMatchmaking();

	SteamMatchmakingPtr->AddRequestLobbyListStringFilter(LobbyDataKey[ELobbyDataType::EGameName].c_str(), GameName.c_str(), ELobbyComparison::k_ELobbyComparisonEqual);

	SteamAPICall_t hRequestLobbyList = SteamMatchmakingPtr->RequestLobbyList();

	numLobbyMatchList = -1;

	m_LobbyMatchListCallResult.Set(hRequestLobbyList, this, &NetSystem_Steam::On_LobbyMatchList);
}

void NetSystem_Steam::On_LobbyMatchList(LobbyMatchList_t* pLobbyMatchList, bool bIOFailure)
{
	std::cout << "Number Of Lobbies found: " << pLobbyMatchList->m_nLobbiesMatching << std::endl;

	numLobbyMatchList = pLobbyMatchList->m_nLobbiesMatching;

	OnLobbyMatchList();
}

void NetSystem_Steam::OnLobbyMatchList()
{
	EventEmitter::EmitEvent(EventType::LOBBY_SEARCHCOMPLETED);
}

void NetSystem_Steam::SelectLobbyByIndex(int Index)
{
	if (Index >= 0 && Index < numLobbyMatchList)
	{
		steamIDLobby = SteamMatchmaking()->GetLobbyByIndex(Index).ConvertToUint64();
	}
}

void NetSystem_Steam::JoinLobby()
{
	if (steamIDLobby == 0) return;

	SteamAPICall_t hJoinLobby = SteamMatchmaking()->JoinLobby(steamIDLobby);
	m_JoinLobbyCallResult.Set(hJoinLobby, this, &NetSystem_Steam::On_LobbyJoined);
}

void NetSystem_Steam::On_LobbyJoined(LobbyEnter_t* pResult, bool bIOFailure)
{
	if (pResult->m_EChatRoomEnterResponse == EChatRoomEnterResponse::k_EChatRoomEnterResponseSuccess)
	{
		std::cout << "Joined lobby of : " << SteamFriends()->GetFriendPersonaName(SteamMatchmaking()->GetLobbyOwner(pResult->m_ulSteamIDLobby)) << std::endl;
		LobbyHolderIPv4Address = SteamMatchmaking()->GetLobbyData(steamIDLobby, LobbyDataKey[ELobbyDataType::ELocalIPv4Address].c_str());
		std::cout << "OnwerIPAddress: " << LobbyHolderIPv4Address << std::endl;

		int numMembers = GetNumCurrentLobbyMembers();
		for (int i = 0; i < numMembers; ++i)
		{
			std::cout << "Member " << i << ":";
			std::cout << SteamFriends()->GetFriendPersonaName(SteamMatchmaking()->GetLobbyMemberByIndex(pResult->m_ulSteamIDLobby, i)) << std::endl;
		}

		OnJoinLobbySuccess();
	}
	else if (pResult->m_EChatRoomEnterResponse == EChatRoomEnterResponse::k_EChatRoomEnterResponseError)
	{
		std::cout << "Join Lobby failed!\n";
		OnJoinLobbyFailed();
	}
	else
	{
		std::cout << "Join Lobby failed! --- UnkownError\n";
		OnJoinLobbyFailed();
	}

}

void NetSystem_Steam::OnJoinLobbySuccess()
{
	EventEmitter::EmitEvent(EventType::LOBBY_JOINED);
}

void NetSystem_Steam::OnJoinLobbyFailed()
{
	EventEmitter::EmitEvent(EventType::LOBBY_JOINFAILED);
}

void NetSystem_Steam::LeaveLobby()
{
	if (steamIDLobby == 0) return;

	SteamMatchmaking()->LeaveLobby(steamIDLobby);
}

void NetSystem_Steam::On_LobbyChatUpdate(LobbyChatUpdate_t* pCallback)
{
	std::cout << SteamFriends()->GetFriendPersonaName(pCallback->m_ulSteamIDUserChanged) << " data changed: " << pCallback->m_rgfChatMemberStateChange;

	int numMembers = GetNumCurrentLobbyMembers();
	for (int i = 0; i < numMembers; ++i)
	{
		std::cout << "Member " << i << ":";
		std::cout << SteamFriends()->GetFriendPersonaName(SteamMatchmaking()->GetLobbyMemberByIndex(steamIDLobby, i)) << std::endl;
	}

	std::cout << SteamFriends()->GetFriendPersonaName(SteamMatchmaking()->GetLobbyOwner(steamIDLobby)) << std::endl;
}

bool NetSystem_Steam::SetLobbyData(ELobbyDataType type, string Value)
{
	if (steamIDLobby == 0) return false;

	return SteamMatchmaking()->SetLobbyData(steamIDLobby, LobbyDataKey[type].c_str(), Value.c_str());
}

string NetSystem_Steam::GetLobbyOwnerNameByIndex(int Index) const
{
	CSteamID thisLobbyID = SteamMatchmaking()->GetLobbyByIndex(Index);

	return string(SteamMatchmaking()->GetLobbyData(thisLobbyID, LobbyDataKey[ELobbyDataType::EOwnerName].c_str()));
}

int NetSystem_Steam::GetNumCurrentLobbyMembers() const
{
	if (steamIDLobby == 0)return 0;

	return SteamMatchmaking()->GetNumLobbyMembers(steamIDLobby);
}

int NetSystem_Steam::GetNumLobbyMembersByIndex(int Index) const
{
	return SteamMatchmaking()->GetNumLobbyMembers(SteamMatchmaking()->GetLobbyByIndex(Index));
}

string NetSystem_Steam::GetLobbyMemberNameByIndex(int Index) const
{
	return SteamFriends()->GetFriendPersonaName(SteamMatchmaking()->GetLobbyMemberByIndex(steamIDLobby, Index));
}

void NetSystem_Steam::On_LobbyDataUpdate(LobbyDataUpdate_t* pCallback)
{
	
}

