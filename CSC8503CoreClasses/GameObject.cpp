#include "GameObject.h"
#include "CollisionDetection.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "NetworkObject.h"
#include "Debug.h"

using namespace NCL::CSC8503;

GameObject::GameObject(const std::string& objectName)	{
	name			= objectName;
	worldID			= -1;
	isActive		= true;
	boundingVolume	= nullptr;
	physicsObject	= nullptr;
	renderObject	= nullptr;
	networkObject	= nullptr;
}

GameObject::~GameObject()	{
	delete boundingVolume;
	boundingVolume = nullptr;
	delete physicsObject;
	physicsObject = nullptr;
	delete renderObject;
	renderObject = nullptr;
	delete networkObject;
	networkObject = nullptr;
}

bool GameObject::GetBroadphaseAABB(Vector3&outSize) const {
	if (!boundingVolume || isActive == false) {
		return false;
	}
	outSize = broadphaseAABB;
	return true;
}

void GameObject::UpdateBroadphaseAABB() {
	if (!boundingVolume || isActive ==false) {
		return;
	}
	if (boundingVolume->type == VolumeType::AABB) {
		broadphaseAABB = ((AABBVolume&)*boundingVolume).GetHalfDimensions();
	}
	else if (boundingVolume->type == VolumeType::Sphere) {
		if (!SweptVolume) {
			float r = ((SphereVolume&)*boundingVolume).GetRadius();
			broadphaseAABB = Vector3(r, r, r);
		}
		else if (SweptVolume->type == VolumeType::Capsule) {
			Matrix3 mat = Matrix3(SweptTransform.GetOrientation());
			mat = mat.Absolute();
			float halfheight = ((CapsuleVolume&)*SweptVolume).GetHalfHeight();
			float radius = ((CapsuleVolume&)*SweptVolume).GetRadius();
			Vector3 halfSizes = Vector3(radius, halfheight, radius);
			broadphaseAABB = mat * halfSizes;
		}
	}
	else if (boundingVolume->type == VolumeType::OBB) {
		Matrix3 mat = Matrix3(transform.GetOrientation());
		mat = mat.Absolute();
		Vector3 halfSizes = ((OBBVolume&)*boundingVolume).GetHalfDimensions();
		broadphaseAABB = mat * halfSizes;
	}
	else if (boundingVolume->type == VolumeType::Capsule) {
		Matrix3 mat = Matrix3(transform.GetOrientation());
		mat = mat.Absolute();
		float halfheight = ((CapsuleVolume&)*boundingVolume).GetHalfHeight();
		float radius = ((CapsuleVolume&)*boundingVolume).GetRadius();
		Vector3 halfSizes = Vector3(radius, halfheight, radius);
		broadphaseAABB = mat * halfSizes;
	}
}

void GameObject::UpdateSweptVolume(float dt)
{
	if (!boundingVolume) return;
	if (boundingVolume->type == VolumeType::Sphere) {
		Vector3 velocity = GetPhysicsObject()->GetLinearVelocity();
		float r = ((SphereVolume&)*boundingVolume).GetRadius();
		float height = velocity.Length() * dt / 2;
		SweptVolume = new CapsuleVolume(height, r);	
		SweptTransform	.SetPosition(transform.GetPosition() + velocity.Normalised() * height)
						.SetScale(Vector3(r, height, r) * 2)
						.SetOrientation(Quaternion::AxisAngleToQuaterion(velocity.Normalised(), 180/3.14f));
		//Ray test = Ray(SweptTransform.GetPosition(), SweptTransform.GetOrientation().ToEuler());
		//Debug::DrawLine(test.GetPosition(), test.GetPosition() + test.GetDirection() * 10);
	}
	else {
		SweptVolume = new CollisionVolume;
	}
}
