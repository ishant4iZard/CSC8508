#pragma once
#include "GameObject.h"
#include <vector>
#include "Vector3.h"
#include "State.h"
#include "NavigationPath.h"
#include "NavigationGrid.h"
#include "TutorialGame.h"
#include "GameWorld.h"
#include "Player.h"

using NCL::Maths::Vector3;

namespace NCL::CSC8503
{
	class Goose : public GameObject
	{
	public:
		
		
		Goose ( GameWorld& inworld, Player* inplayer,const std::string& objectName = "");

		virtual void Update(float dt);
		virtual void OnCollisionBegin(GameObject* otherObject) override;
		void DebugDisplayPath(std::vector<Vector3> paths);

		bool LookingAtPlayer();
		State* Idle;
		State* Roaming;
		State* Chasing;

	protected:
		std::vector<Vector3> pathList;
		StateMachine* stateMachine;

		GameWorld& world;

		float meleeDamage = 2.0f;

		float pathTimer = 5.0f;
		float pathTimerCurrent = 0.0f;

		Vector3 mazePos;
		float mazeRadius = 48.0f;

		void FindRandomPatrolPoint();

		Player* player;

		Vector3 basePos = Vector3(0, -6.5f, 0);
		float moveSpeed = 6.0f;
		float rotationSpeed = 5.0f;
		float distanceThreshold = 2.0f;
		Vector3 previousPosition;

		NavigationGrid* grid;
		NavigationPath path;
		int currentDestinationIndex = 0;
		Vector3 currentDestination = Vector3(0, 0, 0);

		void MoveTowards(Vector3 src, const Vector3& pos, float dt);
		void MoveTowards(const Vector3& pos, float dt, bool useForce = false);
		void RotateTowards(const Vector3& pos, float rotSpeed, float dt);
		void RotateAway(const Vector3& pos, float rotSpeed, float dt);

		bool FindPath(const Vector3& destination, std::vector<Vector3>& _pathList);
		Vector3 GetRandomMazePoint();
	};
}
