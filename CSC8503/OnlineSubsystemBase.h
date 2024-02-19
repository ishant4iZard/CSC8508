#pragma once

#include <string>


namespace NCL {
	namespace  CSC8503 {

		using std::string;

		class OnlineSubsystemBase
		{
		public:
			/**  */
			virtual void CreateLobby() = 0;

			virtual void SearchLobbies() = 0;

			virtual void SelectLobbyByIndex(int Index) = 0;

			virtual void JoinLobby() = 0;

			virtual void LeaveLobby() = 0;

			virtual string GetLobbyOwnerNameByIndex(int Index) const = 0;

			virtual int GetNumCurrentLobbyMembers() const = 0;

			virtual int GetNumLobbyMembersByIndex(int Index) const = 0;

			virtual string GetLobbyMemberNameByIndex(int Index) const = 0;

			virtual void SetCurrentUserName() = 0;

			virtual void SetLocalIPv4Address(const string& IPAddress) = 0;

			inline string GetCurrentUserName() const { return CurrentUserName; }
			inline int GetNumOfLobbyMatchList() const { return numLobbyMatchList; }

			inline bool GetIsOnlineSubsystemInitSuccess() const { return isOnlineSubsystemInitSuccess; }
			inline void SetIsOnlineSubsystemInitSuccess(bool result) { isOnlineSubsystemInitSuccess = result; }

		protected:
			virtual void OnCreateLobbySuccess() = 0;
			virtual void OnCreateLobbyFailed() = 0;
			virtual void OnLobbyMatchList() = 0;
			virtual void OnJoinLobbySuccess() = 0;
			virtual void OnJoinLobbyFailed() = 0;

			string CurrentUserName;
			int numLobbyMatchList = 0;

			bool isOnlineSubsystemInitSuccess;
		};
	}
}

