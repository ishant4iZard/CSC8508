#pragma once
#include "TutorialGame.h"
#include "NetworkBase.h"
#include "PushdownState.h"

namespace NCL {
	namespace CSC8503 {
		class GameServer;
		class GameClient;
		class NetworkPlayer;
		class PushdownMachine;
		class DeltaPacket;
		class FullPacket;
		class ClientPacket;
		class RoundStatePacket;
		class PlayerStatePacket;
		class BulletStatePacket;
		class Item;

		enum PlayInputBtns {
			Up,
			Down,
			Right,
			Left,
			Sprint,
			Fire
		};

		class NetworkedGame : public TutorialGame, public PacketReceiver {
		public:
			NetworkedGame();
			~NetworkedGame();

			bool StartAsServer();
			bool StartAsClient(char a, char b, char c, char d);

			void UpdateGame(float dt) override;

			void SpawnPlayer();
			void SpawnAI();
			void SpawnItem();
			void SpawnBullet(NetworkPlayer* o, Vector3 firePos, Vector3 fireDir);
			void SeverSendBulletDelPckt(int bulletID);
			void ClientSpawnBullet(int playNum, int bulletID);

			void RemoveObjectFromWorld(GameObject* o, bool andDelete);

			void StartLevel();
			void LevelOver();
			void LevelDelayOver(float dt);

			void ReceivePacket(int type, GamePacket* payload, int source) override;

			void OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b);

			bool isRoundStart() { return isRoundstart; }

			std::string getRoundTimeToString();
			std::string getLocalPLayerScoreToString();
			std::string getLocalPlayerSprintCDToString();
			std::string getLocalPlayerHasTreasure();
			std::string getLocalPlayerFireCDToString();
			std::string getPlayersScore(int ID);
			std::string getSelfPlace();
			void PrintPlayerFinalScore(int place, Vector2 pos);

			bool isGameOver() { return isGameover; }

			bool isServer() { return thisServer != nullptr; }
			bool isClient() { return thisClient != nullptr; }

			int GetPlayerPeerID(int num) { return PlayersList[num]; }
			int GetClientPlayerNum();
			int GetClientPlayerNum(int peerID);

		protected:
			void InitWorld() override;

			void UpdateAsServer(float dt);
			void UpdateAsClient(float dt);
			void ServerUpdatePlayerList();

			void UpdateGamePlayerInput(float dt);
			void UpdateScoreTable();

			void BroadcastSnapshot(bool deltaFrame);
			void ServerSendRoundState();
			void ServerSendPlayerState();
			void updateRoundTime(float dt);

			void UpdateMinimumState();

			bool serverProcessCP(ClientPacket* cp, int source);
			bool clientProcessRp(RoundStatePacket* rp);
			bool clientProcessFp(FullPacket* fp);
			bool clientProcessDp(DeltaPacket* dp);
			bool clientProcessPp(PlayerStatePacket* Pp);
			bool clientProcessBp(BulletStatePacket* Bp);

			void findOSpointerWorldPosition(Vector3& position);

			GameObject* AddNetPlayerToWorld(const Vector3& position, int playerNum);
			void AddNetOBBCube();

			std::map<int, int> stateIDs;  
			int GlobalStateID;

			GameServer* thisServer;
			GameClient* thisClient;
			float timeToNextPacket;
			int packetsToSnapshot;

			std::map<int, NetworkObject*> networkObjects;

			std::vector<int> PlayersList;
			std::vector<GameObject*> serverPlayers;
			GameObject* localPlayer;

			std::vector<NetworkPlayer*> geese;
			NetworkPlayer* undercoverAgent;

			PushdownMachine* MenuSystem;
			bool isRoundstart;
			bool isGameover;
			bool roundDelayOver;
			float delayTime;

			Item* treasure;

			float RoundTime;
			std::vector<int> scoreTable;
		};

		class InGame : public PushdownState
		{
			PushdownResult OnUpdate(float dt, PushdownState** newState) override
			{
				if (game)
				{
					NetworkedGame* thisGame = (NetworkedGame*)game;
					if (thisGame->isRoundStart()) { return RoundOnGoing(dt, thisGame); }
					else { return RoundOver(dt, newState, thisGame); }
				}
			}

		protected:
			PushdownResult RoundOver(float dt, PushdownState** newState, NetworkedGame* thisGame)
			{
				overDisplayTime -= dt;
				if (overDisplayTime > 0) { Debug::Print("Round Over!", Vector2(38, 35), Debug::RED); }
				else
				{
					Debug::Print(thisGame->getSelfPlace(), Vector2(15, 20), Debug::RED);
					Debug::Print("====================================", Vector2(15, 30), Debug::YELLOW);
					for (int i = 1; i < 5; ++i)
					{
						thisGame->PrintPlayerFinalScore(i, Vector2(18, 30 + i * 10));
					}
					Debug::Print("====================================", Vector2(15, 80), Debug::YELLOW);
					Debug::Print("Press Esc : Return to Multiplayer Lobby", Vector2(12, 90), Debug::YELLOW);
					if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) { return PushdownResult::Pop; }
				}
				return PushdownResult::NoChange;
			}

			PushdownResult RoundOnGoing(float dt, NetworkedGame* thisGame)
			{
				startDisplayTime -= dt;
				if (startDisplayTime > 0) { Debug::Print("Round Start!", Vector2(38, 35), Debug::RED); }
				blinkTime -= dt;
				if (blinkTime < 0) {
					displayTreasureSign = !displayTreasureSign;
					blinkTime = 0.6f;
				}

				Debug::Print(thisGame->getRoundTimeToString(), Vector2(34, 10), Debug::YELLOW);
				Debug::Print(thisGame->getLocalPLayerScoreToString(), Vector2(70, 10), Debug::YELLOW);
				Debug::Print(thisGame->getLocalPlayerSprintCDToString(), Vector2(12, 80), Debug::YELLOW);
				Debug::Print(thisGame->getLocalPlayerFireCDToString(), Vector2(70, 80), Debug::YELLOW);
				if (displayTreasureSign) { Debug::Print(thisGame->getLocalPlayerHasTreasure(), Vector2(30, 17), Debug::RED); }

				if (Window::GetKeyboard()->KeyHeld(KeyCodes::TAB))
				{
					Debug::Print("====================================", Vector2(15, 22), Debug::YELLOW);
					Debug::Print(thisGame->getPlayersScore(0), Vector2(18, 30), Debug::RED);
					Debug::Print(thisGame->getPlayersScore(1), Vector2(18, 40), Debug::BLUE);
					Debug::Print(thisGame->getPlayersScore(2), Vector2(18, 50), Debug::YELLOW);
					Debug::Print(thisGame->getPlayersScore(3), Vector2(18, 60), Debug::CYAN);
					Debug::Print("====================================", Vector2(15, 70), Debug::YELLOW);
				}
				else {
					Debug::Print("Hold TAB to show score table", Vector2(25, 95), Debug::YELLOW);
				}

				if (thisGame->isServer())
				{
					Debug::Print("You:Player 1", Vector2(5, 10), Debug::RED);
					if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) { thisGame->LevelOver(); }
				}
				if (thisGame->isClient())
				{
					switch (thisGame->GetClientPlayerNum())
					{
					case 1:
						Debug::Print("You:Player 2", Vector2(5, 10), Debug::BLUE);
						break;
					case 2:
						Debug::Print("You:Player 3", Vector2(5, 10), Debug::YELLOW);
						break;
					case 3:
						Debug::Print("You:Player 4", Vector2(5, 10), Debug::CYAN);
						break;
					}
				}
				return PushdownResult::NoChange;
			}

			float startDisplayTime = 1.2f;
			float overDisplayTime = 1.2f;
			float blinkTime = 0.6f;
			bool displayTreasureSign = true;
		};

		class MultiplayerLobby : public PushdownState
		{
			PushdownResult OnUpdate(float dt, PushdownState** newState) override
			{
				Debug::Print("MultiPlayer Lobby", Vector2(33, 10), Debug::YELLOW);
				Debug::Print("================================================", Vector2(5, 20), Debug::YELLOW);
				Debug::Print("================================================", Vector2(5, 70), Debug::YELLOW);
				Debug::Print("Press Esc : MultiPlayer Menu", Vector2(5, 90), Debug::YELLOW);
				if (game)
				{
					NetworkedGame* thisGame = (NetworkedGame*)game;
					if (thisGame->isRoundStart())
					{
						*newState = new InGame();
						return PushdownResult::Push;
					}
					if (thisGame->isServer())
					{
						Debug::Print("is You", Vector2(40, 30), Debug::RED);
						Debug::Print("Press S : Game Start!", Vector2(5, 80), Debug::YELLOW);
						if (Window::GetKeyboard()->KeyPressed(KeyCodes::S))
						{
							thisGame->StartLevel();
						}
					}
					if (thisGame->isClient())
					{
						int num = thisGame->GetClientPlayerNum();
						Debug::Print("is You", Vector2(40, 30 + num * 10), Debug::RED);
						Debug::Print("Wait for Server to start game....", Vector2(5, 80), Debug::YELLOW);
					}
					if (thisGame->GetPlayerPeerID(0) != -1)
					{
						Debug::Print("Player 1", Vector2(5, 30), Debug::RED);
					}
					if (thisGame->GetPlayerPeerID(1) != -1)
					{
						Debug::Print("Player 2", Vector2(5, 40), Debug::BLUE);
					}
					if (thisGame->GetPlayerPeerID(2) != -1)
					{
						Debug::Print("Player 3", Vector2(5, 50), Debug::YELLOW);
					}
					if (thisGame->GetPlayerPeerID(3) != -1)
					{
						Debug::Print("Player 4", Vector2(5, 60), Debug::CYAN);
					}
				}
				if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE))
				{
					return PushdownResult::Pop;
				}
				return PushdownResult::NoChange;
			}
		};

		class MultiplayerMenu : public PushdownState
		{
			PushdownResult OnUpdate(float dt, PushdownState** newState) override
			{
				Debug::Print("Press 1 : Start As Server", Vector2(5, 10), Debug::YELLOW);
				Debug::Print("Press 2 : Start As Client", Vector2(5, 20), Debug::YELLOW);
				Debug::Print("Press Esc : Main Menu", Vector2(5, 70), Debug::YELLOW);

				if (game)
				{
					NetworkedGame* thisGame = (NetworkedGame*)game;
					if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM1))
					{
						thisGame->StartAsServer();
						*newState = new MultiplayerLobby();
						return PushdownResult::Push;
					}
					if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM2))
					{
						if (thisGame->StartAsClient(127, 0, 0, 1))
						{
							*newState = new MultiplayerLobby();
							return PushdownResult::Push;
						}
						Debug::Print("Failed to connect the server!!!", Vector2(50, 30), Debug::RED);
					}
				}
				if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE))
				{
					return PushdownResult::Pop;
				}
				return PushdownResult::NoChange;
			}
		};

		class HintMenu : public PushdownState
		{
			PushdownResult OnUpdate(float dt, PushdownState** newState) override
			{
				Debug::Print("Mouse Move for Rotate Player", Vector2(5, 10), Debug::YELLOW);
				Debug::Print("Mouse Left Btn for Player Fire", Vector2(5, 17), Debug::YELLOW);
				Debug::Print("W for Move Up", Vector2(5, 24), Debug::YELLOW);
				Debug::Print("S for Move Down", Vector2(5, 31), Debug::YELLOW);
				Debug::Print("A for Move Left", Vector2(5, 38), Debug::YELLOW);
				Debug::Print("D for Move Right", Vector2(5, 45), Debug::YELLOW);
				Debug::Print("Shift for Sprint Forward", Vector2(5, 52), Debug::YELLOW);


				Debug::Print("Press Esc : Back to Main menu ", Vector2(5, 80), Debug::YELLOW);
				if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE))
				{
					return PushdownResult::Pop;
				}
				return PushdownResult::NoChange;
			}

		};

		class MainMenu : public PushdownState
		{
			PushdownResult OnUpdate(float dt, PushdownState** newState) override
			{
				Debug::Print("The Crazy Goat!!", Vector2(35, 13), GetColorChanged(dt)); 
				Debug::Print("Press 1 : Solo Game", Vector2(5, 23), Debug::YELLOW);
				Debug::Print("Press 2 : MultiPlayer Game", Vector2(5, 33), Debug::YELLOW);
				Debug::Print("Press 3 : Control Hint", Vector2(5, 43), Debug::YELLOW);
				Debug::Print("Press 4 : Option", Vector2(5, 53), Debug::YELLOW);
				Debug::Print("Press Esc : Game Over", Vector2(5, 80), Debug::YELLOW);

				if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM1))
				{

				}
				if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM2))
				{
					*newState = new MultiplayerMenu();
					return PushdownResult::Push;
				}
				if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM3))
				{
					*newState = new HintMenu();
					return PushdownResult::Push;
				}
				if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM4))
				{

				}
				if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE))
				{
					return PushdownResult::Pop;
				}
				return PushdownResult::NoChange;
			}

		protected:
			Vector4 GetColorChanged(float dt)
			{
				time += dt;
				float r = (std::sin(time) + 1.0f) / 2.0f;
				float g = (std::sin(time + 2.094f) + 1.0f) / 2.0f;
				float b = (std::sin(time + 4.188f) + 1.0f) / 2.0f;

				return Vector4(r, g, b, 1.0);
			}
			float time = 0.0f;
		};
	}
}



