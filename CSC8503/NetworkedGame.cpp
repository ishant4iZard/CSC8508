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
#include "NavigationGrid.h"
#include "Particles.h"
#include <cmath>

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
	PlayersNameList.clear();
	PlayersScoreList.clear();
	for (int i = 0; i < 4; ++i)
	{
		PlayersList.push_back(-1);
		ControledPlayersList.push_back(nullptr);
		PlayersNameList.push_back(std::string(" "));
		PlayersScoreList.push_back(-1);
	}

	audioEngine = new AudioEngine();

	// Looping background music
	backGroundMusic = audioEngine->CreateSound("../../Assets/Audio/abc.mp3", true);
	audioEngine->PlaySound(backGroundMusic, true);

	// Trigger music
	fireSFX = audioEngine->CreateSound("../../Assets/Audio/jump.mp3", false);

	debugHUD = new DebugHUD();
	poolPTR = new ThreadPool(2);
}

NetworkedGame::~NetworkedGame()	{
	delete thisServer;
	delete thisClient;
	delete audioEngine;
	delete poolPTR;
	delete debugHUD;
}

void NetworkedGame::InitializeProjectilePool(NetworkPlayer* player)
{
	if (!player) return;

	for (int i = 0; i < PROJECTILE_POOL_SIZE; i++) {
		Projectile* newBullet = new Projectile(player, this);

		float radius = 1.0f;
		Vector3 sphereSize = Vector3(radius, radius, radius);
		SphereVolume* volume = new SphereVolume(radius);
		newBullet->SetBoundingVolume((CollisionVolume*)volume);
		newBullet->GetTransform().SetScale(sphereSize).SetPosition(Vector3(0, 0, 0));
		newBullet->SetRenderObject(new RenderObject(&newBullet->GetTransform(), sphereMesh, basicTex, basicShader));
		newBullet->SetPhysicsObject(new PhysicsObject(&newBullet->GetTransform(), newBullet->GetBoundingVolume()));
		newBullet->GetPhysicsObject()->SetInverseMass(Projectile::inverseMass);
		newBullet->GetPhysicsObject()->InitSphereInertia();
		newBullet->GetPhysicsObject()->SetElasticity(1.0f);
		newBullet->GetPhysicsObject()->SetFriction(1.0f);
		newBullet->GetPhysicsObject()->SetFriction(1.0f);

		int playerNum = player->GetPlayerNum();
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
		networkObjects.insert(std::pair<int, NetworkObject*>(bulletID, newBullet->GetNetworkObject()));

		world->AddGameObject(newBullet);
		ProjectileList.push_back(newBullet);

		newBullet->deactivate();
	}
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
	thisServer->RegisterPacketHandler(Client_Hello, this);

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
	thisClient->SetLocalPlayerIndex(localPlayerIndex);
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
	thisClient->RegisterPacketHandler(Round_Over, this);
	thisClient->RegisterPacketHandler(Player_Score, this);
	thisClient->RegisterPacketHandler(Player_BulletNum, this);
	thisClient->RegisterPacketHandler(PowerUp_Spawn, this);

	StartLevel();
}

void NetworkedGame::DestroyServer()
{
	if (!thisServer) return;
	EndLevel();
	delete thisServer;
	thisServer = nullptr;
}

void NetworkedGame::DestroyClient()
{
	if (!thisClient) return;
	EndLevel();
	delete thisClient;
	thisClient = nullptr;
}

void NetworkedGame::UpdateGame(float dt) {
	std::optional<time_point<high_resolution_clock>> frameStartTime;
	if(isDebuHUDActive)
		frameStartTime = high_resolution_clock::now();

	Debug::UpdateRenderables(dt);
	if (thisServer && !appState->GetIsGameOver() && !appState->GetIsGamePaused()) {
		UpdatePhysics = true;
		//PhysicsUpdate(dt);
		timer += dt;
	}

	/*NonPhysicsUpdate(dt);
	PhysicsUpdate(dt);*/

	//std::thread physicsUpdateThread(&NetworkedGame::PhysicsUpdate, this,dt);
	//std::thread nonPhysicsUpdateThread(&NetworkedGame::NonPhysicsUpdate, this, dt);
	if (poolPTR) {
		poolPTR->enqueue([this, dt]() {this->PhysicsUpdate(dt); });
		poolPTR->enqueue([this, dt]() {this->NonPhysicsUpdate(dt); });
	}

	for (auto AIStateObject : AIStateObjectList) {
		AIStateObject->DetectProjectiles(ProjectileList);
		AIStateObject->Update(dt);
	}

	/*if (AIStateObject) {
		AIStateObject->DetectProjectiles(ProjectileList);
		AIStateObject->Update(dt);
	}*/

	Menu->Update(dt);
	if (appState->GetIsGamePaused()) {
		//audioEngine->Pause();
	}
	else {
		//audioEngine->UnPause();
	}
	audioEngine->Update();

	TutorialGame::UpdateGame(dt);

	std::optional<time_point<high_resolution_clock>> frameEndTime;
	if (isDebuHUDActive)
		frameEndTime = high_resolution_clock::now();
		
	//nonPhysicsUpdateThread.join();
	//physicsUpdateThread.join();

	if (Window::GetKeyboard()->KeyHeld(KeyCodes::Type::I))
	{
		isDebuHUDActive = true;

		if (!frameStartTime.has_value() || !frameEndTime.has_value()) return;

		auto duration = duration_cast<microseconds>(frameEndTime.value() - frameStartTime.value());
		debugHUD->DrawDebugHUD({
			dt,
			duration.count(),
			physics->GetNumberOfCollisions(),
			world->GetNumberOfObjects()
		});

	}
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
		/*if (i->IsActive()) {
			i->Particles->Update(dt, i, 2, i->GetTransform().GetScale()/2);
			i->Particles->Draw();
		}*/ //Particle system test 

		if (i->GetTimeLeft() <= 0) {
			i->deactivate();
			DeactivateProjectilePacket newPacket;
			newPacket.NetObjectID = i->GetNetworkObject()->GetNetworkID();
			if (this->GetServer())
			{
				this->GetServer()->SendGlobalPacket(newPacket);
			}
		}
	}
}

void NetworkedGame::UpdatePowerUpSpawnTimer(float dt)
{
	if (appState->GetIsGameOver() || appState->GetIsGamePaused()) return;

	powerUpSpawnTimer += dt;
	if (powerUpSpawnTimer >= POWER_UP_SPAWN_TIME
		&& activePowerUpCount <= MAX_POWER_UP_COUNT
		)
	{
		powerUpSpawnTimer = 0.0f;
		SpawnPowerUp(PowerUpSpawnNetID);
		SpawnPowerUpPacket spPacket(PowerUpSpawnNetID);
		if (thisServer) { thisServer->SendGlobalPacket(spPacket); }
		++PowerUpSpawnNetID;
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
	ServerUpdateScoreList();
	ServerUpdateBulletNumList();

	/** Server network footstep */
	TotalSizeOutgoingPacket = thisServer->OutgoingPacketSize;
	TotalSizeIncomingPakcet = thisServer->IncomingPacketSize;
	thisServer->OutgoingPacketSize = 0;
	thisServer->IncomingPacketSize = 0;

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

	if (thisClient->GetClientState() == CLIENT_STATE_DISCONNECTED)
	{
		std::cout << "Client Disconnected!";
		EventEmitter::EmitEvent(EventType::ROUND_OVER);
	}

	/** Client network footprint */
	TotalSizeOutgoingPacket = thisClient->OutgoingPacketSize;
	TotalSizeIncomingPakcet = thisClient->IncomingPacketSize;
	thisClient->OutgoingPacketSize = 0;
	thisClient->IncomingPacketSize = 0;

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
	plPacket.timer = this->timer;
	thisServer->SendGlobalPacket(plPacket);
}

void NetworkedGame::ServerUpdateScoreList()
{
	for (int i = 0; i < 4; ++i)
	{
		if (ControledPlayersList[i] != nullptr)
		{
			PlayersScoreList[i] = ControledPlayersList[i]->GetScore();
		}
	}
	PlayersScorePacket psPacket(PlayersScoreList);
	psPacket.PowerUpState = CurrentPowerUpType;
	thisServer->SendGlobalPacket(psPacket);
}

void NetworkedGame::ServerUpdateBulletNumList()
{
	for (int i = 0; i < 4; ++i)
	{
		if (ControledPlayersList[i] != nullptr)
		{
			PlayersBulletNumList[i] = ControledPlayersList[i]->GetNumBullets();
		}
	}
	PlayersBulletNumPacket bnPacket(PlayersBulletNumList);
	thisServer->SendGlobalPacket(bnPacket);
}

void NetworkedGame::CheckPlayerListAndSpawnPlayers()
{
	for (int i = 0; i < 4; ++i)
	{
		if (PlayersList[i] != -1)
		{
			//world->gameObjectsMutex.lock();
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
				/*world->gameObjectsMutex.unlock();*/

				ControledPlayersList[i] = AddNetworkPlayerToWorld(pos, i);
				ControledPlayersList[i] ->SetMovementDir(movementDirection);
				//InitializeProjectilePool(ControledPlayersList[i]);
			}
			/*else
			{
				world->gameObjectsMutex.unlock();
			}*/
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
	// This was for projectile pooling
	//Projectile* newBullet = nullptr;
	//for (auto i : ProjectileList) {
	//	if (i->IsActive()) continue;
	//	newBullet = i;
	//}

	//if (!newBullet) return;
	//newBullet->activate();
	//newBullet->ResetTime();

	//newBullet->GetTransform().SetPosition(firePos);
	//newBullet->GetPhysicsObject()->ClearForces();
	//newBullet->GetPhysicsObject()->SetLinearVelocity(Vector3(0, 0, 0));
	//newBullet->GetPhysicsObject()->SetAngularVelocity(Vector3(0, 0, 0));
	//Vector3 force = fireDir * Projectile::FireForce;
	//newBullet->GetPhysicsObject()->ApplyLinearImpulse(force);
	//
	//int playerNum = owner->GetPlayerNum();

	//if (thisServer)
	//{
	//	PlayerFirePacket firePacket;
	//	firePacket.PlayerNum = playerNum;
	//	firePacket.NetObjectID = newBullet->GetNetworkObject()->GetNetworkID();
	//	thisServer->SendGlobalPacket(firePacket);
	//}

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

	//newBullet->Particles = new GenerateParticle((OGLShader*)particleShader, (OGLTexture*)basicTex, 10);

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
	newBullet->GetPhysicsObject()->SetFriction(0.3f);
	//newBullet->GetPhysicsObject()->SetFriction(1.0f);

	Vector3 force = fireDir * Projectile::FireForce;
	//newBullet->GetPhysicsObject()->SetLinearVelocity(fireDir);
	newBullet->GetPhysicsObject()->ApplyLinearImpulse(force);

	ProjectileList.push_back(newBullet);

	if (thisServer)
	{
		PlayerFirePacket firePacket;
		firePacket.PlayerNum = playerNum;
		firePacket.NetObjectID = bulletID;
		firePacket.NetObjectID = newBullet->GetNetworkObject()->GetNetworkID();
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
	if (networkObjects.find(objectID) == networkObjects.cend()) return;
	networkObjects[objectID]->GetGameObject()->deactivate();
}

void NetworkedGame::StartLevel() {
	InitWorld();
	SpawnDataDrivenLevel(GameLevelNumber::LEVEL_1);
	InitTeleporters();
	PlayersList.clear();
	ControledPlayersList.clear();
	PlayersScoreList.clear();
	PlayersBulletNumList.clear();
	Projectile::CurrentAvailableProjectileID = 1000;
	PowerUpSpawnNetID = POWER_UP_INIT_NETID;
	AIInitialID = 3000;
	if (poolPTR) {
		delete poolPTR;
		poolPTR = nullptr;
	}
	for (int i = 0; i < 4; ++i)
	{
		PlayersList.push_back(-1);
		ControledPlayersList.push_back(nullptr);
		//PlayersNameList.push_back(std::string(" "));
		PlayersScoreList.push_back(-1);
		PlayersBulletNumList.push_back(-1);
	}
	ProjectileList.clear();
	
	//PlayersNameList.clear();
	CheckPlayerListAndSpawnPlayers();
	SpawnAI();

	physics->createStaticTree();//this needs to be at the end of all initiations
	appState->SetIsGameOver(false);
	appState->SetIsGamePaused(false);
	poolPTR = new ThreadPool(2);

}

void NetworkedGame::EndLevel()
{

	if (poolPTR) {
		delete poolPTR;
		poolPTR = nullptr;
	}
	world->ClearAndErase();
	physics->Clear();
	ControledPlayersList.clear();
	networkObjects.clear();
	gravitywell.clear();

	AIStateObjectList.clear();
	InitCamera();

}

void NetworkedGame::ReceivePacket(int type, GamePacket* payload, int source) {
	switch (type)
	{
	case BasicNetworkMessages::Received_State: {
		ClientPacket* realPacket = (ClientPacket*)payload;
		serverProcessClientPacket(realPacket, source);
		break;
	}
	case BasicNetworkMessages::Client_Hello: {
		if (thisServer)
		{
			ClientHelloPacket* realPacket = (ClientHelloPacket*)payload;
			if (isDevMode)
			{
				thisServer->AddConnectClient(realPacket->PeerID);
			}
			else
			{
				thisServer->SetClientList(realPacket->PlayerListIndex, realPacket->PeerID);
			}
		}
		break;
	}
	case BasicNetworkMessages::Message: {
		PLayersListPacket* realPacket = (PLayersListPacket*)payload;
		realPacket->GetPlayerList(PlayersList);
		this->timer = realPacket->timer;
		break;
	}
	case BasicNetworkMessages::Player_Score: {
		PlayersScorePacket* realPacket = (PlayersScorePacket*)payload;
		realPacket->GetPlayerScore(PlayersScoreList);
		CurrentPowerUpType = powerUpType(realPacket->PowerUpState);
		break;
	}
	case BasicNetworkMessages::Player_BulletNum: {
		PlayersBulletNumPacket* realPacket = (PlayersBulletNumPacket*)payload;
		realPacket->GetPlayerBulletNum(PlayersBulletNumList);
		break;
	}
	case BasicNetworkMessages::PowerUp_Spawn: {
		SpawnPowerUpPacket* realPacket = (SpawnPowerUpPacket*)payload;
		SpawnPowerUp(realPacket->NetObjectID);
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
	case BasicNetworkMessages::Round_Over: {
		ServerRoundOverPacket* realPacket = (ServerRoundOverPacket*)payload;
		std::cout << "Client Round over msg Received!\n";
		if (realPacket->isRoundOver) { 
			EventEmitter::EmitEvent(EventType::ROUND_OVER); 
		}
		break;
	}
	}
}

void NetworkedGame::OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b) {
	
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

void NetworkedGame::SetPlayerNameByIndexInList(const std::string& Name, int Index)
{
	if (Index > 3) return;

	PlayersNameList[Index] = Name;
}

void NetworkedGame::ServerSendRoundOverMsg()
{
	if (thisServer)
	{
		std::cout << "Server Send Round over Msg!\n";
		ServerRoundOverPacket newPacket;
		newPacket.isRoundOver = true;
		thisServer->SendGlobalPacket(newPacket);
	}
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

AiStatemachineObject* NetworkedGame::AddAiStateObjectToWorld(const Vector3& position) {
	NavigationGrid* navGrid = new NavigationGrid(world);
	AiStatemachineObject* AIStateObject = new AiStatemachineObject(world, navGrid);

	float radius = 4.0f;
	SphereVolume* volume = new SphereVolume(radius);
	AIStateObject->SetBoundingVolume((CollisionVolume*)volume);
	AIStateObject->GetTransform()
		.SetScale(Vector3(radius, radius, radius))
		.SetPosition(Vector3(position.x, 5.6, position.z));

	AIStateObject->SetRenderObject(new RenderObject(&AIStateObject->GetTransform(), sphereMesh, nullptr, basicShader));
	AIStateObject->SetPhysicsObject(new PhysicsObject(&AIStateObject->GetTransform(), AIStateObject->GetBoundingVolume()));
	AIStateObject->SetNetworkObject(new NetworkObject(*AIStateObject, AIInitialID));
	networkObjects.insert(std::pair<int, NetworkObject*>(AIInitialID, AIStateObject->GetNetworkObject()));
	++AIInitialID;

	AIStateObject->GetPhysicsObject()->SetInverseMass(1 / 10000000.0f);
	AIStateObject->GetPhysicsObject()->SetElasticity(0.000002);
	AIStateObject->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(AIStateObject);
	AIStateObjectList.push_back(AIStateObject);

	return AIStateObject;
}

void NetworkedGame::PhysicsUpdate(float dt)
{
	if (UpdatePhysics/* && !appState->GetIsGamePaused()*/) {
		PhysicsMutex.lock();
		physics->Update(dt);
		CurrentPowerUpType = physics->GetCurrentPowerUpState();
		UpdatePhysics = false;
		PhysicsMutex.unlock();
	}
}

void NetworkedGame::NonPhysicsUpdate(float dt)
{
	//Debug::UpdateRenderables(dt);
	NonPhysicsMutex.lock();

	if (!appState->GetIsGameOver()&& !appState->GetIsGamePaused()) {
		timeToNextPacket -= dt;
		if (timeToNextPacket <= 0) {
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

			if (!appState->GetIsGamePaused()) {
				HandleInputAsServer();
				UpdatePlayerState(dt);
				UpdateProjectiles(dt);
				UpdatePowerUpSpawnTimer(dt);
				for (auto i : gravitywell)
					i->PullProjectilesWithinField(ProjectileList);
			}
			//physics->Update(dt);
			//UpdatePhysics = true;
		}
		if (thisClient) {
			thisClient->UpdateClient(dt);
			if (!appState->GetIsGamePaused()) {
				HandleInputAsClient();
			}
		}
	}

	
	NonPhysicsMutex.unlock();


}

void NetworkedGame::SpawnAI() {
	// TODO : Read from csv and load ais
	AddAiStateObjectToWorld(Vector3(60, 5.6, 60));
	AddAiStateObjectToWorld(Vector3(-60, 5.6, 60));
	AddAiStateObjectToWorld(Vector3(-60, 5.6, -60));
	AddAiStateObjectToWorld(Vector3(60, 5.6, -60));
}

void NetworkedGame::SpawnPowerUp(int NetID)
{
	PowerUp* NewPowerUp = InitPowerup();
	NewPowerUp->SetNetworkObject(new NetworkObject(*NewPowerUp, NetID));
	networkObjects.insert(std::pair<int, NetworkObject*>(NetID, NewPowerUp->GetNetworkObject()));
}

int NetworkedGame::GetLocalPlayerBulletNum() const
{
	if (!LocalPlayer) return 0;
	return LocalPlayer->GetNumBullets();
}

float NetworkedGame::GetOutgoingPacketSizePerSecond() const
{
	float result = TotalSizeOutgoingPacket * 60.0f / 1024.0f;
	return result;
}

float NetworkedGame::GetInComingPacketSizePerSecond() const
{
	float result = TotalSizeIncomingPakcet * 60.0f / 1024.0f;
	return result;
}


//void NetworkedGame::DetectProjectiles(GameObject* gameObject) {
//
//	Vector3 objectPosition = gameObject->GetTransform().GetPosition();
//	Vector3 objectForward = gameObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);
//	Ray ray(objectPosition, objectForward);
//
//	const int numRays = 30;
//	const float angleIncrement = 2 * 3.14 / numRays;
//
//	vector<Ray> rays;
//	for (int i = 0; i < numRays; i++) {
//		float angle = angleIncrement * i;
//		float x = cos(angle); 
//		float z = sin(angle); 
//
//		Vector3 dir = Vector3(x, 0, z);
//		rays.push_back(Ray(objectPosition, dir));
//		Debug::DrawLine(objectPosition, dir * 100, Debug::RED);
//	}
//
//	RayCollision closestCollision;
//	//	closestCollision.rayDistance = 100.0f;
//
//	float shortDistance = 999999;
//	for (auto ray : rays) {
//		if (world->Raycast(ray, closestCollision, true, gameObject)) {
//			GameObject* ObjectHited = (GameObject*)closestCollision.node;
//
//			if (ObjectHited)
//			{
//				if (ObjectHited->gettag() == "Projectile"&&closestCollision.rayDistance<10.0f)
//				{
//					//std::cout << "Object detected";
//					float distance = (ObjectHited->GetTransform().GetPosition() - objectPosition).Length();
//
//					if (distance < shortDistance) {
//						projectileToChase = ObjectHited;
//						shortDistance = distance;
//					}
//					//ObjectHited->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * 100000, closestCollision.collidedAt);	
//				}
//			}	
//		}
//	}
//
//	ChaseClosestProjectile(0.2);
//}
//
//void NetworkedGame::ChaseClosestProjectile(float dt) {
//	
//	if (projectileToChase == nullptr) return;
//
//	Vector3 movementDirection = projectileToChase->GetTransform().GetPosition() - testStateObject->GetTransform().GetPosition();
//	movementDirection.Normalised();
//
//	testStateObject->GetPhysicsObject()->SetLinearVelocity(movementDirection * 10 * 0.2);
//}