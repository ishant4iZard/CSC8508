#include "NetworkPlayer.h"
#include "NetworkedGame.h"
#include "PhysicsObject.h"
# define  SQUARE(x) (x * x) 

using namespace NCL;
using namespace CSC8503;

NetworkPlayer::NetworkPlayer(NetworkedGame* game, int num)	{
	this->game = game;
	playerNum  = num;
	this->settag("Player");
	timeElapsed = 0.0f;
	timeElapsed = projectileReplenishTimer = 0.0f;
	numProjectilesAccumulated = MAX_PROJECTILE_CAPACITY;
	Oscillationspeed = 10;
}

NetworkPlayer::~NetworkPlayer()	{

}

void NetworkPlayer::OnCollisionBegin(GameObject* otherObject) {
	if (game) {
		if (dynamic_cast<NetworkPlayer*>(otherObject))
		{
			game->OnPlayerCollision(this, (NetworkPlayer*)otherObject);
		}
	}
}

Quaternion GenerateOrientation(const Vector3& axis, float angle)
{
	float halfAngle = angle / 2;
	float s = std::sin(halfAngle);
	float w = std::cos(halfAngle);
	float x = axis.x * s;
	float y = axis.y * s;
	float z = axis.z * s;
	return Quaternion(x, y, z, w);
}

void NetworkPlayer::SetPlayerYaw(const Vector3& pointPos)
{
	Quaternion orientation;
	Vector3 pos = transform.GetPosition();
	Vector3 targetForwardVec = (pointPos - pos);
	targetForwardVec.y = 0;
	targetForwardVec = targetForwardVec.Normalised();

	Vector3 forward = Vector3(0, 0, -1);

	float cosTheta = Vector3::Dot(forward, targetForwardVec);
	Vector3 rotationAxis;
	float angle;
	if (cosTheta < -1 + 0.001f)
	{
		rotationAxis = Vector3::Cross(Vector3(0, 0, 1), forward);
		if (rotationAxis.Length() < 0.01)
		{
			rotationAxis = Vector3::Cross(Vector3(1, 0, 0), forward);
		}
		rotationAxis = rotationAxis.Normalised();
		angle = 3.1415926f;
	}
	else
	{
		rotationAxis = Vector3::Cross(forward, targetForwardVec);
		rotationAxis = rotationAxis.Normalised();
		angle = std::acos(cosTheta);
	}
	orientation = GenerateOrientation(rotationAxis, angle);

	transform.SetOrientation(orientation);
}

void NetworkPlayer::OscillatePlayer(float dt) {
	timeElapsed += dt;
	
	if (timeElapsed >= 2.0f) {
		timeElapsed = 0.0f;
		Oscillationspeed *= -1;
	}

	Vector3 velocity = movementDirection * Oscillationspeed;
	this->GetPhysicsObject()->SetLinearVelocity(velocity);
}

void NetworkPlayer::RotatePlayer(float dt) {
	// Tried force based circular motion but had to discard due to friction causing problems
	/*float mass = this->GetPhysicsObject()->GetInverseMass();
	float force = SQUARE(ORBIT_SPEED) / (mass * ORBIT_RADIUS);

	Vector3 forceDir = Vector3::Cross(Vector3(0, 1, 0), (this->GetPhysicsObject()->GetLinearVelocity()).Normalised());

	this->GetPhysicsObject()->AddForce(forceDir * (-1 *force));*/

	Vector3 velocityDir = Vector3::Cross(Vector3(0, 1, 0), ORBIT_CENTER - this->GetTransform().GetPosition()).Normalised();
	this->GetPhysicsObject()->SetLinearVelocity(velocityDir * ORBIT_SPEED);
}

void NetworkPlayer::ReplenishProjectiles(float dt) {
	projectileReplenishTimer += dt;

	if (projectileReplenishTimer > PROJECTILE_RELOAD_RATE)
	{
		numProjectilesAccumulated = (numProjectilesAccumulated + 1) % MAX_PROJECTILE_CAPACITY;
		projectileReplenishTimer = 0.0f;
	}
}

void NetworkPlayer::Fire()
{
	if (numProjectilesAccumulated <= 0) return;
	numProjectilesAccumulated--;

	Vector3 fireDir = GetPlayerForwardVector();
	Vector3 firePos = transform.GetPosition() + fireDir * 10;
	game->SpawnProjectile(this, firePos, fireDir);
	//std::cout << "player " << playerNum << " fired!" << std::endl;
}

Vector3 NetworkPlayer::GetPlayerForwardVector()
{
	Vector3 vec = Vector3(0, 0, -1);
	vec = transform.GetOrientation() * vec;
	vec = vec.Normalised();
	return vec;
}

