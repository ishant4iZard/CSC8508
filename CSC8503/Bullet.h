#pragma once
#include "GameObject.h"
#include "PhysicsObject.h"

namespace NCL {
	namespace CSC8503 {
		class NetworkPlayer;

		class bullet : public GameObject
		{
		public:
			static constexpr float FireForce = 40000;
			static constexpr float inverseMass = 1.0 / 10.0;

			static std::vector<bullet*> bulletList;
			static std::vector<bullet*> bulletsDiscard;
			static int bulletID;

			static void UpdateBulletList(){
				for (auto i = bulletList.begin(); i != bulletList.end();)
				{
					if ((*i)->GetPhysicsObject()->GetLinearVelocity().Length() <= 0.5f)
					{
						bullet* temp = *i;
						bulletsDiscard.push_back(temp);
						i = bulletList.erase(i);
						temp->DestroySelf();
					}
					else {
						++i;
					}
				}
			}

			void OnCollisionBegin(GameObject* otherObject) override;
			void setOwner(NetworkPlayer* owner) { this->owner = owner; }

			void DestroySelf();

			bullet() { owner = nullptr; }
			bullet(NetworkPlayer* owner);
			~bullet();

		protected:
			NetworkPlayer* owner;
		};

		
		class Item : public GameObject
		{
			typedef std::function<void(NetworkPlayer*, Item*)> OnItemCollisionBegin;

		public:
			Item(OnItemCollisionBegin someFunc) { func = someFunc; }

			void OnCollisionBegin(GameObject* otherObject) override;

			void setOwner(NetworkPlayer* owner) { this->owner = owner; }
			NetworkPlayer* getOwner() const { return owner; }
		protected:
			NetworkPlayer* owner = nullptr;
			OnItemCollisionBegin func;
		};
	}
}
