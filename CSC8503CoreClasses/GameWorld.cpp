#include "GameWorld.h"
#include "GameObject.h"
#include "Constraint.h"
#include "CollisionDetection.h"
#include "Camera.h"

using namespace NCL;
using namespace NCL::CSC8503;

GameWorld::GameWorld()
{
	shuffleConstraints = false;
	shuffleObjects = false;
	worldIDCounter = 0;
	worldStateCounter = 0;
}

GameWorld::~GameWorld()
{
}

void GameWorld::Clear()
{
	gameObjects.clear();
	constraints.clear();
	worldIDCounter = 0;
	worldStateCounter = 0;
}

void GameWorld::ClearAndErase()
{
	for (auto &i : gameObjects)
	{
		delete i;
	}
	for (auto &i : constraints)
	{
		delete i;
	}
	Clear();
}

void GameWorld::AddGameObject(GameObject *o)
{
	gameObjectsMutex.lock();
	gameObjects.emplace_back(o);
	o->SetWorldID(worldIDCounter++);
	worldStateCounter++;
	gameObjectsMutex.unlock();
}

void GameWorld::RemoveGameObject(GameObject *o, bool andDelete)
{
	gameObjects.erase(std::remove(gameObjects.begin(), gameObjects.end(), o), gameObjects.end());
	if (andDelete)
	{
		delete o;
	}
	worldStateCounter++;
}

void GameWorld::GetObjectIterators(
	GameObjectIterator &first,
	GameObjectIterator &last) const
{

	first = gameObjects.begin();
	last = gameObjects.end();
}

void GameWorld::OperateOnContents(GameObjectFunc f)
{
	gameObjectsMutex.lock();

	for (GameObject *g : gameObjects)
	{
		f(g);
	}

	gameObjectsMutex.unlock();
}

void GameWorld::UpdateWorld(float dt)
{
	auto rng = std::default_random_engine{};

	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::default_random_engine e(seed);

	// for (GameObject* g : gameObjects) {
	// g->Update(dt);
	//}

	// if (shuffleObjects) {
	//	std::shuffle(gameObjects.begin(), gameObjects.end(), e);
	// }

	// if (shuffleConstraints) {
	//	std::shuffle(constraints.begin(), constraints.end(), e);
	// }
}

bool GameWorld::Raycast(Ray &r, RayCollision &closestCollision, bool closestObject, GameObject *ignoreThis) const
{
	// The simplest raycast just goes through each object and sees if there's a collision
	RayCollision collision;

	for (auto &i : gameObjects)
	{
		if (!i)
			continue;
		if (!i->GetBoundingVolume())
		{ // objects might not be collideable etc...
			continue;
		}
		if (i == ignoreThis)
		{
			continue;
		}
		RayCollision thisCollision;
		if (CollisionDetection::RayIntersection(r, *i, thisCollision))
		{

			if (!closestObject)
			{
				closestCollision = collision;
				closestCollision.node = i;
				return true;
			}
			else
			{
				if (thisCollision.rayDistance < collision.rayDistance)
				{
					thisCollision.node = i;
					collision = thisCollision;
				}
			}
		}
	}
	if (collision.node)
	{
		closestCollision = collision;
		closestCollision.node = collision.node;
		return true;
	}
	return false;
}

bool GameWorld::FindObjectByRaycast(Ray &r, RayCollision &closestCollision, std::string tag, GameObject *ignoreThis) const
{
	RayCollision collision;

	for (auto &i : gameObjects)
	{
		if (i->gettag() != tag || !i->GetBoundingVolume() || i == ignoreThis)
		{
			continue;
		}

		RayCollision thisCollision;
		if (CollisionDetection::RayIntersection(r, *i, thisCollision))
		{
			if (thisCollision.rayDistance < collision.rayDistance)
			{
				thisCollision.node = i;
				collision = thisCollision;
			}
		}
	}
	if (collision.node)
	{
		closestCollision = collision;
		closestCollision.node = collision.node;
		return true;
	}
	return false;
}

/*
Constraint Tutorial Stuff
*/

void GameWorld::AddConstraint(Constraint *c)
{
	constraints.emplace_back(c);
}

void GameWorld::RemoveConstraint(Constraint *c, bool andDelete)
{
	constraints.erase(std::remove(constraints.begin(), constraints.end(), c), constraints.end());
	if (andDelete)
	{
		delete c;
	}
}

void GameWorld::GetConstraintIterators(
	std::vector<Constraint *>::const_iterator &first,
	std::vector<Constraint *>::const_iterator &last) const
{
	first = constraints.begin();
	last = constraints.end();
}