#include "NetworkPlayer.h"
#include "NetworkedGame.h"
#include "PhysicsObject.h"
# define  SQUARE(x) (x * x) 

using namespace NCL;
using namespace CSC8503;

Vector3 Lerp(const Vector3& start, const Vector3& end, float t) {
	t = std::clamp(t, 0.0f, 1.0f);

	float lerpedX = start.x + t * (end.x - start.x);
	float lerpedY = start.y + t * (end.y - start.y);
	float lerpedZ = start.z + t * (end.z - start.z);

	return Vector3(lerpedX, lerpedY, lerpedZ);
}

NetworkPlayer::NetworkPlayer(NetworkedGame* game, int num)	{
	this->game = game;
	playerNum  = num;
	this->settag("Player");
	timeElapsed = 0.0f;
	timeElapsed = projectileReplenishTimer = 0.0f;
	numProjectilesAccumulated = MAX_PROJECTILE_CAPACITY;
	movementSpeed = 10;
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
	this->pointPos = pointPos;
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
		movementSpeed *= -1;
	}

	Vector3 velocity = movementDirection * movementSpeed;
	this->GetPhysicsObject()->SetLinearVelocity(velocity);
}

void NetworkPlayer::RotatePlayer(float dt) {
	// Tried force based circular motion but had to discard due to friction causing problems
	/*float mass = this->GetPhysicsObject()->GetInverseMass();
	float force = SQUARE(ORBIT_SPEED) / (mass * ORBIT_RADIUS);

	Vector3 forceDir = Vector3::Cross(Vector3(0, 1, 0), (this->GetPhysicsObject()->GetLinearVelocity()).Normalised());

	this->GetPhysicsObject()->AddForce(forceDir * (-1 *force));*/

	const static float ORBIT_RADIUS = 75.0; // Ensure that this value matches the distance of the player from the center
	const static float ORBIT_SPEED = 10.0f;
	const static Vector3 ORBIT_CENTER = Vector3(0, 0, 0);

	Vector3 velocityDir = Vector3::Cross(Vector3(0, 1, 0), ORBIT_CENTER - this->GetTransform().GetPosition()).Normalised();
	this->GetPhysicsObject()->SetLinearVelocity(velocityDir * ORBIT_SPEED);
}

void NetworkPlayer::MovePlayerInSquarePattern(float dt) {
	const float CORNER_LOCATION = 75;
	const static vector<Vector3> CORNERS = {
		Vector3{CORNER_LOCATION, 0, -1 * CORNER_LOCATION},
		Vector3{CORNER_LOCATION, 0, CORNER_LOCATION},
		Vector3{-1 * CORNER_LOCATION, 0, -1 * CORNER_LOCATION},
		Vector3{-1 * CORNER_LOCATION, 0, CORNER_LOCATION}
	};
	const static vector<Vector3> MOVEMENT_DIRECTIONS = {
		Vector3{0, 0, 1},
		Vector3{-1, 0, 0},
		Vector3{1, 0, 0},
		Vector3{0, 0, -1}
	};
	const static float BUFFER_DISTANCE = 10;

	for (int i = 0; i < 4; i++) {
		float distanceFromCorner = (CORNERS[i] - GetTransform().GetPosition()).Length();

		if (distanceFromCorner < BUFFER_DISTANCE) {
			movementDirection = MOVEMENT_DIRECTIONS[i];
			break;
		}
	}

	Vector3 velocity = movementDirection * movementSpeed;
	this->GetPhysicsObject()->SetLinearVelocity(velocity);
}

void NetworkPlayer::MovePlayerTowardsCursor(float dt){
	Vector3 playerPos = transform.GetPosition();
	playerPos.y = 5.6;
	transform.SetPosition(playerPos);

	Vector3 movementDirection = (pointPos - transform.GetPosition()).Normalised();
	movementDirection.y = 0;

	Vector3 currentVelocity = this->GetPhysicsObject()->GetLinearVelocity();
	Vector3 targetVelocity = movementDirection * movementSpeed;
	Vector3 velocity = Lerp(currentVelocity, targetVelocity, dt);

	this->GetPhysicsObject()->SetLinearVelocity(velocity);
}

void NetworkPlayer::ReplenishProjectiles(float dt) {
	const static int PROJECTILE_RELOAD_RATE = 1; // 1 projectile per second is replenished

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

	Vector3 fireDir = GetPlayerForwardVector().Normalised();
	Vector3 firePos = transform.GetPosition() + fireDir * 3;
	std::cout << firePos.y;

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

