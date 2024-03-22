#pragma once
#include "GameWorld.h"
#include "QuadTree.h"
#include "../CSC8503/Event.h"
#include "../CSC8503/PowerUp.h"

namespace NCL {
	namespace CSC8503 {
		class PhysicsSystem:public EventListener{
		public:
			PhysicsSystem(GameWorld& g);
			~PhysicsSystem();

			void Clear();

			void Update(float dt);

			void UseGravity(bool state) {
				applyGravity = state;
			}

			void SetGlobalDamping(float d) {
				globalDamping = d;
			}
			void SetLinearDamping(float d) {
				linearDamping = d;
			}

			void SetGravity(const Vector3& g);

			void createStaticTree();

			void SetBroadphase(bool is) {
				useBroadPhase = is;
			}

			void ReceiveEvent(EventType T) override;

			int GetNumberOfCollisions() const { return allCollisions.size(); }

			inline powerUpType GetCurrentPowerUpState() const { return activePowerup; }

		protected:
			void BasicCollisionDetection();
			void BroadPhase();
			void NarrowPhase();

			void ClearForces();

			void IntegrateAccel(float dt);
			void IntegrateVelocity(float dt);

			void UpdateConstraints(float dt);

			void UpdateCollisionList();
			void UpdateObjectAABBs();
			void UpdateObjectSwept(float dt);

			void ImpulseResolveCollision(GameObject& a , GameObject&b, CollisionDetection::ContactPoint& p) const;

			GameWorld& gameWorld;

			bool	applyGravity;
			Vector3 gravity;
			float	dTOffset;
			float	globalDamping;
			float	linearDamping;
			

			std::set<CollisionDetection::CollisionInfo> allCollisions;
			std::set<CollisionDetection::CollisionInfo> broadphaseCollisions;
			std::vector<CollisionDetection::CollisionInfo> broadphaseCollisionsVec;
			bool useBroadPhase		= true;
			int numCollisionFrames	= 5;

			powerUpType activePowerup;
			float powerUptime;
			const float powerUpLifetime = 10;

			Vector3 wind = Vector3();
		};
	}
}

