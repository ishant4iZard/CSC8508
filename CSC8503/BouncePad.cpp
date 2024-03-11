#include "BouncePad.h"
#include "TutorialGame.h"
#include "PhysicsObject.h"

using namespace NCL;
using namespace CSC8503;

BouncePad::BouncePad( Mesh* inMesh, Texture* inTex, Shader* inShader)
{
    AABBVolume* volume = new AABBVolume(this->scale / 2.0f, false, true);
    SetBoundingVolume((CollisionVolume*)volume);

    GetTransform()
        .SetPosition(Vector3(0, 0, 0))
        .SetScale(this->scale);

   SetRenderObject(new RenderObject(&GetTransform(), inMesh, inTex, inShader));
   SetPhysicsObject(new PhysicsObject(&GetTransform(), (CollisionVolume*)volume));

    GetPhysicsObject()->SetInverseMass(0);
    GetPhysicsObject()->InitCubeInertia();
    GetPhysicsObject()->SetElasticity(1.05f);
}

//BouncePad::BouncePad(Mesh* inMesh, Texture* inTex, Shader* inShader)
//{
//    AABBVolume* volume = new AABBVolume(this->scale / 2.0f, false, true);
//    SetBoundingVolume((CollisionVolume*)volume);
//
//    GetTransform()
//        .SetPosition(Vector3(0, 0, 0))
//        .SetScale(this->scale);
//
//    SetRenderObject(new RenderObject(&GetTransform(), inMesh, inTex, inShader));
//    SetPhysicsObject(new PhysicsObject(&GetTransform(), (CollisionVolume*)volume));
//
//    GetPhysicsObject()->SetInverseMass(0);
//    GetPhysicsObject()->InitCubeInertia();
//    GetPhysicsObject()->SetElasticity(1.05f);
//}

BouncePad::~BouncePad()
{
}