#include "RenderObjectMaleGuard.h"

#include <string>
#include "../OpenGLRendering/OGLTexture.h"
#include "Maths.h"
using namespace std;

using namespace NCL;
using namespace Rendering;
using namespace CSC8503;

RenderObjectMaleGuard::RenderObjectMaleGuard(Transform* parentTransform, Mesh* mesh, Texture* tex, Shader* shader, MeshMaterial* material)
	:RenderObject(parentTransform, mesh, tex, shader), maleGuardMaterial(material) {

	maleGuardMaterial = material;
	
}

RenderObjectMaleGuard::RenderObjectMaleGuard(Transform* parentTransform, Mesh* mesh, Texture* tex, Shader* shader)
	:RenderObject(parentTransform, mesh, tex, shader) {

	//maleGuardMaterial = material;
	orientation = Quaternion();
}

RenderObjectMaleGuard::~RenderObjectMaleGuard() {

}

void RenderObjectMaleGuard::SetMaleGuardPosition(Vector3 inPos) {
	maleGuardPos = inPos;
}

Vector3 RenderObjectMaleGuard::GetMaleGuardPosition() {
	return maleGuardPos;
}

void RenderObjectMaleGuard::SetMaleGuardScale(Vector3 inScale) {
	maleGuardScale = inScale;
}

Vector3 RenderObjectMaleGuard::GetMaleGuardScale() {
	return maleGuardScale;
}

void RenderObjectMaleGuard::SetMaleGuardRotation(Vector4 inRotation) {
	rotateDegrees = inRotation.x;
	maleGuardRotation = Vector3(inRotation.y, inRotation.z, inRotation.w);
}

Vector4 RenderObjectMaleGuard::GetMaleGuardRotation() {
	return Vector4(rotateDegrees, maleGuardRotation.x, maleGuardRotation.y, maleGuardRotation.z);
}

void RenderObjectMaleGuard::SetMaleGuardQuaternion(Quaternion inOrientation) {
	orientation = inOrientation;
}

Quaternion RenderObjectMaleGuard::GetMaleGuardQuaternion() {
	return orientation;
}
