
#include "NetworkedGame.h"
#include "NetworkPlayer.h"
#include "NetworkObject.h"
#include "GameServer.h"
#include "GameClient.h"
#include "PushdownMachine.h"

#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"

#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "StateGameObject.h"
#include "Bullet.h"

#include "StateMachine.h"
#include "State.h"


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
	thisServer = nullptr;
	thisClient = nullptr;

	NetworkBase::Initialise();
	timeToNextPacket  = 0.0f;
	packetsToSnapshot = 0;

	MenuSystem = new PushdownMachine(new MainMenu());
	MenuSystem->SetGame(this);
	isGameover = false;
	isRoundstart = false;

	PlayersList.clear();
	for (int i = 0; i < 4; ++i)
	{
		PlayersList.push_back(-1);
	}
}

NetworkedGame::~NetworkedGame()	{
	delete thisServer;
	delete thisClient;
	delete MenuSystem;
}

bool NetworkedGame::StartAsServer() {
	if (thisServer != nullptr)
	{
		return true;
	}
	thisServer = new GameServer(NetworkBase::GetDefaultPort(), 3);

	thisServer->RegisterPacketHandler(Received_State, this);
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

	thisClient->RegisterPacketHandler(Delta_State, this);
	thisClient->RegisterPacketHandler(Full_State, this);
	//thisClient->RegisterPacketHandler(Player_Connected, this);
	//thisClient->RegisterPacketHandler(Player_Disconnected, this);
	thisClient->RegisterPacketHandler(Message, this);
	thisClient->RegisterPacketHandler(Round_State, this);
	thisClient->RegisterPacketHandler(Player_State, this);
	thisClient->RegisterPacketHandler(bullet_state, this);
	return true;
}

void NetworkedGame::UpdateGame(float dt) {
	if (!MenuSystem->Update(dt))
	{
		isGameover = true;
	}

	timeToNextPacket -= dt;
	if (timeToNextPacket < 0) {
		// only the round start, the Server and Client can send the message about the round game;
		if (isRoundstart)
		{
			if (thisServer) {
				UpdateAsServer(dt);
			}
			else if (thisClient) {
				UpdateAsClient(dt);
			}
		}
		if (thisServer) { 
			ServerUpdatePlayerList(); 
			ServerSendRoundState();
			ServerSendPlayerState();
		}

		timeToNextPacket += 1.0f / 60.0f; //20hz server/client update
	}

	// Server and Client Receive and process there packet
	if (thisServer) { thisServer->UpdateServer(); }
	if (thisClient) { thisClient->UpdateClient(); }

	if (isRoundstart)
	{
		updateRoundTime(dt);
		UpdateGamePlayerInput(dt);
		UpdateScoreTable();
		LevelDelayOver(dt);

		//Test View
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::Q))
		{
			if (lockedObject == nullptr) {
				lockedObject = localPlayer; 
				Window::GetWindow()->ShowOSPointer(true);
			}
			else if (lockedObject == lockedObject) { 
				lockedObject = nullptr;
				Window::GetWindow()->ShowOSPointer(false);
			}
		}

		if (!inSelectionMode) {
			world->GetMainCamera().UpdateCamera(dt);
		}
		if (lockedObject != nullptr) {
			Vector3 objPos = lockedObject->GetTransform().GetPosition();
			Vector3 camPos = objPos + lockedOffset;

			Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

			Matrix4 modelMat = temp.Inverse();

			Quaternion q(modelMat);
			Vector3 angles = q.ToEuler(); //nearly there now!

			world->GetMainCamera().SetPosition(camPos);
			world->GetMainCamera().SetPitch(angles.x);
			world->GetMainCamera().SetYaw(angles.y);
		}

		//UpdateKeys();
		world->UpdateWorld(dt);
		renderer->Update(dt);
		// Only Server need to do physics calculation
		if (thisServer) { physics->Update(dt); }
	}

	renderer->Render();
	Debug::UpdateRenderables(dt);
}

int NetworkedGame::GetClientPlayerNum()
{
	if (thisClient)
	{
		for (int i = 1; i < 4; ++i)
		{
			if (PlayersList[i] == thisClient->GetPeerID())
			{
				return i;
			}
		}
	}
	return -1;
}

int NetworkedGame::GetClientPlayerNum(int peerID)
{
	for (int i = 1; i < 4; ++i)
	{
		if (PlayersList[i] == peerID)
		{
			return i;
		}
	}
	return -1;
}

void NetworkedGame::InitWorld()
{
	world->ClearAndErase();
	physics->Clear();

	InitDefaultFloor();
	InitMapWall();

	AddNetOBBCube();

	BridgeConstraintTest();
	//testStateObject = AddStateObjectToWorld(Vector3(0, 10, 0));

	SpawnPlayer();
	SpawnAI();
	SpawnItem();
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
}

void NetworkedGame::UpdateAsClient(float dt) {
	ClientPacket newPacket;

	Vector3 PointerPos;
	findOSpointerWorldPosition(PointerPos);
	newPacket.PointerPos = PointerPos;
	newPacket.btnStates[0] = Window::GetKeyboard()->KeyHeld(KeyCodes::W) ? 1 : 0;
	newPacket.btnStates[1] = Window::GetKeyboard()->KeyHeld(KeyCodes::S) ? 1 : 0;
	newPacket.btnStates[2] = Window::GetKeyboard()->KeyHeld(KeyCodes::D) ? 1 : 0;
	newPacket.btnStates[3] = Window::GetKeyboard()->KeyHeld(KeyCodes::A) ? 1 : 0;
	newPacket.btnStates[4] = Window::GetKeyboard()->KeyPressed(KeyCodes::SHIFT) ? 1 : 0;
	newPacket.btnStates[5] = Window::GetMouse()->ButtonPressed(MouseButtons::Type::Left) ? 1 : 0;
	newPacket.lastID = GlobalStateID;

	//if (Window::GetKeyboard()->KeyPressed(KeyCodes::SPACE)) {
	//	//fire button pressed!
	//	//newPacket.buttonstates[0] = 1;
	//	newPacket.lastID = 0; //You'll need to work this out somehow...
	//}
	thisClient->SendPacket(newPacket);
}

void NetworkedGame::ServerUpdatePlayerList()
{
	if (thisServer == nullptr)
	{
		return;
	}
	PlayersList[0] = 0;
	int peerID;
	for (int i = 0; i < 3; ++i)
	{
		if (thisServer->GetNetPeer(i, peerID))
		{
			PlayersList[i + 1] = peerID;
		}
		else
		{
			PlayersList[i + 1] = -1;
		}
	}
	PLayerListPacket plist(PlayersList);
	thisServer->SendGlobalPacket(plist);
}

void NetworkedGame::UpdateGamePlayerInput(float dt)
{
	if (thisServer)
	{
		if (localPlayer != nullptr)
		{
			NetworkPlayer* realPlayer = (NetworkPlayer*)localPlayer;

			Vector3 PointerPos;
			findOSpointerWorldPosition(PointerPos);
			realPlayer->SetPlayerYaw(PointerPos);
			bool btnVal[4] = { false, false, false ,false };
			if (Window::GetKeyboard()->KeyHeld(KeyCodes::W)) { btnVal[Up] = true; }
			if (Window::GetKeyboard()->KeyHeld(KeyCodes::S)) { btnVal[Down] = true; }
			if (Window::GetKeyboard()->KeyHeld(KeyCodes::D)) { btnVal[Right] = true; }
			if (Window::GetKeyboard()->KeyHeld(KeyCodes::A)) { btnVal[Left] = true; }
			realPlayer->MovePlayer(btnVal[Up], btnVal[Down], btnVal[Right], btnVal[Left]);
			if (Window::GetKeyboard()->KeyPressed(KeyCodes::SHIFT)) { realPlayer->PlayerSprint(); }
			if (Window::GetMouse()->ButtonPressed(MouseButtons::Type::Left)) { realPlayer->PlayerFire(); }
			realPlayer->GameTick(dt);
		}
		for (int i = 1; i < 4; ++i)
		{
			if (serverPlayers[i] != nullptr)
			{
				NetworkPlayer* thePlayer = (NetworkPlayer*)(serverPlayers[i]);
				bool btnVal[4] = { false, false, false ,false };
				if (thePlayer->GetBtnState(Up) == 1) { btnVal[Up] = true; }
				if (thePlayer->GetBtnState(Down) == 1) { btnVal[Down] = true; }
				if (thePlayer->GetBtnState(Right) == 1) { btnVal[Right] = true; }
				if (thePlayer->GetBtnState(Left) == 1) { btnVal[Left] = true; }
				thePlayer->MovePlayer(btnVal[Up], btnVal[Down], btnVal[Right], btnVal[Left]);
				thePlayer->GameTick(dt);
			}
		}
		bullet::UpdateBulletList();

		for (auto i : geese)
		{
			i->getStateMachine()->Update(dt);
		}

		undercoverAgent->ExcuteBehavioursTree(dt);
	}

	float yaw = treasure->GetTransform().GetOrientation().ToEuler().y + 45 * dt;
	treasure->GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(0, yaw, 0));
}

void NetworkedGame::UpdateScoreTable()
{
	if (!isServer()) { return; }
	for (int i = 0; i < 4; ++i)
	{
		if (serverPlayers[i] != nullptr)
		{
			NetworkPlayer* thisPlayer = (NetworkPlayer*)(serverPlayers[i]);
			scoreTable[i] = thisPlayer->getPlayerScore();
		}
	}
}

void NetworkedGame::BroadcastSnapshot(bool deltaFrame) {
	std::vector<GameObject*>::const_iterator first;
	std::vector<GameObject*>::const_iterator last;

	world->GetObjectIterators(first, last);

	for (auto i = first; i != last; ++i) {
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

void NetworkedGame::ServerSendRoundState()
{
	RoundStatePacket state;
	state.isRoundStart = this->isRoundstart;
	if (isRoundstart)
	{
		for (int i = 0; i < 4; ++i) { state.scoretable[i] = scoreTable[i]; }
		state.isTreasureExist = treasure->getOwner() == nullptr ? true : false;
	}
	thisServer->SendGlobalPacket(state);
}

void NetworkedGame::ServerSendPlayerState()
{
	if (isRoundstart)
	{
		for (int i = 1; i < 4; ++i)
		{
			if (serverPlayers[i] != nullptr)
			{
				PlayerStatePacket state;
				NetworkPlayer* thisPlayer = (NetworkPlayer*)(serverPlayers[i]);
				state.playerNum = i;
				state.state[0] = (int)thisPlayer->getSprintCD();
				state.state[1] = (int)thisPlayer->getFireCD();
				state.state[2] = thisPlayer->getHaveTreasure() ? 1 : 0;
				thisServer->SendGlobalPacket(state);
			}
		}
	}
}

void NetworkedGame::updateRoundTime(float dt)
{
	RoundTime -= dt;
	if (RoundTime < 0.0)
	{
		if (thisServer)
		{
			roundDelayOver = true;
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

bool NetworkedGame::serverProcessCP(ClientPacket* cp, int source)
{
	int playerID = GetClientPlayerNum(source);
	if (playerID != -1)
	{
		NetworkPlayer* thePlayer = (NetworkPlayer*)(serverPlayers[playerID]);
		thePlayer->SetPlayerYaw(cp->PointerPos);
		if (cp->btnStates[Sprint] == 1) { thePlayer->PlayerSprint(); }
		if (cp->btnStates[Fire] == 1)   { thePlayer->PlayerFire(); }
		thePlayer->SetBtnState(Up, cp->btnStates[Up]);
		thePlayer->SetBtnState(Down, cp->btnStates[Down]);
		thePlayer->SetBtnState(Right, cp->btnStates[Right]);
		thePlayer->SetBtnState(Left, cp->btnStates[Left]);

		auto i = stateIDs.find(playerID);
		if (i == stateIDs.end()) { stateIDs.insert(std::pair<int, int>(playerID, cp->lastID)); }
		else { i->second = cp->lastID; }
		return true;
	}
	return false;
}

bool NetworkedGame::clientProcessRp(RoundStatePacket* rp)
{
	if (rp->isRoundStart != isRoundStart())
	{
		switch (rp->isRoundStart)
		{
		case true:
			this->StartLevel();
			break;
		case false:
			this->LevelOver();
			break;
		}
	}
	if (rp->isRoundStart) {
		for (int i = 0; i < 4; ++i) { scoreTable[i] = rp->scoretable[i]; }
		if (rp->isTreasureExist) {
			treasure->GetRenderObject()->SetColour(Vector4(1, 0.8, 0, 1));
		}
		else {
			treasure->GetRenderObject()->SetColour(Vector4(0, 0, 0, 0));
		}
	}
	return true;
}

bool NetworkedGame::clientProcessFp(FullPacket* fp)
{
	auto itr = networkObjects.find(fp->objectID);
	if (itr == networkObjects.end()) {
		std::cout << "Client Num" << GetClientPlayerNum() << "can't find netObject" << std::endl;
		return false;
	}
	itr->second->ReadPacket(*fp);
	if (fp->fullState.stateID > GlobalStateID) { GlobalStateID = fp->fullState.stateID; }
	return true;
}

bool NetworkedGame::clientProcessDp(DeltaPacket* dp)
{
	auto itr = networkObjects.find(dp->objectID);
	if (itr == networkObjects.end()) {
		std::cout << "Client Num" << GetClientPlayerNum() << "can't find netObject" << std::endl;
		return false;
	}
	itr->second->ReadPacket(*dp);
	return true;
}

bool NetworkedGame::clientProcessPp(PlayerStatePacket* Pp)
{
	if (Pp->playerNum == GetClientPlayerNum())
	{
		if (localPlayer)
		{
			NetworkPlayer* thisPlayer = (NetworkPlayer*)localPlayer;
			thisPlayer->setSprintCD(Pp->state[0]);
			thisPlayer->setFireCD(Pp->state[1]);
			thisPlayer->setPlayerSocer(scoreTable[Pp->playerNum]);
			thisPlayer->setHaveTreasure(Pp->state[2] == 1 ? true : false);
			return true;
		}
	}
	return false;
}

bool NetworkedGame::clientProcessBp(BulletStatePacket* Bp)
{
	if (Bp->bulletInfo[1] == 1)
	{
		ClientSpawnBullet(Bp->bulletInfo[0], Bp->bulletID);
	}
	else if (Bp->bulletInfo[1] == 0)
	{
		auto i = networkObjects.find(Bp->bulletID);
		if (i != networkObjects.end())
		{
			GameObject* bulletPtr = i->second->getGameObjectPtr();
			//bullet::bulletsDiscard.push_back((bullet*)bulletPtr);
			RemoveObjectFromWorld(bulletPtr, true);
		}
	}
	return false;
}

void NetworkedGame::findOSpointerWorldPosition(Vector3& position)
{
	Ray ray = CollisionDetection::BuildRayFromMouse(world->GetMainCamera());
	RayCollision closestCollision;
	if (world->Raycast(ray, closestCollision, true))
	{
		position = closestCollision.collidedAt;
		//Debug::DrawLine(ray.GetPosition(), position, Debug::GREEN, 3.0f);
	}
}

GameObject* NetworkedGame::AddNetPlayerToWorld(const Vector3& position, int playerNum)
{
	float meshSize = 2.0f;
	Vector3 volumeSize = Vector3(1.0, 1.6, 1.0);
	float inverseMass = 1.0f / 60.0f;

	NetworkPlayer* character = new NetworkPlayer(this, playerNum);
	AABBVolume* volume = new AABBVolume(volumeSize);

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

void NetworkedGame::AddNetOBBCube()
{
	Vector3 p = Vector3(-200, 10, -200);
	int x = 6;
	int y = 6;
	for (int i = 0; i < 100; ++i)
	{
		do 
		{
			x = rand() % 50;
			y = rand() % 50;
		} while (x > 9 && x < 40 && y > 9 && y < 40);

		Vector3 pos = p + Vector3(x * 8 + 4, 0, y * 8 + 4);
		
		GameObject* obb = AddOBBCubeToWorld(pos, Vector3(1, 1, 1));
		obb->SetNetworkObject(new NetworkObject(*obb, 500 + i));
		networkObjects.insert(std::pair<int, NetworkObject*>(500 + i, obb->GetNetworkObject()));
	}
}

void NetworkedGame::SpawnPlayer() {
	serverPlayers.clear();
	for (int i = 0; i < 4; ++i)
	{
		if (GetPlayerPeerID(i) != -1)
		{
			Vector3 pos;
			switch (i)
			{
			case 0:
				pos = Vector3(-188, 3, -188);
				break;
			case 1:
				pos = Vector3( 188, 3, -188);
				break;
			case 2:
				pos = Vector3(-188, 3,  188);
				break;
			case 3:
				pos = Vector3( 188, 3,  188);
				break;
			}
			serverPlayers.push_back(AddNetPlayerToWorld(pos, i));
		}
		else
		{
			serverPlayers.push_back(nullptr);
		}
	}
	if (isServer())
	{
		localPlayer = serverPlayers[0];
	}
	else if (isClient())
	{
		localPlayer = serverPlayers[GetClientPlayerNum()];
	}
	LockCameraToObject(localPlayer);
}

void NetworkedGame::SpawnAI()		
{
	geese.clear();
	for (int num = 6; num < 8; ++num)
	{
		NetworkPlayer* goose = new NetworkPlayer(this, num, 1);
		geese.push_back(goose);

		float meshSize = 2.0f;
		Vector3 volumeSize = Vector3(1.0, 1.6, 1.0);
		float inverseMass = 1.0f / 60.0f;
		Vector3 pos = num == 6 ? Vector3(-108, 1, -108) : Vector3(108, 1, 108);
		goose->setPatrolIndex(num - 5);

		AABBVolume* volume = new AABBVolume(volumeSize);
		goose->SetBoundingVolume((CollisionVolume*)volume);
		goose->GetTransform()
			.SetScale(Vector3(meshSize, meshSize, meshSize))
			.SetPosition(pos);

		goose->SetRenderObject(new RenderObject(&goose->GetTransform(), gooseMesh, nullptr, basicShader));
		goose->SetPhysicsObject(new PhysicsObject(&goose->GetTransform(), goose->GetBoundingVolume()));
		goose->SetNetworkObject(new NetworkObject(*goose, num));

		goose->GetPhysicsObject()->SetInverseMass(inverseMass);
		goose->GetPhysicsObject()->InitCubeInertia();
		goose->GetRenderObject()->SetColour(Vector4(0.588, 0.3, 0.08, 1));
		world->AddGameObject(goose);
		networkObjects.insert(std::pair<int, NetworkObject*>(num, goose->GetNetworkObject()));
	}

	undercoverAgent = new NetworkPlayer(this, 8, 2);
	float meshSize = 3.0f;
	Vector3 volumeSize = Vector3(1.0, 2.0, 1.0);
	float inverseMass = 1.0f / 60.0f;
	AABBVolume* volume = new AABBVolume(volumeSize);
	undercoverAgent->SetBoundingVolume((CollisionVolume*)volume);
	undercoverAgent->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(Vector3(0,0,0));

	undercoverAgent->SetRenderObject(new RenderObject(&undercoverAgent->GetTransform(), enemyMesh, nullptr, basicShader));
	undercoverAgent->SetPhysicsObject(new PhysicsObject(&undercoverAgent->GetTransform(), undercoverAgent->GetBoundingVolume()));
	undercoverAgent->SetNetworkObject(new NetworkObject(*undercoverAgent, 8));

	undercoverAgent->GetPhysicsObject()->SetInverseMass(inverseMass);
	undercoverAgent->GetPhysicsObject()->InitCubeInertia();
	undercoverAgent->GetRenderObject()->SetColour(Vector4(0.588, 0.3, 0.08, 1));
	world->AddGameObject(undercoverAgent);
	networkObjects.insert(std::pair<int, NetworkObject*>(8, undercoverAgent->GetNetworkObject()));
}

void NetworkedGame::SpawnItem()
{
	for (int i = 0; i < 4; ++i)
	{
		if (PlayersList[i] != -1)
		{
			Item* spwanPoint = new Item(
				[&](NetworkPlayer* hitplayer, Item* thisItem)->void
				{
					if (hitplayer == thisItem->getOwner())
					{
						if (hitplayer->getHaveTreasure()) 
						{ 
							if (!roundDelayOver)
							{
								hitplayer->addPlayerScore(bringBackTreasure);
								roundDelayOver = true;
							} 
						}
						//Debug::DrawLine(hitplayer->GetTransform().GetPosition(), hitplayer->GetTransform().GetPosition() + Vector3(0, 50, 0), Debug::MAGENTA);
					}
				}
			);
			if (serverPlayers[i]) { spwanPoint->setOwner((NetworkPlayer*)(serverPlayers[i])); }
			Vector4 colour;
			Vector3 pos;
			switch (i)
			{
			case 0:
				colour = Debug::RED;
				pos = Vector3(-188, 0.5, -188);
				break;
			case 1:
				colour = Debug::BLUE;
				pos = Vector3( 188, 0.5, -188);
				break;
			case 2:
				colour = Debug::YELLOW;
				pos = Vector3(-188, 0.5,  188);
				break;
			case 3:
				colour = Debug::CYAN;
				pos = Vector3( 188, 0.5,  188);
				break;
			}
			colour.w = 0.5;
			Vector3 halfsize = Vector3(12, 0.5, 12);

			AABBVolume* volume = new AABBVolume(halfsize);
			spwanPoint->SetBoundingVolume((CollisionVolume*)volume);
			spwanPoint->GetTransform()
				.SetScale(halfsize * 2)
				.SetPosition(pos);
			spwanPoint->SetRenderObject(new RenderObject(&spwanPoint->GetTransform(), cubeMesh, basicTex, basicShader));
			spwanPoint->GetRenderObject()->SetColour(colour);
			spwanPoint->SetPhysicsObject(nullptr);
			world->AddGameObject(spwanPoint);
		}
	}
	
	treasure = new Item(
		[&](NetworkPlayer* hitplayer, Item* thisItem)->void
		{
			if (thisItem->getOwner() == nullptr)
			{
				hitplayer->setHaveTreasure(true);
				thisItem->setOwner(hitplayer);
				thisItem->GetRenderObject()->SetColour(Vector4(0, 0, 0, 0));
			}
		}
	);
	float radius = 0.4f;
	SphereVolume* volume = new SphereVolume(2.0f);
	treasure->SetBoundingVolume((CollisionVolume*)volume);
	treasure->GetTransform().SetScale(Vector3(radius, radius, radius)).SetPosition(Vector3(4, 2, -4));
	treasure->SetRenderObject(new RenderObject(&treasure->GetTransform(), coinMesh, basicTex, basicShader));
	treasure->GetRenderObject()->SetColour(Vector4(1, 0.8, 0, 1));
	treasure->SetPhysicsObject(nullptr);
	world->AddGameObject(treasure);
}

void NetworkedGame::SpawnBullet(NetworkPlayer* o, Vector3 firePos, Vector3 fireDir)
{
	bullet* newbullet = new bullet(o);
	bullet::bulletList.push_back(newbullet);

	float radius = 1.0f;
	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	newbullet->SetBoundingVolume((CollisionVolume*)volume);

	newbullet->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(firePos);

	newbullet->SetRenderObject(new RenderObject(&newbullet->GetTransform(), sphereMesh, basicTex, basicShader));
	newbullet->SetPhysicsObject(new PhysicsObject(&newbullet->GetTransform(), newbullet->GetBoundingVolume()));

	newbullet->GetPhysicsObject()->SetInverseMass(bullet::inverseMass);
	newbullet->GetPhysicsObject()->InitSphereInertia();

	int playerNum = o->GetPlayerNum();
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
	newbullet->GetRenderObject()->SetColour(colour);

	int bulletID = ++bullet::bulletID;
	newbullet->SetNetworkObject(new NetworkObject(*newbullet, bulletID));

	world->AddGameObject(newbullet);
	networkObjects.insert(std::pair<int, NetworkObject*>(bulletID, newbullet->GetNetworkObject()));

	Vector3 force = fireDir * bullet::FireForce;
	newbullet->GetPhysicsObject()->AddForce(force);
	newbullet->GetPhysicsObject()->SetLinearVelocity(fireDir);

	BulletStatePacket bulletP;
	bulletP.bulletID = bulletID;
	bulletP.bulletInfo[0] = playerNum;
	bulletP.bulletInfo[1] = 1;
	thisServer->SendGlobalPacket(bulletP);
}

void NetworkedGame::SeverSendBulletDelPckt(int bulletID)
{
	if (thisServer)
	{
		BulletStatePacket state;
		state.bulletID = bulletID;
		state.bulletInfo[0] = 0;
		state.bulletInfo[1] = 0;
		thisServer->SendGlobalPacket(state);
	}
}

void NetworkedGame::ClientSpawnBullet(int playNum, int bulletID)
{
	bullet* newbullet = new bullet();

	float radius = 1.0f;
	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	newbullet->SetBoundingVolume((CollisionVolume*)volume);

	newbullet->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(Vector3(0, -100, 0));

	newbullet->SetRenderObject(new RenderObject(&newbullet->GetTransform(), sphereMesh, basicTex, basicShader));

	Vector4 colour;
	switch (playNum)
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
	newbullet->GetRenderObject()->SetColour(colour);

	newbullet->SetNetworkObject(new NetworkObject(*newbullet, bulletID));

	world->AddGameObject(newbullet);
	networkObjects.insert(std::pair<int, NetworkObject*>(bulletID, newbullet->GetNetworkObject()));
}

void NetworkedGame::RemoveObjectFromWorld(GameObject* o, bool andDelete)
{
	if (o->GetNetworkObject() != nullptr)
	{
		networkObjects.erase(o->GetNetworkObject()->getNetWorkID());
	}
	world->RemoveGameObject(o, andDelete);
}

void NetworkedGame::StartLevel() {
	InitWorld();
	//AddWallToWorld(Vector3(-188, 4, -188), Vector3(4, 4, 4));
	physics->UseGravity(true);

	scoreTable.clear();
	for (int i = 0; i < 4; ++i) { scoreTable.push_back(0); }
	//Change Round State
	GlobalStateID = -1;
	RoundTime = 600.0f;
	roundDelayOver = false;
	delayTime = 0.6f;
	isRoundstart = true;
}

void NetworkedGame::LevelOver()
{
	isRoundstart = false;
}

void NetworkedGame::LevelDelayOver(float dt)
{
	if (roundDelayOver)
	{
		delayTime -= dt;
		if (delayTime < 0.0f)
		{
			LevelOver();
		}
	}
}

void NetworkedGame::ReceivePacket(int type, GamePacket* payload, int source) {
	switch (type)
	{
	case BasicNetworkMessages::Message:{
		PLayerListPacket* realPacket = (PLayerListPacket*)payload;
		realPacket->GetPlayerList(PlayersList);
		break;
	}
	case BasicNetworkMessages::Round_State: {
		RoundStatePacket* realPacket = (RoundStatePacket*)payload;
		clientProcessRp(realPacket);
		break;
	}
	case BasicNetworkMessages::Full_State: {
		FullPacket* realPacket = (FullPacket*)payload;
		clientProcessFp(realPacket);
		break;
	}
	case BasicNetworkMessages::Delta_State: {
		DeltaPacket* realPacket = (DeltaPacket*)payload;
		clientProcessDp(realPacket);
		break;
	}
	case BasicNetworkMessages::Received_State: {
		ClientPacket* realPacket = (ClientPacket*)payload;
		serverProcessCP(realPacket, source);
		break;
	}
	case BasicNetworkMessages::Player_State: {
		PlayerStatePacket* realPacket = (PlayerStatePacket*)payload;
		clientProcessPp(realPacket);
		break;
	}
	case BasicNetworkMessages::bullet_state: {
		BulletStatePacket* realPacket = (BulletStatePacket*)payload;
		clientProcessBp(realPacket);
		break;
	}
	}
}

void NetworkedGame::OnPlayerCollision(NetworkPlayer* a, NetworkPlayer* b) {
	if (thisServer) { //detected a collision between players!
		/*MessagePacket newPacket;
		newPacket.messageID = COLLISION_MSG;
		newPacket.playerID  = a->GetPlayerNum();

		thisClient->SendPacket(newPacket);

		newPacket.playerID = b->GetPlayerNum();
		thisClient->SendPacket(newPacket);*/
	}
}

std::string NetworkedGame::getRoundTimeToString()
{
	int minute = (int)RoundTime / 60;
	int second = (int)RoundTime % 60;
	std::string m = std::to_string(minute);
	std::string s = std::to_string(second);
	if (minute < 10) { m = "0" + m; }
	if (second < 10) { s = "0" + s; }
	std::string time = "Round Time " + m + ":" + s;
	return time;
}

std::string NetworkedGame::getLocalPLayerScoreToString()
{
	if (localPlayer)
	{
		NetworkPlayer* p = (NetworkPlayer*)localPlayer;
		int score = p->getPlayerScore();
		std::string s = std::to_string(score);
		if (score < 100) { s = " " + s; }
		if (score < 10)  { s = " " + s; }
		s = "YourScore: " + s;
		return s;
	}
	return std::string();
}

std::string NetworkedGame::getLocalPlayerSprintCDToString()
{
	if (localPlayer)
	{
		NetworkPlayer* p = (NetworkPlayer*)localPlayer;
		float cd = p->getSprintCD();
		int t = (int)cd + 1;
		std::string sct;
		if (cd > 0) { sct = std::to_string(t) + "s"; }
		else if (cd == 0) { sct = "Ready"; }
		sct = "Sprint:" + sct;
		return sct;
	}
	return std::string();
}

std::string NetworkedGame::getLocalPlayerHasTreasure()
{
	if (localPlayer)
	{
		NetworkPlayer* p = (NetworkPlayer*)localPlayer;
		if (p->getHaveTreasure())
		{
			return std::string("You Got the Treasure!!!");
		}
	}
	return std::string();
}

std::string NetworkedGame::getLocalPlayerFireCDToString()
{
	if (localPlayer)
	{
		NetworkPlayer* p = (NetworkPlayer*)localPlayer;
		float cd = p->getFireCD();
		int t = (int)cd + 1;
		std::string fct;
		if (cd > 0) { fct = std::to_string(t) + "s"; }
		else if (cd == 0) { fct = "Ready"; }
		fct = "Fire:" + fct;
		return fct;
	}
	return std::string();
}

std::string NetworkedGame::getPlayersScore(int ID)
{
	if (PlayersList[ID] != -1)
	{
		int score = scoreTable[ID];
		std::string s = std::to_string(score);
		if (score < 100) { s = " " + s; }
		if (score < 10) { s = " " + s; }
		std::string blank = "    ";
		s = "Player " + std::to_string(ID + 1) + blank + "Score:" + s + blank +"ping : 1";
		return s;
	}
	return std::string();
}

std::string NetworkedGame::getSelfPlace()
{
	int playerNum = 0;
	if (thisServer) { playerNum = 0; }
	if (thisClient) { playerNum = GetClientPlayerNum(); }

	int place = 1;
	for (int i = 0; i < 4; ++i)
	{
		if (i == playerNum) { continue; }
		if (scoreTable[i] > scoreTable[playerNum])
		{
			++place;
		}
	}
	std::string selfPlace;
	if (place == 1) { 
		selfPlace = "Congratulation!! You are the WINNER!!"; 
		return selfPlace;
	}
	std::string num;
	switch (place) {
	case 2:
		num = "second";
		break;
	case 3:
		num = "third";
		break;
	case 4:
		num = "fourth";
		break;
	}
	selfPlace = "Sorry!! You won the " + num + " place...";
	return selfPlace;
}

void NetworkedGame::PrintPlayerFinalScore(int place, Vector2 pos)
{
	std::string p;
	switch (place)
	{
	case 1:
		p = "1st";
		break;
	case 2:
		p = "2nd";
		break;
	case 3:
		p = "3rd";
		break;
	case 4:
		p = "4th";
		break;
	}

	vector<std::pair<int,int>> placeList;

	for (int i = 0; i < 4; ++i)
	{
		if (PlayersList[i] != -1)
		{
			bool hasInsert = false;
			for (auto itr = placeList.begin(); itr != placeList.end(); ++itr)
			{
				if (itr->second < scoreTable[i])
				{
					placeList.insert(itr, std::pair<int, int>(i, scoreTable[i]));
					hasInsert = true;
					break;
				}
			}
			if (!hasInsert) {
				placeList.push_back(std::pair<int, int>(i, scoreTable[i]));
			}
		}
	}

	if (place > placeList.size())
	{
		return;
	}
	int playerNum = placeList[place - 1].first;
	int score = placeList[place - 1].second;

	std::string blank = "       ";
	std::string s = p + blank + "Player " + std::to_string(playerNum + 1) + blank + "Score:" + std::to_string(score);

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

	Debug::Print(s, pos, colour);
}


