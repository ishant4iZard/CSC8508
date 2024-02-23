#include "NetworkedGame.h"
#include "NetworkPlayer.h"
#include "NetworkObject.h"
#include "GameServer.h"
#include "GameClient.h"
#include "PushdownMachine.h"
#include "MenuSystem.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"
#include "Projectile.h"
#include "Hole.h"
#include "GravityWell.h"

#define COLLISION_MSG 30

struct MessagePacket : public GamePacket {
	short playerID;
	short messageID;

	MessagePacket() {
		type = Message;
		size = sizeof(short) * 2;
	}
};

NetworkedGame::NetworkedGame()	{
	appState = ApplicationState::GetInstance();

	thisServer = nullptr;
	thisClient = nullptr;

	NetworkBase::Initialise();
	timeToNextPacket  = 0.0f;
	packetsToSnapshot = 0;

	Menu = new MenuSystem(this);

	PlayersList.clear();
	ControledPlayersList.clear();
	for (int i = 0; i < 4; ++i)
	{
		PlayersList.push_back(-1);
		ControledPlayersList.push_back(nullptr);
	}

	audioEngine = new AudioEngine();

	// Looping background music
	backGroundMusic = audioEngine->CreateSound("../../Assets/Audio/abc.mp3", true);
	audioEngine->PlaySound(backGroundMusic, true);

	// Trigger music
	fireSFX = audioEngine->CreateSound("../../Assets/Audio/jump.mp3", false);
}

NetworkedGame::~NetworkedGame()	{
	delete thisServer;
	delete thisClient;
	delete audioEngine;
}

bool NetworkedGame::StartAsServer() {
	if (thisClient != nullptr)
	{
		delete thisClient;
		thisClient = nullptr;
	}

	if (thisServer != nullptr)
	{
		return true;
	}

	thisServer = new GameServer(NetworkBase::GetDefaultPort(), 3);
	if (!thisServer->IsValid())
	{
		delete thisServer;
		thisServer = nullptr;
		return false;
	}

	thisServer->RegisterPacketHandler(Received_State, this);

	appState->SetIsServer(true);

	StartLevel();

	return true;
}

bool NetworkedGame::StartAsClient(char a, char b, char c, char d) {
	if (thisClient != nullptr)
	{
		return true;
	}

	thisClient = new GameClient();
	if (!thisClient->Connect(a, b, c, d, NetworkBase::GetDefaultPort()))
	{
		return false;
	}

	thisClient->RegisterPacketHandler(Message, this);
	thisClient->RegisterPacketHandler(Delta_State, this);
	thisClient->RegisterPacketHandler(Full_State, this);
	thisClient->RegisterPacketHandler(Player_Connected, this);
	thisClient->RegisterPacketHandler(Player_Disconnected, this);
	thisClient->RegisterPacketHandler(Player_Fire, this);
	thisClient->RegisterPacketHandler(Projectile_Deactivate, this);

	//StartLevel();
}

void NetworkedGame::UpdateGame(float dt) {
	Debug::UpdateRenderables(dt);
	Menu->Update(dt);

	if (!appState->GetIsGameOver()) {
		timeToNextPacket -= dt;
		if (timeToNextPacket < 0) {
			if (thisServer) {
				UpdateAsServer(dt);
			}
			else if (thisClient) {
				UpdateAsClient(dt);
			}
			timeToNextPacket += 1.0f / 60.0f; //60hz server/client update
		}

		// Server and Client Receive and process there packet
		if (thisServer) { 
			thisServer->UpdateServer();
			HandleInputAsServer();
			UpdatePlayerState(dt);
			UpdateProjectiles(dt);
			
			gravitywell->PullProjectilesWithinField(ProjectileList);
			physics->Update(dt);
		}
		if (thisClient) { 
			thisClient->UpdateClient(); 
			HandleInputAsClient();
		}

	}
	audioEngine->Update();
	TutorialGame::UpdateGame(dt);
	Debug::UpdateRenderables(dt);
}

void NetworkedGame::UpdatePlayerState(float dt) {
	for (auto i : ControledPlayersList)
	{
		if (i != nullptr)
		{
			i->ReplenishProjectiles(dt);
			i->MovePlayerTowardsCursor(dt);
		}
	}
}

void NetworkedGame::UpdateProjectiles(float dt) {
	for (auto i : ProjectileList) {
		if (i == nullptr) continue;

		i->ReduceTimeLeft(dt);

		if (i->GetTimeLeft() <= 0) {
			i->deactivate();
			DeactivateProjectilePacket newPacket;
			newPacket.NetObjectID = i->GetNetworkObject()->GetNetworkID();
			if (i->GetGame()->GetServer())
			{
				i->GetGame()->GetServer()->SendGlobalPacket(newPacket);
			}
		}
	}
}

void NetworkedGame::UpdateAsServer(float dt) {
	packetsToSnapshot--;
	if (packetsToSnapshot < 0) {
		BroadcastSnapshot(false);
		packetsToSnapshot = 5;
	}
	else {
		BroadcastSnapshot(true);
	}
	UpdateMinimumState();

	ServerUpdatePlayersList();
	CheckPlayerListAndSpawnPlayers();

	if (LocalPlayer)
	{
		Vector3 PointerPos;
		findOSpointerWorldPosition(PointerPos);
		LocalPlayer->SetPlayerYaw(PointerPos);
		if (ServerFired)
		{
			LocalPlayer->isFire = !LocalPlayer->isFire;
			ServerFired = false;
		}
	}
	for (auto i : ControledPlayersList)
	{
		if (i != nullptr)
		{
			if (i->isFire)
			{
				i->Fire();
				i->isFire = false;
			}
		}
	}
}

void NetworkedGame::UpdateAsClient(float dt) {
	ClientPacket newPacket;

	Vector3 PointerWorldPos;
	findOSpointerWorldPosition(PointerWorldPos);
	newPacket.PointerPos = PointerWorldPos;
	newPacket.lastID = GlobalStateID;
	if (ClientFired) { newPacket.bIsFireBtnClicked = true; ClientFired = false; }

	thisClient->SendPacket(newPacket);

	CheckPlayerListAndSpawnPlayers();
}

void NCL::CSC8503::NetworkedGame::HandleInputAsServer()
{
	if (LocalPlayer)
	{
		if (Window::GetMouse()->ButtonPressed(MouseButtons::Type::Left)) { 
			ServerFired = true; 
			audioEngine->PlaySound(fireSFX, false);
		}
	}
}

void NCL::CSC8503::NetworkedGame::HandleInputAsClient()
{
	if (Window::GetMouse()->ButtonPressed(MouseButtons::Type::Left)) { 
		ClientFired = true; 
		audioEngine->PlaySound(fireSFX, false);
	}
}

void NetworkedGame::BroadcastSnapshot(bool deltaFrame) {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;

	world->GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		if (!(*i)->IsActive())
		{
			continue;
		}
		NetworkObject* o = (*i)->GetNetworkObject();
		if (!o) {
			continue;
		}
		//TODO - you'll need some way of determining
		//when a player has sent the server an acknowledgement
		//and store the lastID somewhere. A map between player
		//and an int could work, or it could be part of a 
		//NetworkPlayer struct. 
		int playerState = o->GetLatestNetworkState().stateID;
		GamePacket* newPacket = nullptr;
		if (o->WritePacket(&newPacket, deltaFrame, playerState)) {
			thisServer->SendGlobalPacket(*newPacket);
			delete newPacket;
		}
	}
}

void NetworkedGame::UpdateMinimumState() {
	//Periodically remove old data from the server
	int minID = INT_MAX;
	int maxID = 0; //we could use this to see if a player is lagging behind?

	for (auto i : stateIDs) {
		minID = std::min(minID, i.second);
		maxID = std::max(maxID, i.second);
	}
	//every client has acknowledged reaching at least state minID
	//so we can get rid of any old states!
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;
	world->GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
		NetworkObject* o = (*i)->GetNetworkObject();
		if (!o) {
			continue;
		}
		o->UpdateStateHistory(minID); //clear out old states so they arent taking up memory...
	}
}

void NetworkedGame::ServerUpdatePlayersList()
{
	PlayersList[0] = 0;
	for (int i = 0; i < 3; ++i)
	{
		PlayersList[i + 1] = thisServer->GetClientNetID(i);
	}
	PLayersListPacket plPacket(PlayersList);
	thisServer->SendGlobalPacket(plPacket);
}

void NetworkedGame::CheckPlayerListAndSpawnPlayers()
{
	for (int i = 0; i < 4; ++i)
	{
		if (PlayersList[i] != -1)
		{
			if (ControledPlayersList[i] == nullptr)
			{
				Vector3 pos;
				Vector3 movementDirection;
				switch (i)
				{
				case 0:
					pos = Vector3(0, 5.6, -75);
					movementDirection = Vector3(1, 0, 0);
					break;
				case 1:
					pos = Vector3(75, 5.6, 0);
					movementDirection = Vector3(0, 0, 1);
					break;
				case 2:
					pos = Vector3(0, 5.6, 75);
					movementDirection = Vector3(-1, 0, 0);
					break;
				case 3:
					pos = Vector3(-75, 5.6, 0);
					movementDirection = Vector3(0, 0, -1);
					break;
				}
				ControledPlayersList[i] = AddNetworkPlayerToWorld(pos, i);
				ControledPlayersList[i]->SetMovementDir(movementDirection);
			}
		}
		if (GetLocalPlayerNumber() != -1)
		{
			if (ControledPlayersList[GetLocalPlayerNumber()] != nullptr)
			{
				LocalPlayer = ControledPlayersList[GetLocalPlayerNumber()];
			}
		}
	}
}

NetworkPlayer* NetworkedGame::AddNetworkPlayerToWorld(const Vector3& position, int playerNum)
{
	float meshSize = 2.0f;
	Vector3 volumeSize = Vector3(1.0, 1.6, 1.0);
	float inverseMass = 1.0f / 600000.0f;

	NetworkPlayer* character = new NetworkPlayer(this, playerNum);

	SphereVolume* volume = new SphereVolume(1.6f);

	character->SetBoundingVolume((CollisionVolume*)volume);
	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), charMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));
	character->SetNetworkObject(new NetworkObject(*character, playerNum));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(character);
	networkObjects.insert(std::pair<int, NetworkObject*>(playerNum, character->GetNetworkObject()));

	Vector4 colour;
	switch (playerNum)
	{
	case 0:
		colour = Debug::RED;
		break;
	case 1:
		colour = Debug::BLUE;
		break;
	case 2:
		colour = Debug::YELLOW;
		break;
	case 3:
		colour = Debug::CYAN;
		break;
	}
	character->GetRenderObject()->SetColour(colour);

	return character;
}

void NetworkedGame::findOSpointerWorldPosition(Vector3& position)
{
	Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());
	RayCollision closestCollision;
	if (world->Raycast(ray, closestCollision, true))
	{
		position = closestCollision.collidedAt;
	}
}

void NetworkedGame::SpawnPlayer() {

}

void NetworkedGame::SpawnProjectile(NetworkPlayer* owner, Vector3 firePos, Vector3 fireDir)
{
	Projectile* newBullet = new Projectile(owner, this);

	float radius = 1.0f;
	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	newBullet->SetBoundingVolume((CollisionVolume*)volume);
	newBullet->GetTransform().SetScale(sphereSize).SetPosition(firePos);
	newBullet->SetRenderObject(new RenderObject(&newBullet->GetTransform(), sphereMesh, basicTex, basicShader));
	newBullet->SetPhysicsObject(new PhysicsObject(&newBullet->GetTransform(), newBullet->GetBoundingVolume()));
	newBullet->GetPhysicsObject()->SetInverseMass(Projectile::inverseMass);
	newBullet->GetPhysicsObject()->InitSphereInertia();

	int playerNum = owner->GetPlayerNum();
	Vector4 colour;
	switch (playerNum)
	{
	case 0:
		colour = Debug::RED;
		break;
	case 1:
		colour = Debug::BLUE;
		break;
	case 2:
		colour = Debug::YELLOW;
		break;
	case 3:
		colour = Debug::CYAN;
		break;
	}
	newBullet->GetRenderObject()->SetColour(colour);

	int bulletID = Projectile::CurrentAvailableProjectileID++;
	newBullet->SetNetworkObject(new NetworkObject(*newBullet, bulletID));

	world->AddGameObject(newBullet);
	networkObjects.insert(std::pair<int, NetworkObject*>(bulletID, newBullet->GetNetworkObject()));

	newBullet->GetPhysicsObject()->SetElasticity(1.0f);
	newBullet->GetPhysicsObject()->SetFriction(1.0f);
	newBullet->GetPhysicsObject()->SetFriction(1.0f);

	Vector3 force = fireDir * Projectile::FireForce;
	//newBullet->GetPhysicsObject()->SetLinearVelocity(fireDir);
	newBullet->GetPhysicsObject()->AddForce(force);

	ProjectileList.push_back(newBullet);

	if (thisServer)
	{
		PlayerFirePacket firePacket;
		firePacket.PlayerNum = playerNum;
		firePacket.NetObjectID = bulletID;
		thisServer->SendGlobalPacket(firePacket);
	}
}

void NetworkedGame::OnRep_SpawnProjectile(int PlayerNum, int NetObjectID)
{
	Projectile* newBullet = new Projectile(nullptr, this);

	float radius = 1.0f;
	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	newBullet->SetBoundingVolume((CollisionVolume*)volume);
	newBullet->GetTransform().SetScale(sphereSize);
	newBullet->SetRenderObject(new RenderObject(&newBullet->GetTransform(), sphereMesh, basicTex, basicShader));

	Vector4 colour;
	switch (PlayerNum)
	{
	case 0:
		colour = Debug::RED;
		break;
	case 1:
		colour = Debug::BLUE;
		break;
	case 2:
		colour = Debug::YELLOW;
		break;
	case 3:
		colour = Debug::CYAN;
		break;
	}
	newBullet->GetRenderObject()->SetColour(colour);

	newBullet->SetNetworkObject(new NetworkObject(*newBullet, NetObjectID));

	world->AddGameObject(newBullet);
	networkObjects.insert(std::pair<int, NetworkObject*>(NetObjectID, newBullet->GetNetworkObject()));
}

void NetworkedGame::OnRep_DeactiveProjectile(int objectID)
{
	networkObjects[objectID]->GetGameObject()->deactivate();
}

void NetworkedGame::StartLevel() {
	CheckPlayerListAndSpawnPlayers();
}

void NetworkedGame::ReceivePacket(int type, GamePacket* payload, int source) {
	switch (type)
	{
	case BasicNetworkMessages::Received_State: {
		ClientPacket* realPacket = (ClientPacket*)payload;
		serverProcessClientPacket(realPacket, source);
		break;
	}
	case BasicNetworkMessages::Message: {
		PLayersListPacket* realPacket = (PLayersListPacket*)payload;
		realPacket->GetPlayerList(PlayersList);
		break;
	}
	case BasicNetworkMessages::Full_State: {
		FullPacket* realPacket = (FullPacket*)payload;
		clientProcessFullPacket(realPacket);
		break;
	}
	case BasicNetworkMessages::Delta_State: {
		DeltaPacket* realPacket = (DeltaPacket*)payload;
		clientProcessDeltaPacket(realPacket);
		break;
	}
	case BasicNetworkMessages::Player_Fire: {
		PlayerFirePacket* realPacket = (PlayerFirePacket*)payload;
		OnRep_SpawnProjectile(realPacket->PlayerNum, realPacket->NetObjectID);
		break;
	}
	case BasicNetworkMessages::Projectile_Deactivate: {
		DeactivateProjectilePacket* realPacket = (DeactivateProjectilePacket*)payload;
		OnRep_DeactiveProjectile(realPacket->NetObjectID);
		break;
	}
	}
}

void NetworkedGame::OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b) {
	if (thisServer) { //detected a collision between players!
		MessagePacket newPacket;
		newPacket.messageID = COLLISION_MSG;
		newPacket.playerID  = a->GetPlayerNum();

		thisClient->SendPacket(newPacket);

		newPacket.playerID = b->GetPlayerNum();
		thisClient->SendPacket(newPacket);
	}
}

int NetworkedGame::GetConnectedClientsNum()
{
	if (thisServer)
	{
		return thisServer->GetConnectedClientsNum();
	}
	return 0;
}

/** -1 means no connet */
int NetworkedGame::GetLocalPlayerNumber() const
{
	if (thisServer)
	{
		return 0;
	}
	if (thisClient)
	{
		for (int i = 1; i < 4; ++i)
		{
			if (PlayersList[i] == thisClient->GetClientNetID())
			{
				return i;
			}
		}
	}
	return -1;
}

int NetworkedGame::GetPlayerNumberFromNetID(const int NetID) const
{
	for (int i = 1; i < 4; ++i)
	{
		if (PlayersList[i] == NetID)
		{
			return i;
		}
	}
	return -1;
}

int NetworkedGame::GetClientState()
{
	if (thisClient)
	{
		return thisClient->GetClientState();
	}
	return -1;
}

bool NetworkedGame::serverProcessClientPacket(ClientPacket* cp, int source)
{
	int playerID = GetPlayerNumberFromNetID(source);
	if (playerID != -1)
	{
		NetworkPlayer* thePlayer = ControledPlayersList[playerID];
		thePlayer->SetPlayerYaw(cp->PointerPos);
		if (cp->bIsFireBtnClicked) { thePlayer->isFire = !thePlayer->isFire; }

		auto i = stateIDs.find(playerID);
		if (i == stateIDs.end()) { stateIDs.insert(std::pair<int, int>(playerID, cp->lastID)); }
		else { i->second = cp->lastID; }

		return true;
	}
	return false;
}

bool NetworkedGame::clientProcessFullPacket(FullPacket* fp)
{
	auto itr = networkObjects.find(fp->objectID);
	if (itr == networkObjects.end()) {
		std::cout << "Client Num: " << GetLocalPlayerNumber() << " can't find netObject" << std::endl;
		return false;
	}
	itr->second->ReadPacket(*fp);
	if (fp->fullState.stateID > GlobalStateID) { GlobalStateID = fp->fullState.stateID; }
	return true;
}

bool NetworkedGame::clientProcessDeltaPacket(DeltaPacket* dp)
{
	auto itr = networkObjects.find(dp->objectID);
	if (itr == networkObjects.end()) {
		std::cout << "Client Num" << GetLocalPlayerNumber() << "can't find netObject" << std::endl;
		return false;
	}
	itr->second->ReadPacket(*dp);
	return true;
}
