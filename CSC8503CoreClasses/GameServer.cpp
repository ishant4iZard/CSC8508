#include "GameServer.h"
#include "GameWorld.h"
#include "./enet/enet.h"
using namespace NCL;
using namespace CSC8503;

GameServer::GameServer(int onPort, int maxClients)	{
	port		= onPort;
	clientMax	= maxClients;
	clientCount = 0;
	netHandle	= nullptr;
	netPeers = new int[maxClients];
	clearPeerArray();
	Initialise();
}

GameServer::~GameServer()	{
	Shutdown();
}

void GameServer::Shutdown() {
	SendGlobalPacket(BasicNetworkMessages::Shutdown);
	enet_host_destroy(netHandle);
	netHandle = nullptr;
}

bool GameServer::Initialise() 
{
	ENetAddress adress;
	adress.host = ENET_HOST_ANY;
	adress.port = port;

	netHandle = enet_host_create(&adress, clientMax, 1, 0, 0);

	if (!netHandle)
	{
		std::cout << __FUNCTION__ << "failed to create network handle!" << std::endl;
		return false;
	}

	return true;
}

bool GameServer::SendGlobalPacket(int msgID) 
{
	GamePacket packet;
	packet.type = msgID;
	return SendGlobalPacket(packet);
}

bool GameServer::SendGlobalPacket(GamePacket& packet) 
{
	ENetPacket* dataPacket = enet_packet_create(&packet, packet.GetTotalSize(), 0);
	enet_host_broadcast(netHandle, 0, dataPacket);
	return true;
}

bool GameServer::SendSinglePacket(GamePacket& packet, int clientNum)
{
	ENetPacket* dataPacket = enet_packet_create(&packet, packet.GetTotalSize(), 0);
	return true;
}

void GameServer::UpdateServer() 
{
	if (!netHandle) { return; }
	ENetEvent event;
	while (enet_host_service(netHandle, &event, 0) > 0)
	{
		int type = event.type;
		ENetPeer* p = event.peer;
		int peer = p->incomingPeerID;

		if (type == ENetEventType::ENET_EVENT_TYPE_CONNECT)
		{
			std::cout << "Server: New client connected : PeerID " << std::to_string(peer) << std::endl;
			AddPeer(peer);
			DebugNetPeer();
		}
		else if (type == ENetEventType::ENET_EVENT_TYPE_DISCONNECT)
		{
			std::cout << "Server: A client has disconnected : PeerID " << std::to_string(peer) << std::endl;
			DeletPeer(peer);
		}
		else if (type == ENetEventType::ENET_EVENT_TYPE_RECEIVE)
		{
			GamePacket* packet = (GamePacket*)event.packet->data;
			ProcessPacket(packet, peer);
		}
		enet_packet_destroy(event.packet);
	}
}

void GameServer::clearPeerArray()
{
	for (int i = 0; i < clientMax; ++i)
	{
		netPeers[i] = -1;
	}
}

void GameServer::AddPeer(int Peer)
{
	int minVacantPos = clientMax;
	for (int i = 0; i < clientMax; ++i)
	{
		if (netPeers[i] == Peer)
		{
			return;
		}
		if (netPeers[i] == -1)
		{
			minVacantPos = std::min(minVacantPos, i);
		}
	}
	if(minVacantPos < clientMax)
	{
		netPeers[minVacantPos] = Peer;
	}
}

void GameServer::DeletPeer(int Peer)
{
	for (int i = 0; i < clientMax; ++i)
	{
		if (netPeers[i] == Peer)
		{
			netPeers[i] = -1;
			--clientCount;
			return;
		}
	}
}

void GameServer::DebugNetPeer()
{
	for (int i = 0; i < clientMax; ++i)
	{
		std::cout << i << " : PeerID: " << netPeers[i] << std::endl;;
	}
}

bool GameServer::GetNetPeer(int peerNum, int& peerID)
{
	if (peerNum >= clientMax) { return false; }
	if (netPeers[peerNum] == -1) { return false; }
	peerID = netPeers[peerNum];
	return true;
}

void GameServer::SetGameWorld(GameWorld &g) {
	gameWorld = &g;
}