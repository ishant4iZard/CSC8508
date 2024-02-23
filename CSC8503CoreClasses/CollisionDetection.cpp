#include "CollisionDetection.h"
#include "CollisionVolume.h"
#include "AABBVolume.h"
#include "OBBVolume.h"
#include "SphereVolume.h"
#include "Window.h"
#include "Maths.h"
#include "Debug.h"

using namespace NCL;

bool CollisionDetection::RayPlaneIntersection(const Ray&r, const Plane&p, RayCollision& collisions) {
	float ln = Vector3::Dot(p.GetNormal(), r.GetDirection());

	if (ln == 0.0f) {
		return false; //direction vectors are perpendicular!
	}
	
	Vector3 planePoint = p.GetPointOnPlane();

	Vector3 pointDir = planePoint - r.GetPosition();

	float d = Vector3::Dot(pointDir, p.GetNormal()) / ln;

	collisions.collidedAt = r.GetPosition() + (r.GetDirection() * d);

	return true;
}

bool CollisionDetection::RayIntersection(const Ray& r,GameObject& object, RayCollision& collision) {
	bool hasCollided = false;

	const Transform& worldTransform = object.GetTransform();
	const CollisionVolume* volume	= object.GetBoundingVolume();

	if (!volume || volume->isTrigger ||!object.IsActive()) {
		return false;
	}

	switch (volume->type) {
		case VolumeType::AABB:		hasCollided = RayAABBIntersection(r, worldTransform, (const AABBVolume&)*volume	, collision); break;
		case VolumeType::OBB:		hasCollided = RayOBBIntersection(r, worldTransform, (const OBBVolume&)*volume	, collision); break;
		case VolumeType::Sphere:	hasCollided = RaySphereIntersection(r, worldTransform, (const SphereVolume&)*volume	, collision); break;

		case VolumeType::Capsule:	hasCollided = RayCapsuleIntersection(r, worldTransform, (const CapsuleVolume&)*volume, collision); break;
	}

	return hasCollided;
}

bool CollisionDetection::RayBoxIntersection(const Ray&r, const Vector3& boxPos, const Vector3& boxSize, RayCollision& collision) {
	
	Vector3 boxMin = boxPos - boxSize;
	Vector3 boxMax = boxPos + boxSize;

	Vector3 rayPos = r.GetPosition();
	Vector3 rayDir = r.GetDirection();

	Vector3 tVals(-1, -1, -1);

	for (int i = 0; i < 3; ++i) {
		if (rayDir[i] > 0) {
			tVals[i] = (boxMin[i] - rayPos[i]) / rayDir[i];	
		}
		else if (rayDir[i] < 0) {
			tVals[i] = (boxMax[i] - rayPos[i]) / rayDir[i];
		}
	}

	float bestT = tVals.GetMaxElement();
	if (bestT < 0.0f) {
		return false;
	}

	Vector3 intersection = rayPos + (rayDir * bestT);
	const float epsilon = 0.0001f;

	for (int i = 0; i < 3; ++i) {
		if (intersection[i] + epsilon < boxMin[i] || intersection[i] - epsilon > boxMax[i]) {
			return false;
		}
	}

	collision.collidedAt = intersection;
	collision.rayDistance = bestT;


	return true;
}

bool CollisionDetection::RayAABBIntersection(const Ray&r, const Transform& worldTransform, const AABBVolume& volume, RayCollision& collision) {
	Vector3 boxPos = worldTransform.GetPosition();
	Vector3 boxSize = volume.GetHalfDimensions();
	return RayBoxIntersection(r, boxPos, boxSize, collision);
}

bool CollisionDetection::RayOBBIntersection(const Ray&r, const Transform& worldTransform, const OBBVolume& volume, RayCollision& collision) {
	Quaternion orientation = worldTransform.GetOrientation();
	Vector3 position = worldTransform.GetPosition();

	Matrix3 transform = Matrix3(orientation);
	Matrix3 invTransform = Matrix3(orientation.Conjugate());

	Vector3 localRayPos = r.GetPosition() - position;

	Ray tempRay(invTransform * localRayPos, invTransform * r.GetDirection());

	bool collided = RayBoxIntersection(tempRay, Vector3(),
		volume.GetHalfDimensions(), collision);

	if (collided) {
		collision.collidedAt = transform * collision.collidedAt + position;
	}
	return collided;
}

bool CollisionDetection::RaySphereIntersection(const Ray&r, const Transform& worldTransform, const SphereVolume& volume, RayCollision& collision) {
	Vector3	spherePos = worldTransform.GetPosition();
	float sphereRadius = volume.GetRadius();

	Vector3 dir = (spherePos - r.GetPosition());

	float sphereProj = Vector3::Dot(dir, r.GetDirection());

	if (sphereProj < 0.0f) {
		return false;
	}

	Vector3 point = r.GetPosition() + (r.GetDirection() * sphereProj);
	float sphereDist = (point - spherePos).Length();
	
	if (sphereDist > sphereRadius) {
			return false;
	}

	float offset =
		sqrt((sphereRadius * sphereRadius) - (sphereDist * sphereDist));

	collision.rayDistance = sphereProj - (offset);
	collision.collidedAt = r.GetPosition() +
		(r.GetDirection() * collision.rayDistance);
	return true;
}

float findTCapsule(Vector3 pa, Vector3 pb, float capsuleRadius, const Ray& r) {
	Vector3 lineSegment = pb - pa;
	Vector3 relativeOrigin = r.GetPosition() - pa;
	float baba = Vector3::Dot(lineSegment, lineSegment);
	float bard = Vector3::Dot(lineSegment, r.GetDirection());
	float baoa = Vector3::Dot(lineSegment, relativeOrigin);
	float rdoa = Vector3::Dot(r.GetDirection(), relativeOrigin);
	float oaoa = Vector3::Dot(relativeOrigin, relativeOrigin);
	float a = baba - bard * bard;
	float b = baba * rdoa - baoa * bard;
	float c = baba * oaoa - baoa * baoa - capsuleRadius * capsuleRadius * baba;
	float h = b * b - a * c;

	if (h >= 0.0)
	{
		float t = (-b - sqrt(h)) / a;
		float y = baoa + t * bard;
		if (y > 0.0 && y < baba) 
			return t;
		
		Vector3 oc = (y <= 0.0) ? relativeOrigin : r.GetPosition() - pb;
		b = Vector3::Dot(r.GetDirection(), oc);
		c = Vector3::Dot(oc, oc) - capsuleRadius * capsuleRadius;
		h = b * b - c;
		if (h > 0.0)
			return -b - sqrt(h);
		
	}
	return -1;
}

bool CollisionDetection::RayCapsuleIntersection(const Ray& r, const Transform& worldTransform, const CapsuleVolume& volume, RayCollision& collision) {
	Vector3	capsulePos = worldTransform.GetPosition();
	Quaternion orientation = worldTransform.GetOrientation(); 
	float capsuleRadius = volume.GetRadius();
	float halfHeight = volume.GetHalfHeight();

	Vector3 Updirection = orientation * Vector3(0, 1, 0);

	Vector3 pa = capsulePos + Updirection * halfHeight;
	Vector3 pb = capsulePos - Updirection * halfHeight;

	float t = findTCapsule(pa, pb, capsuleRadius, r);

	if (t == -1)
		return false;

	collision.rayDistance = t;
	collision.collidedAt = r.GetPosition() +
		(r.GetDirection() * collision.rayDistance);

	return true;
}

bool CollisionDetection::ObjectIntersection(GameObject* a, GameObject* b, CollisionInfo& collisionInfo) {
	const CollisionVolume* volA = a->GetBoundingVolume();
	const CollisionVolume* volB = b->GetBoundingVolume();

	if (!volA || !volB) {
		return false;
	}

	collisionInfo.a = a;
	collisionInfo.b = b;

	Transform& transformA = a->GetTransform();
	Transform& transformB = b->GetTransform();

	VolumeType pairType = (VolumeType)((int)volA->type | (int)volB->type);

	//Two AABBs
	if (pairType == VolumeType::AABB) {
		return AABBIntersection((AABBVolume&)*volA, transformA, (AABBVolume&)*volB, transformB, collisionInfo);
	}
	//Two Spheres
	if (pairType == VolumeType::Sphere) {
		return SphereIntersection((SphereVolume&)*volA, transformA, (SphereVolume&)*volB, transformB, collisionInfo);
	}
	//Two OBBs
	if (pairType == VolumeType::OBB) {
		return OBBIntersection((OBBVolume&)*volA, transformA, (OBBVolume&)*volB, transformB, collisionInfo);
	}
	//Two Capsules
	if (pairType == VolumeType::Capsule) {
		//capsule intersection
		return CapsuleIntersection((CapsuleVolume&)*volA, transformA, (CapsuleVolume&)*volB, transformB, collisionInfo);
	}

	//AABB vs Sphere pairs
	if (volA->type == VolumeType::AABB && volB->type == VolumeType::Sphere) {
		return AABBSphereIntersection((AABBVolume&)*volA, transformA, (SphereVolume&)*volB, transformB, collisionInfo);
	}
	if (volA->type == VolumeType::Sphere && volB->type == VolumeType::AABB) {
		collisionInfo.a = b;
		collisionInfo.b = a;
		return AABBSphereIntersection((AABBVolume&)*volB, transformB, (SphereVolume&)*volA, transformA, collisionInfo);
	}

	//OBB vs sphere pairs
	if (volA->type == VolumeType::OBB && volB->type == VolumeType::Sphere) {
		return OBBSphereIntersection((OBBVolume&)*volA, transformA, (SphereVolume&)*volB, transformB, collisionInfo);
	}
	if (volA->type == VolumeType::Sphere && volB->type == VolumeType::OBB) {
		collisionInfo.a = b;
		collisionInfo.b = a;
		return OBBSphereIntersection((OBBVolume&)*volB, transformB, (SphereVolume&)*volA, transformA, collisionInfo);
	}

	//Capsule vs other interactions
	if (volA->type == VolumeType::Capsule && volB->type == VolumeType::Sphere) {
		return SphereCapsuleIntersection((CapsuleVolume&)*volA, transformA, (SphereVolume&)*volB, transformB, collisionInfo);
	}
	if (volA->type == VolumeType::Sphere && volB->type == VolumeType::Capsule) {
		collisionInfo.a = b;
		collisionInfo.b = a;
		return SphereCapsuleIntersection((CapsuleVolume&)*volB, transformB, (SphereVolume&)*volA, transformA, collisionInfo);
	}

	if (volA->type == VolumeType::Capsule && volB->type == VolumeType::AABB) {
		return AABBCapsuleIntersection((CapsuleVolume&)*volA, transformA, (AABBVolume&)*volB, transformB, collisionInfo);
	}
	if (volB->type == VolumeType::Capsule && volA->type == VolumeType::AABB) {
		collisionInfo.a = b;
		collisionInfo.b = a;
		return AABBCapsuleIntersection((CapsuleVolume&)*volB, transformB, (AABBVolume&)*volA, transformA, collisionInfo);
	}

	if (volA->type == VolumeType::OBB && volB->type == VolumeType::AABB) {
		bool temp = OBBIntersection((OBBVolume&)*volA, transformA, (OBBVolume&)*volB, transformB, collisionInfo);
		collisionInfo.point.localB = Vector3();
		return temp;
	}
	if (volB->type == VolumeType::OBB && volA->type == VolumeType::AABB) {
		collisionInfo.a = b;
		collisionInfo.b = a;
		bool temp = OBBIntersection((OBBVolume&)*volB, transformB, (OBBVolume&)*volA, transformA, collisionInfo);
		collisionInfo.point.localA = Vector3();
		return temp;
	}
	if (volA->type == VolumeType::OBB && volB->type == VolumeType::Capsule) {
		bool temp = OBBCapsuleIntersection((OBBVolume&)*volA, transformA, (CapsuleVolume&)*volB, transformB, collisionInfo);
		return temp;
	}
	if (volB->type == VolumeType::OBB && volA->type == VolumeType::Capsule) {
		collisionInfo.a = b;
		collisionInfo.b = a;
		return OBBCapsuleIntersection((OBBVolume&)*volB, transformB, (CapsuleVolume&)*volA, transformA, collisionInfo);
		
	}
	if (volA->type == VolumeType::AABB && volB->type == VolumeType::Capsule) {
		bool temp = AABBCapsuleIntersection((CapsuleVolume&)*volB, transformB, (AABBVolume&)*volA, transformA, collisionInfo);
		return temp;
	}
	if (volB->type == VolumeType::AABB && volA->type == VolumeType::Capsule) {
		collisionInfo.a = b;
		collisionInfo.b = a;
		return AABBCapsuleIntersection((CapsuleVolume&)*volA, transformA, (AABBVolume&)*volB, transformB, collisionInfo);
	}



	return false;
}

bool CollisionDetection::AABBTest(const Vector3& posA, const Vector3& posB, const Vector3& halfSizeA, const Vector3& halfSizeB) {
	Vector3 delta = posB - posA;
	Vector3 totalSize = halfSizeA + halfSizeB;

	if (abs(delta.x) < totalSize.x &&
		abs(delta.y) < totalSize.y &&
		abs(delta.z) < totalSize.z) {
		return true;
	}
	return false;
}

//AABB/AABB Collisions
bool CollisionDetection::AABBIntersection(const AABBVolume& volumeA, const Transform& worldTransformA,
	const AABBVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {
	Vector3 boxAPos = worldTransformA.GetPosition();
	Vector3 boxBPos = worldTransformB.GetPosition();

	Vector3 boxASize = volumeA.GetHalfDimensions();
	Vector3 boxBSize = volumeB.GetHalfDimensions();

	bool overlap = AABBTest(boxAPos, boxBPos, boxASize, boxBSize);
	if (overlap) {
		static const Vector3 faces[6] =
		{
			Vector3(-1, 0, 0), Vector3(1, 0, 0),
			Vector3(0, -1, 0), Vector3(0, 1, 0),
			Vector3(0, 0, -1), Vector3(0, 0, 1),
		};

		Vector3 maxA = boxAPos + boxASize;
		Vector3 minA = boxAPos - boxASize;

		Vector3 maxB = boxBPos + boxBSize;
		Vector3 minB = boxBPos - boxBSize;

		float distances[6] =
		{
			(maxB.x - minA.x),// distance of box ’b’ to ’left ’ of ’a ’.
			(maxA.x - minB.x),// distance of box ’b’ to ’right ’ of ’a ’.
			(maxB.y - minA.y),// distance of box ’b’ to ’bottom ’ of ’a ’.
			(maxA.y - minB.y),// distance of box ’b’ to ’top ’ of ’a ’.
			(maxB.z - minA.z),// distance of box ’b’ to ’far ’ of ’a ’.
			(maxA.z - minB.z) // distance of box ’b’ to ’near ’ of ’a ’.
		};
		float penetration = FLT_MAX;
		Vector3 bestAxis;

		for (int i = 0; i < 6; i++)
		{
			if (distances[i] < penetration) {
				penetration = distances[i];
				bestAxis = faces[i];

			}
		}
		collisionInfo.AddContactPoint(Vector3(), Vector3(),
			bestAxis, penetration);
		return true;

	}

	return false;
}

//Sphere / Sphere Collision
bool CollisionDetection::SphereIntersection(const SphereVolume& volumeA, const Transform& worldTransformA,
	const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {
	float radii = volumeA.GetRadius() + volumeB.GetRadius();
	Vector3 delta = worldTransformB.GetPosition() -
		worldTransformA.GetPosition();

	float deltaLength = delta.Length();

	if (deltaLength < radii) {
		float penetration = (radii - deltaLength);
		Vector3 normal = delta.Normalised();
		Vector3 localA = normal * volumeA.GetRadius();
		Vector3 localB = -normal * volumeB.GetRadius();
		collisionInfo.AddContactPoint(localA, localB, normal, penetration);
		return true;// we ’re colliding !
	}
	return false;
}

//AABB - Sphere Collision
bool CollisionDetection::AABBSphereIntersection(const AABBVolume& volumeA, const Transform& worldTransformA,
	const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {
	Vector3 boxSize = volumeA.GetHalfDimensions();

	Vector3 delta = worldTransformB.GetPosition() -
		worldTransformA.GetPosition();

	Vector3 closestPointOnBox = Vector3::Clamp(delta, -boxSize, boxSize);
	Vector3 localPoint = delta - closestPointOnBox;
	float distance = localPoint.Length();

	if (distance < volumeB.GetRadius()) {// yes , we ’re colliding !
		Vector3 collisionNormal = localPoint.Normalised();
		float penetration = (volumeB.GetRadius() - distance);

		Vector3 localA = Vector3();
		Vector3 localB = -collisionNormal * volumeB.GetRadius();

		collisionInfo.AddContactPoint(localA, localB,
			collisionNormal, penetration);
		return true;

	}
	return false;
}




bool  CollisionDetection::OBBSphereIntersection(const OBBVolume& volumeA, const Transform& worldTransformA,
	const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {
	Quaternion orientation = worldTransformA.GetOrientation();

	Matrix3 transform = Matrix3(orientation);
	Matrix3 invTransform = Matrix3(orientation.Conjugate());

	Vector3 boxSize = volumeA.GetHalfDimensions();
	Vector3 delta = invTransform * (worldTransformB.GetPosition()- worldTransformA.GetPosition());
	Vector3 closestPointOnBox = Vector3::Clamp(delta, -boxSize, boxSize);

	Vector3 localPoint = delta - closestPointOnBox;
	float distance = localPoint.Length();

	if (distance < volumeB.GetRadius()) {// yes , we ’re colliding !
		Vector3 collisionNormal =  transform * localPoint.Normalised();
		float penetration = (volumeB.GetRadius() - distance);

		Vector3 localA = collisionNormal * closestPointOnBox;
		Vector3 localB = -collisionNormal * volumeB.GetRadius();

	
		collisionInfo.AddContactPoint(localA, localB,
			collisionNormal, penetration);
		return true;
	}
	return false;
}



void ClosestPtPointSegment(Vector3 a, Vector3 b, Vector3 c, float& t, Vector3& d) {
	Vector3 ab = b - a;
	t = Vector3::Dot(c - a, ab) / Vector3::Dot(ab, ab);
	d = a + ab * fmin(fmax(t, 0), 1) ;
}

bool CollisionDetection::AABBCapsuleIntersection(
	const CapsuleVolume& volumeA, const Transform& worldTransformA,
	const AABBVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {

	Vector3	capsulePos = worldTransformA.GetPosition();
	Quaternion orientation = worldTransformA.GetOrientation();
	float capsuleRadius = volumeA.GetRadius();
	float halfHeight = volumeA.GetHalfHeight();
	Vector3 Updirection = orientation * Vector3(0, 1, 0);
	Vector3 pa = capsulePos + Updirection * halfHeight;
	Vector3 pb = capsulePos - Updirection * halfHeight;

	Vector3 cubePos = Vector3::Clamp(worldTransformA.GetPosition(), worldTransformB.GetPosition() - volumeB.GetHalfDimensions(), worldTransformB.GetPosition() + volumeB.GetHalfDimensions());

	Vector3 bestTpoint;
	float bestT;

	ClosestPtPointSegment(pa, pb, cubePos, bestT, bestTpoint);

	SphereVolume s(capsuleRadius);
	Transform t;
	t.SetPosition(bestTpoint)
		.SetScale(Vector3(1, 1, 1) * capsuleRadius);


	
	bool collided = AABBSphereIntersection(volumeB, worldTransformB, s, t, collisionInfo);
	collisionInfo.point.localA += t.GetPosition() - worldTransformA.GetPosition();

	collisionInfo.point.normal *= -1;

	return collided;
}

bool CollisionDetection::OBBCapsuleIntersection(const OBBVolume& volumeA, const Transform& worldTransformA, const CapsuleVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo)
{
	Quaternion orientation = worldTransformA.GetOrientation();
	Matrix3 invTransform = Matrix3(orientation.Conjugate());

	CapsuleVolume c(volumeB.GetHalfHeight()/2, volumeB.GetRadius()/2);
	Transform cT;
	cT.SetPosition(invTransform * worldTransformB.GetPosition());
	cT.SetScale(Vector3(volumeB.GetRadius(), volumeB.GetHalfHeight(), volumeB.GetRadius())*2);
	cT.SetOrientation(invTransform * worldTransformB.GetOrientation());

	AABBVolume a(volumeA.GetHalfDimensions());
	Transform aT;
	aT.SetPosition(invTransform * worldTransformA.GetPosition());
	aT.SetScale(volumeA.GetHalfDimensions());

	bool collided = AABBCapsuleIntersection(c, cT, a, aT, collisionInfo);
	collisionInfo.point.localA = Matrix3(orientation) * collisionInfo.point.localA;
	collisionInfo.point.localB = Matrix3(orientation) * collisionInfo.point.localB;
	collisionInfo.point.normal = Matrix3(orientation) * collisionInfo.point.normal * -1;
	return collided;
}

bool CollisionDetection::SphereCapsuleIntersection(
	const CapsuleVolume& volumeA, const Transform& worldTransformA,
	const SphereVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {
	
	Vector3	capsulePos = worldTransformA.GetPosition();
	Quaternion orientation = worldTransformA.GetOrientation();
	float capsuleRadius = volumeA.GetRadius();
	float halfHeight = volumeA.GetHalfHeight();
	Vector3 Updirection = orientation * Vector3(0, 1, 0);
	Vector3 pa = capsulePos + Updirection * halfHeight;
	Vector3 pb = capsulePos - Updirection * halfHeight;


	Vector3 spherePos = worldTransformB.GetPosition();
	float sphereRadius = volumeB.GetRadius();

	float radius = sphereRadius + capsuleRadius;
	/*float dist2 = SqDistPointSegment(pa, pb, spherePos);

	float penetration = radius - sqrt(dist2);
	if (penetration < 0) {
		return false;
	}*/

	Vector3 bestTpoint;
	float bestT;

	ClosestPtPointSegment(pa, pb, spherePos, bestT, bestTpoint);

	float penetration = radius - (bestTpoint - spherePos).Length();
	if (penetration < 0) {
		return false;
	}

	//std::cout << bestT<<"\n";
	//Vector3 localA = collisionPoint - capsulePos;

	Vector3 collisionNormal = (spherePos - bestTpoint).Normalised();

	Vector3 localA = (bestTpoint - collisionNormal * capsuleRadius) - capsulePos;

	collisionInfo.AddContactPoint(localA, collisionNormal * capsuleRadius,
		collisionNormal, penetration);


	return true;
}


vector<Vector3> getVertices(const Transform& worldTransform, const OBBVolume& volume) {
	Vector3 center = worldTransform.GetPosition();
	Vector3 boxSize = volume.GetHalfDimensions();
	Quaternion orientation = worldTransform.GetOrientation();


	Matrix3 transform = Matrix3(orientation);
	Matrix3 invTransform = Matrix3(orientation.Conjugate());

	vector<Vector3> vertices;
	for (int i = 0; i < 8; i++) {
		Vector3 vertex;
		if (i < 4)
			vertex.x = boxSize.x;
		else
			vertex.x = -boxSize.x;
		if (i < 2 || i == 4 || i == 5)
			vertex.y = boxSize.y;
		else
			vertex.y = -boxSize.y;
		if (i % 2 == 0)
			vertex.z = boxSize.z;
		else
			vertex.z = -boxSize.z;

		vertex = invTransform * vertex;
		vertex += center;
		vertices.push_back(vertex);
	}
	return vertices;
}

vector<Vector3> getAxis(const Transform& worldTransform) {
	Quaternion orientation = worldTransform.GetOrientation();
	Vector3 position = worldTransform.GetPosition();

	vector<Vector3> axis;

	axis.push_back(orientation * Vector3(1, 0, 0));
	axis.push_back(orientation * Vector3(0, 1, 0));
	axis.push_back(orientation * Vector3(0, 0, 1));

	return axis;
}

//bool CollisionDetection::OBBIntersection(const OBBVolume& volumeA, const Transform& worldTransformA,
//	const OBBVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo) {
//	float ra, rb;
//	Matrix3 R, AbsR;
//	vector<Vector3> axisA = getAxis(worldTransformA);
//	vector<Vector3> axisB = getAxis(worldTransformB);
//	Vector3 boxsizeA = volumeA.GetHalfDimensions();
//	Vector3 boxsizeB = volumeB.GetHalfDimensions();
//
//	float minRa,minRb,penetration;
//	float minPenetration = std::numeric_limits<float>::max();
//	Vector3 collisionNormal;
//
//	for (int i = 0; i < 3; i++) {
//		for (int j = 0; j < 3; j++) {
//			R.array[i][j] = Vector3::Dot(axisA[i], axisB[j]);
//			AbsR.array[i][j] = std::abs(R.array[i][j]);
//		}
//	}
//	Vector3 t = worldTransformB.GetPosition() - worldTransformA.GetPosition();
//	t = Vector3(Vector3::Dot(t, axisA[0]), Vector3::Dot(t, axisA[1]), Vector3::Dot(t, axisA[2]));
//
//	for (int i = 0; i < 3; i++) {
//		ra = boxsizeA[i];
//		rb = boxsizeB[0] * AbsR.array[i][0] + boxsizeB[1] * AbsR.array[i][1] + boxsizeB[2] * AbsR.array[i][2];
//		if (std::abs(t[i]) > ra + rb) 
//			return 0; 
//		penetration = ra + rb - std::abs(t[i]);
//		if (penetration < minPenetration) {
//			minRa = ra;
//			minRb = rb;
//			minPenetration = penetration;
//			collisionNormal = axisA[i].Normalised();
//		}
//	}
//	for (int i = 0; i < 3; i++) {
//		ra = boxsizeA[0] * AbsR.array[0][i] + boxsizeA[1] * AbsR.array[1][i] + boxsizeA[2] * AbsR.array[2][1];
//		rb = boxsizeB[i] ;
//		float tempd = std::abs(t[0] * R.array[0][i] + t[1] * R.array[1][i] + t[2] * R.array[2][i]);
//		if (tempd > ra + rb)
//			return 0;
//		penetration = ra + rb - tempd;
//		if (penetration < minPenetration) {
//			minRa = ra;
//			minRb = rb;
//			minPenetration = penetration;
//			collisionNormal = axisB[i].Normalised();
//		}
//	}
//	float tempd;
//	ra = boxsizeA[1] * AbsR.array[2][0] + boxsizeA[2] * AbsR.array[1][0];
//	rb = boxsizeA[1] * AbsR.array[0][2] + boxsizeA[2] * AbsR.array[0][1];
//	tempd = std::abs(t[2] * R.array[1][0] - t[1] * R.array[2][0]);
//	if (tempd > ra + rb)
//		return 0;
//	penetration = ra + rb - tempd;
//	if (penetration + 0.1f < minPenetration) {
//		Vector3 tempcollisionNormal = Vector3::Cross(axisA[0], axisB[0]);
//		if (tempcollisionNormal != Vector3()){
//			minRa = ra;
//			minRb = rb;
//			minPenetration = penetration;
//			collisionNormal = tempcollisionNormal.Normalised();
//		}
//	}
//
//	ra = boxsizeA[1] * AbsR.array[2][1] + boxsizeA[2] * AbsR.array[1][1];
//	rb = boxsizeA[0] * AbsR.array[0][2] + boxsizeA[2] * AbsR.array[0][0];
//	tempd = std::abs(t[2] * R.array[1][1] - t[1] * R.array[2][1]);
//	if (tempd > ra + rb)
//		return 0;
//	penetration = ra + rb - tempd;
//	if (penetration + 0.1f < minPenetration) {
//		Vector3 tempcollisionNormal = Vector3::Cross(axisA[0], axisB[1]);
//		if (tempcollisionNormal != Vector3()) {
//			minRa = ra;
//			minRb = rb;
//			minPenetration = penetration;
//			collisionNormal = tempcollisionNormal.Normalised();
//		}
//	}
//
//	ra = boxsizeA[1] * AbsR.array[2][2] + boxsizeA[2] * AbsR.array[1][2]; 
//	rb = boxsizeA[0] * AbsR.array[0][1] + boxsizeA[1] * AbsR.array[0][0]; 
//	tempd = std::abs(t[2] * R.array[1][2] - t[1] * R.array[2][2]);
//	if (tempd > ra + rb)
//		return 0;
//	penetration = ra + rb - tempd;
//	if (penetration + 0.1f < minPenetration) {
//		Vector3 tempcollisionNormal = Vector3::Cross(axisA[0], axisB[2]);
//		if (tempcollisionNormal != Vector3()) {
//			minRa = ra;
//			minRb = rb;
//			minPenetration = penetration;
//			collisionNormal = tempcollisionNormal.Normalised();
//		}
//	}
//
//	ra = boxsizeA[0] * AbsR.array[2][0] + boxsizeA[2] * AbsR.array[0][0]; 
//	rb = boxsizeA[1] * AbsR.array[1][2] + boxsizeA[2] * AbsR.array[1][1];
//	tempd = std::abs(t[0] * R.array[2][0] - t[2] * R.array[0][0]);
//	if (tempd > ra + rb)
//		return 0;
//	penetration = ra + rb - tempd;
//	if (penetration + 0.1f < minPenetration) {
//		Vector3 tempcollisionNormal = Vector3::Cross(axisA[1], axisB[0]);
//		if (tempcollisionNormal != Vector3()) {
//			minRa = ra;
//			minRb = rb;
//			minPenetration = penetration;
//			collisionNormal = tempcollisionNormal.Normalised();
//		}
//	}
//
//	ra = boxsizeA[0] * AbsR.array[2][1] + boxsizeA[2] * AbsR.array[0][1]; 
//	rb = boxsizeA[0] * AbsR.array[1][2] + boxsizeA[2] * AbsR.array[1][0]; 
//	tempd = std::abs(t[0] * R.array[2][1] - t[2] * R.array[0][1]);
//	if (tempd > ra + rb)
//		return 0; 
//	penetration = ra + rb - tempd;
//	if (penetration + 0.1f < minPenetration) {
//		Vector3 tempcollisionNormal = Vector3::Cross(axisA[1], axisB[1]);
//		if (tempcollisionNormal != Vector3()) {
//			minRa = ra;
//			minRb = rb;
//			minPenetration = penetration;
//			collisionNormal = tempcollisionNormal.Normalised();
//		}
//	}
//
//	ra = boxsizeA[0] * AbsR.array[2][2] + boxsizeA[2] * AbsR.array[0][2]; 
//	rb = boxsizeA[0] * AbsR.array[1][1] + boxsizeA[1] * AbsR.array[1][0]; 
//	tempd = std::abs(t[0] * R.array[2][2] - t[2] * R.array[0][2]);
//	if (tempd > ra + rb)
//		return 0; 
//	penetration = ra + rb - tempd;
//	if (penetration + 0.1f < minPenetration) {
//		Vector3 tempcollisionNormal = Vector3::Cross(axisA[1], axisB[2]);
//		if (tempcollisionNormal != Vector3()) {
//			minRa = ra;
//			minRb = rb;
//			minPenetration = penetration;
//			collisionNormal = tempcollisionNormal.Normalised();
//		}
//	}
//
//	ra = boxsizeA[0] * AbsR.array[1][0] + boxsizeA[1] * AbsR.array[0][0]; 
//	rb = boxsizeA[1] * AbsR.array[2][2] + boxsizeA[2] * AbsR.array[2][1]; 
//	tempd = std::abs(t[1] * R.array[0][0] - t[0] * R.array[1][0]);
//	if (tempd > ra + rb)
//		return 0; 
//	penetration = ra + rb - tempd;
//	if (penetration + 0.1f < minPenetration) {
//		Vector3 tempcollisionNormal = Vector3::Cross(axisA[2], axisB[0]);
//		if (tempcollisionNormal != Vector3()) {
//			minRa = ra;
//			minRb = rb;
//			minPenetration = penetration;
//			collisionNormal = tempcollisionNormal.Normalised();
//		}
//	}
//	
//	ra = boxsizeA[0] * AbsR.array[1][1] + boxsizeA[1] * AbsR.array[0][1]; 
//	rb = boxsizeA[0] * AbsR.array[2][2] + boxsizeA[2] * AbsR.array[2][0]; 
//	tempd = std::abs(t[1] * R.array[0][1] - t[0] * R.array[1][1]);
//	if (tempd > ra + rb)
//		return 0; 
//	penetration = ra + rb - tempd;
//	if (penetration + 0.1f < minPenetration) {
//		Vector3 tempcollisionNormal = Vector3::Cross(axisA[2], axisB[1]);
//		if (tempcollisionNormal != Vector3()) {
//			minRa = ra;
//			minRb = rb;
//			minPenetration = penetration;
//			collisionNormal = tempcollisionNormal.Normalised();
//		}
//	}
//
//	ra = boxsizeA[0] * AbsR.array[1][2] + boxsizeA[1] * AbsR.array[0][2]; 
//	rb = boxsizeA[0] * AbsR.array[2][1] + boxsizeA[1] * AbsR.array[2][0]; 
//	tempd = std::abs(t[1] * R.array[0][2] - t[0] * R.array[1][2]);
//	if (tempd > ra + rb)
//		return 0;
//	penetration = ra + rb - tempd;
//	if (penetration+0.1f < minPenetration) {
//		Vector3 tempcollisionNormal = Vector3::Cross(axisA[2], axisB[2]);
//		if (tempcollisionNormal != Vector3()) {
//			minRa = ra;
//			minRb = rb;
//			minPenetration = penetration;
//			collisionNormal = tempcollisionNormal.Normalised();
//		}
//	}
//
//	Vector3 localA = collisionNormal * minRa;
//	Vector3 localB = -collisionNormal * minRb;
//	
//	collisionInfo.AddContactPoint(localA, localB,
//		collisionNormal, minPenetration);
//
//	return true;
//}
//

bool CollisionDetection::CapsuleIntersection(const CapsuleVolume& volumeA, const Transform& worldTransformA,
	const CapsuleVolume& volumeB, const Transform& worldTransformB, CollisionInfo& collisionInfo)
{
	//upgraded the collison detection from https://wickedengine.net/2020/04/26/capsule-collision-detection/ 

	//Capsule A
	Vector3	capsuleAPos = worldTransformA.GetPosition();
	Quaternion capsuleAorientation = worldTransformA.GetOrientation();
	float capsuleARadius = volumeA.GetRadius();
	float capsuleAhalfHeight = volumeA.GetHalfHeight();
	Vector3 capsuleAUpdirection = capsuleAorientation * Vector3(0, 1, 0);
	Vector3 capsuleApa = capsuleAPos + capsuleAUpdirection * capsuleAhalfHeight;
	Vector3 capsuleApb = capsuleAPos - capsuleAUpdirection * capsuleAhalfHeight;
	/*Vector3 capsuleALineEndOffset = capsuleAUpdirection * capsuleARadius;
	Vector3 capsuleApaEnd = capsuleApa + capsuleALineEndOffset;
	Vector3 capsuleApbEnd = capsuleApb - capsuleALineEndOffset;*/

	//capsule B
	Vector3	capsuleBPos = worldTransformB.GetPosition();
	Quaternion capsuleBorientation = worldTransformB.GetOrientation();
	float capsuleBRadius = volumeB.GetRadius();
	float capsuleBhalfHeight = volumeB.GetHalfHeight();
	Vector3 capsuleBUpdirection = capsuleBorientation * Vector3(0, 1, 0);
	Vector3 capsuleBpa = capsuleBPos + capsuleBUpdirection * capsuleBhalfHeight;
	Vector3 capsuleBpb = capsuleBPos - capsuleBUpdirection * capsuleBhalfHeight;
	//Vector3 capsuleBLineEndOffset = capsuleBUpdirection * capsuleBRadius;
	/*Vector3 capsuleBpaEnd = capsuleBpa - capsuleBLineEndOffset;
	Vector3 capsuleBpbEnd = capsuleBpb - capsuleBLineEndOffset;*/

	Vector3 ApaBpa = capsuleBpa - capsuleApa;
	Vector3 ApaBpb = capsuleBpb - capsuleApa;
	Vector3 ApbBpa = capsuleBpa - capsuleApb;
	Vector3 ApbBpb = capsuleBpb - capsuleApb;

	float d0 = ApaBpa.LengthSquared();
	float d1 = ApaBpb.LengthSquared();
	float d2 = ApbBpa.LengthSquared();
	float d3 = ApbBpb.LengthSquared();

	Vector3 bestTApoint, bestTBpoint;
	float bestTA, bestTB;

	if (d2 < d0 || d2 < d1 || d3 < d0 || d3 < d1)
	{
		bestTApoint = capsuleApa;
	}
	else
	{
		bestTApoint = capsuleApb;
	}

	ClosestPtPointSegment(capsuleBpa, capsuleBpb, bestTApoint, bestTB, bestTBpoint);
	ClosestPtPointSegment(capsuleApa, capsuleApb, bestTBpoint, bestTA, bestTApoint);

	//got the best two spheres
	float radii = capsuleARadius + capsuleBRadius;
	Vector3 delta = bestTBpoint - bestTApoint;

	float deltaLength = delta.Length();

	if (deltaLength <= radii) {
		float penetration = (radii - deltaLength);
		Vector3 normal = delta.Normalised();
		Vector3 localA = (bestTApoint - normal * capsuleARadius) - capsuleAPos;
		Vector3 localB = (bestTBpoint + normal * capsuleBRadius) - capsuleBPos;
		collisionInfo.AddContactPoint(localA, localB, normal, penetration);
		std::cout << "collision";
		return true;// we ’re colliding !
	}

	//std::cout << " no collision";

	return false;
}


bool CollisionDetection::OBBIntersection(const OBBVolume& volumeA, const Transform& worldTransformA,
	const OBBVolume& volumeB, const Transform& worldTransformB,
	CollisionInfo& collisionInfo)
{
	vector<Vector3> directions;
	directions.push_back(worldTransformA.GetOrientation() * Vector3(1, 0, 0));
	directions.push_back(worldTransformA.GetOrientation() * Vector3(0, 1, 0));
	directions.push_back(worldTransformA.GetOrientation() * Vector3(0, 0, 1)); 

	directions.push_back(worldTransformB.GetOrientation() * Vector3(1, 0, 0));
	directions.push_back(worldTransformB.GetOrientation() * Vector3(0, 1, 0));
	directions.push_back(worldTransformB.GetOrientation() * Vector3(0, 0, 1));

	for (int i = 0; i < 3; i++) {
		for (int j = 3; j < 6; j++) {
			directions.push_back(Vector3::Cross(directions[i], directions[j]));
		}
	}

	float leastPenetration = FLT_MAX;

	for (int i = 0; i < 15; ++i) {
		if (Vector3::Dot(directions[i], directions[i]) < 0.99f)
			continue;

		// Get min and max extents for both shapes along an axis
		Vector3 maxA = OBBSupport(worldTransformA, directions[i]);
		Vector3 minA = OBBSupport(worldTransformA, -directions[i]);

		Vector3 maxB = OBBSupport(worldTransformB, directions[i]);
		Vector3 minB = OBBSupport(worldTransformB, -directions[i]);

		// Project those points on to the line
		float denom = Vector3::Dot(directions[i], directions[i]);

		Vector3 minExtentA = directions[i] * (Vector3::Dot(minA, directions[i]) / denom);
		Vector3 maxExtentA = directions[i] * (Vector3::Dot(maxA, directions[i]) / denom);
		Vector3 minExtentB = directions[i] * (Vector3::Dot(minB, directions[i]) / denom);
		Vector3 maxExtentB = directions[i] * (Vector3::Dot(maxB, directions[i]) / denom);

		float distance = FLT_MAX;
		float length = FLT_MAX;
		float penDist = FLT_MAX;

		float BoxAleft = Vector3::Dot(maxExtentA - minExtentA, minExtentB - minExtentA);
		float BoxAright = Vector3::Dot(minExtentA - maxExtentA, maxExtentB - maxExtentA);

		if (BoxAright > 0.0f) {
			// Object B to the left
			distance = (maxExtentB - maxExtentA).Length();
			length = (maxExtentA - minExtentA).Length();
			if (distance <= length) {
				penDist = length - distance;
				if (penDist < leastPenetration) {
					leastPenetration = penDist;
					collisionInfo.point.localA = minA;
					collisionInfo.point.localB = maxB;
					collisionInfo.point.normal = -directions[i];
				}
				continue;
			}
		}

		if (BoxAleft > 0.0f) {
			// Object A to the left
			distance = (minExtentB - minExtentA).Length();
			length = (maxExtentA - minExtentA).Length();
			if (distance <= length) {
				penDist = length - distance;
				if (penDist < leastPenetration) {
					leastPenetration = penDist;
					collisionInfo.point.localA = maxA;
					collisionInfo.point.localB = minB;
					collisionInfo.point.normal = directions[i];
				}
				continue;
			}
		}

		if (BoxAleft < 0.0f && BoxAright < 0.0f) {
			// Fully eveloped, should never be minimum intersection so don't update
			continue;
		}

		return false;
	}
	collisionInfo.point.penetration = leastPenetration;
	collisionInfo.point.localA -= worldTransformA.GetPosition();
	collisionInfo.point.localB -= worldTransformB.GetPosition();
	return true;
}

Matrix4 GenerateInverseView(const Camera &c) {
	float pitch = c.GetPitch();
	float yaw	= c.GetYaw();
	Vector3 position = c.GetPosition();

	Matrix4 iview =
		Matrix4::Translation(position) *
		Matrix4::Rotation(-yaw, Vector3(0, -1, 0)) *
		Matrix4::Rotation(-pitch, Vector3(-1, 0, 0));

	return iview;
}

Matrix4 GenerateInverseProjection(float aspect, float nearPlane, float farPlane, float fov) {
	float negDepth = nearPlane - farPlane;

	float invNegDepth = negDepth / (2 * (farPlane * nearPlane));

	Matrix4 m;

	float h = 1.0f / tan(fov*PI_OVER_360);

	m.array[0][0] = aspect / h;
	m.array[1][1] = tan(fov * PI_OVER_360);
	m.array[2][2] = 0.0f;

	m.array[2][3] = invNegDepth;//// +PI_OVER_360;
	m.array[3][2] = -1.0f;
	m.array[3][3] = (0.5f / nearPlane) + (0.5f / farPlane);

	return m;
}

Vector3 CollisionDetection::Unproject(const Vector3& screenPos, const PerspectiveCamera& cam) {
	Vector2i screenSize = Window::GetWindow()->GetScreenSize();

	float aspect	= (float)screenSize.x / (float)screenSize.y;
	float fov		= cam.GetFieldOfVision();
	float nearPlane = cam.GetNearPlane();
	float farPlane  = cam.GetFarPlane();

	//Create our inverted matrix! Note how that to get a correct inverse matrix,
	//the order of matrices used to form it are inverted, too.
	Matrix4 invVP = GenerateInverseView(cam) * GenerateInverseProjection(aspect, fov, nearPlane, farPlane);

	Matrix4 proj  = cam.BuildProjectionMatrix(aspect);

	//Our mouse position x and y values are in 0 to screen dimensions range,
	//so we need to turn them into the -1 to 1 axis range of clip space.
	//We can do that by dividing the mouse values by the width and height of the
	//screen (giving us a range of 0.0 to 1.0), multiplying by 2 (0.0 to 2.0)
	//and then subtracting 1 (-1.0 to 1.0).
	Vector4 clipSpace = Vector4(
		(screenPos.x / (float)screenSize.x) * 2.0f - 1.0f,
		(screenPos.y / (float)screenSize.y) * 2.0f - 1.0f,
		(screenPos.z),
		1.0f
	);

	//Then, we multiply our clipspace coordinate by our inverted matrix
	Vector4 transformed = invVP * clipSpace;

	//our transformed w coordinate is now the 'inverse' perspective divide, so
	//we can reconstruct the final world space by dividing x,y,and z by w.
	return Vector3(transformed.x / transformed.w, transformed.y / transformed.w, transformed.z / transformed.w);
}

Ray CollisionDetection::BuildRayFromMouse(const PerspectiveCamera& cam) {
	Vector2 screenMouse = Window::GetMouse()->GetAbsolutePosition();
	Vector2i screenSize	= Window::GetWindow()->GetScreenSize();

	//We remove the y axis mouse position from height as OpenGL is 'upside down',
	//and thinks the bottom left is the origin, instead of the top left!
	Vector3 nearPos = Vector3(screenMouse.x,
		screenSize.y - screenMouse.y,
		-0.99999f
	);

	//We also don't use exactly 1.0 (the normalised 'end' of the far plane) as this
	//causes the unproject function to go a bit weird. 
	Vector3 farPos = Vector3(screenMouse.x,
		screenSize.y - screenMouse.y,
		0.99999f
	);

	Vector3 a = Unproject(nearPos, cam);
	Vector3 b = Unproject(farPos, cam);
	Vector3 c = b - a;

	c.Normalise();

	return Ray(cam.GetPosition(), c);
}

//http://bookofhook.com/mousepick.pdf
Matrix4 CollisionDetection::GenerateInverseProjection(float aspect, float fov, float nearPlane, float farPlane) {
	Matrix4 m;

	float t = tan(fov*PI_OVER_360);

	float neg_depth = nearPlane - farPlane;

	const float h = 1.0f / t;

	float c = (farPlane + nearPlane) / neg_depth;
	float e = -1.0f;
	float d = 2.0f*(nearPlane*farPlane) / neg_depth;

	m.array[0][0] = aspect / h;
	m.array[1][1] = tan(fov * PI_OVER_360);
	m.array[2][2] = 0.0f;

	m.array[2][3] = 1.0f / d;

	m.array[3][2] = 1.0f / e;
	m.array[3][3] = -c / (d * e);

	return m;
}

/*
And here's how we generate an inverse view matrix. It's pretty much
an exact inversion of the BuildViewMatrix function of the Camera class!
*/
Matrix4 CollisionDetection::GenerateInverseView(const Camera &c) {
	float pitch = c.GetPitch();
	float yaw	= c.GetYaw();
	Vector3 position = c.GetPosition();

	Matrix4 iview =
		Matrix4::Translation(position) *
		Matrix4::Rotation(yaw, Vector3(0, 1, 0)) *
		Matrix4::Rotation(pitch, Vector3(1, 0, 0));

	return iview;
}


/*
If you've read through the Deferred Rendering tutorial you should have a pretty
good idea what this function does. It takes a 2D position, such as the mouse
position, and 'unprojects' it, to generate a 3D world space position for it.

Just as we turn a world space position into a clip space position by multiplying
it by the model, view, and projection matrices, we can turn a clip space
position back to a 3D position by multiply it by the INVERSE of the
view projection matrix (the model matrix has already been assumed to have
'transformed' the 2D point). As has been mentioned a few times, inverting a
matrix is not a nice operation, either to understand or code. But! We can cheat
the inversion process again, just like we do when we create a view matrix using
the camera.

So, to form the inverted matrix, we need the aspect and fov used to create the
projection matrix of our scene, and the camera used to form the view matrix.

*/
Vector3	CollisionDetection::UnprojectScreenPosition(Vector3 position, float aspect, float fov, const PerspectiveCamera& c) {
	//Create our inverted matrix! Note how that to get a correct inverse matrix,
	//the order of matrices used to form it are inverted, too.
	Matrix4 invVP = GenerateInverseView(c) * GenerateInverseProjection(aspect, fov, c.GetNearPlane(), c.GetFarPlane());

	Vector2i screenSize = Window::GetWindow()->GetScreenSize();

	//Our mouse position x and y values are in 0 to screen dimensions range,
	//so we need to turn them into the -1 to 1 axis range of clip space.
	//We can do that by dividing the mouse values by the width and height of the
	//screen (giving us a range of 0.0 to 1.0), multiplying by 2 (0.0 to 2.0)
	//and then subtracting 1 (-1.0 to 1.0).
	Vector4 clipSpace = Vector4(
		(position.x / (float)screenSize.x) * 2.0f - 1.0f,
		(position.y / (float)screenSize.y) * 2.0f - 1.0f,
		(position.z) - 1.0f,
		1.0f
	);

	//Then, we multiply our clipspace coordinate by our inverted matrix
	Vector4 transformed = invVP * clipSpace;

	//our transformed w coordinate is now the 'inverse' perspective divide, so
	//we can reconstruct the final world space by dividing x,y,and z by w.
	return Vector3(transformed.x / transformed.w, transformed.y / transformed.w, transformed.z / transformed.w);
}

