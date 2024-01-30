#pragma once
#include "GameObject.h"
#include "GameClient.h"

#include "BehaviourNode.h"
#include "BehaviourParallel.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"

#include <queue>

using std::queue;

namespace NCL {
	namespace CSC8503 {
		class NetworkedGame;
		class StateMachine;

		enum ScoreType {
			bulletHitAI  = 3,
			bulletHitPlayer = 5,
			bringBackTreasure = 100
		};

		typedef vector<Vector3>::iterator waypointItr;

		class NetworkPlayer : public GameObject {
		public:
			static constexpr float SprintCDT = 4.0f;
			static constexpr float FireCDT = 2.0f;

			//static NetworkPlayer* createAngryGoose(NetworkedGame* game, int num);

			NetworkPlayer(NetworkedGame* game, int num);
			NetworkPlayer(NetworkedGame* game, int num, int AIKind);
			~NetworkPlayer();

			void OnCollisionBegin(GameObject* otherObject) override;

			void GameTick(float dt);
			void ExcuteBehavioursTree(float dt);

			int GetPlayerNum() const {
				return playerNum;
			}

			void SetBtnState(int btn, char val) { btnState[btn] = val; }
			char GetBtnState(int btn)const { return btnState[btn]; }

			void SetPlayerYaw(const Vector3& pointPos);
			void MovePlayer(bool Up, bool Down, bool Right, bool Left);

			bool AIMoveTo(Vector3 destination, float dt);
			bool AIMove(Vector3 destination);

			NetworkPlayer* AIvision();
			void UpdateVisualList(float dt);
			NetworkPlayer* getVisualTarget();

			void PlayerSprint();
			void PlayerFire();

			Vector3 getPlayerForwardVector();

			float getSprintCD() const { return sprintTimer < 0 ? 0 : sprintTimer; }
			void setSprintCD(float st) { sprintTimer = st; }
			float getFireCD() const { return fireTimer < 0 ? 0 : fireTimer; }
			void setFireCD(float ft) { fireTimer = ft; }

			int getPlayerScore() const { return score; }
			void setPlayerSocer(int score) { this->score = score; }
			void addPlayerScore(int increaseNum) { score += increaseNum; }

			NetworkedGame* getGame() { return game; }

			void setHaveTreasure(bool val) { haveTreasure = val; }
			bool getHaveTreasure() const { return haveTreasure; }

			StateMachine* getStateMachine() const { return stateMachine; }
			void setStateMachine(StateMachine* val) { stateMachine = val; }

			int getPatrolIndex()const { return patrolIndex; }
			void setPatrolIndex(int val) { patrolIndex = val; }
		protected:
			void UpdateTimer(float dt);

			void CreateUndercoverAgent();

			NetworkedGame* game;
			int playerNum;
			char btnState[4] = { 0,0,0,0 };

			int score;

			float sprintTimer;
			float fireTimer;

			float pathfindingTimer = 0.0f;
			vector<Vector3> waypoints;
			waypointItr waypoint;
			int patrolIndex;

			bool haveTreasure;

			StateMachine* stateMachine = nullptr;
			vector<std::pair<NetworkPlayer*, float>>  viualList;

			BehaviourSequence* rootSequence;
			NetworkPlayer* targetPlayer = nullptr;
		};
	}
}

