#include "BouncePad.h"
#include "NetworkedGame.h"
#include "NetworkPlayer.h"
#include "PhysicsObject.h"

using namespace NCL;
using namespace CSC8503;

BouncePad::BouncePad(NetworkPlayer* inOwner, NetworkedGame* inGame, Mesh* inMesh, Texture* inTex, Shader* inShader)
{
    SetBoundingVolume((CollisionVolume*)new OBBVolume(this->scale / 2.0f));

    GetTransform()
        .SetPosition(Vector3(0, 0, 0))
        .SetScale(this->scale);

   SetRenderObject(new RenderObject(&GetTransform(), inMesh, inTex, inShader));
   SetPhysicsObject(new PhysicsObject(&GetTransform(), GetBoundingVolume()));

    GetPhysicsObject()->SetInverseMass(1);
}
BouncePad::BouncePad(Mesh* inMesh, Texture* inTex, Shader* inShader)
{
    SetBoundingVolume((CollisionVolume*)new OBBVolume(this->scale / 2.0f));

    GetTransform()
        .SetPosition(Vector3(0, 0, 0))
        .SetScale(this->scale);

    SetRenderObject(new RenderObject(&GetTransform(), inMesh, inTex, inShader));
    SetPhysicsObject(new PhysicsObject(&GetTransform(), GetBoundingVolume()));

    GetPhysicsObject()->SetInverseMass(0.5f);
}

BouncePad::~BouncePad()
{
}