#pragma once
#include "Transform.h"
#include "CollisionVolume.h"

using std::vector;
#define SAFE_DELETE(a) if( (a) != nullptr ) delete (a); (a) = nullptr;

namespace NCL::CSC8503 {
	class NetworkObject;
	class RenderObject;
	class PhysicsObject;


	//add enums for layers and its collision flag
	//add forward raycast

	class GameObject	{
	public:
		GameObject(const std::string& name = "");
		~GameObject();

		void Update(float dt);

		void SetBoundingVolume(CollisionVolume* vol) {
			boundingVolume = vol;
		}

		const CollisionVolume* GetBoundingVolume() const {
			return boundingVolume;
		}

		bool IsActive() const {
			return isActive;
		}
		void deactivate() {
			isActive = false;
		}
		void activate() {
			isActive = true;
		}

		Transform& GetTransform() {
			return transform;
		}
		
		Transform& GetSweptTransform() {
			return SweptTransform;
		}

		RenderObject* GetRenderObject() const {
			return renderObject;
		}

		PhysicsObject* GetPhysicsObject() const {
			return physicsObject;
		}

		NetworkObject* GetNetworkObject() const {
			return networkObject;
		}

		void SetRenderObject(RenderObject* newObject) {
			renderObject = newObject;
		}

		void SetPhysicsObject(PhysicsObject* newObject) {
			physicsObject = newObject;
		}

		void SetNetworkObject(NetworkObject* newObject) {
			networkObject = newObject;
		}

		const std::string& GetName() const {
			return name;
		}

		virtual void OnCollisionBegin(GameObject* otherObject) {
			//std::cout << "OnCollisionBegin event occured!\n";
		}

		virtual void OnCollisionEnd(GameObject* otherObject) {
			//std::cout << "OnCollisionEnd event occured!\n";
		}
		virtual void OnTriggerBegin(GameObject* otherObject) {
			//std::cout << "OnCollisionBegin event occured!\n";
		}

		virtual void OnTriggerEnd(GameObject* otherObject) {
			//std::cout << "OnCollisionEnd event occured!\n";
		}



		bool GetBroadphaseAABB(Vector3&outsize) const;

		CollisionVolume* getSweptVolume() const {
			return SweptVolume;
		}

		void UpdateBroadphaseAABB();

		void UpdateSweptVolume(float dt);

		void SetWorldID(int newID) {
			worldID = newID;
		}

		int		GetWorldID() const {
			return worldID;
		}

		std::string gettag() {
			return tag;
		}

		void settag(std::string ttag) {
			tag = ttag;
		}
		
		void setName(std::string inname) {
			name = inname;
		}



		

	protected:
		Transform			transform;

		CollisionVolume*	boundingVolume;
		PhysicsObject*		physicsObject;
		RenderObject*		renderObject;
		NetworkObject*		networkObject;

		CollisionVolume*	SweptVolume;
		Transform			SweptTransform;

		bool		isActive;
		int			worldID;
		std::string	name;
		std::string	tag;

		Vector3 broadphaseAABB;
	};
}

