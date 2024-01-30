#include "GameClient.h"
#include "./enet/enet.h"
using namespace NCL;
using namespace CSC8503;

GameClient::GameClient()	{
	netHandle = enet_host_create(nullptr, 1, 1, 0, 0);
	peerID = -1;
}

GameClient::~GameClient()	{
	enet_host_destroy(netHandle);
}

bool GameClient::Connect(uint8_t a, uint8_t b, uint8_t c, uint8_t d, int portNum) 
{
	ENetAddress adress;
	adress.port = portNum;
	adress.host = (d << 24) | (c << 16) | (b << 8) | (a);

	netPeer = enet_host_connect(netHandle, &adress, 2, 0);

	return netPeer != nullptr;
}

void GameClient::UpdateClient() 
{
	if (netHandle == nullptr)
	{
		return;
	}
	//Handle all incoming packets
	ENetEvent event;
	while (enet_host_service(netHandle, &event, 0) > 0)
	{
		if (event.type == ENET_EVENT_TYPE_CONNECT)
		{
			peerID = netPeer->outgoingPeerID;
			std::cout << "Connected to sever! peerID : " <<std::to_string(peerID)<< std::endl;
		}
		else if (event.type == ENET_EVENT_TYPE_RECEIVE)
		{
			//std::cout << "Client: Packet Received... peerID : " << std::to_string(peerID) << std::endl;
			GamePacket* packet = (GamePacket*)event.packet->data;
			ProcessPacket(packet);
		}
		enet_packet_destroy(event.packet);
	}
}

void GameClient::SendPacket(GamePacket& payload) 
{
	ENetPacket* dataPacket = enet_packet_create(&payload, payload.GetTotalSize(), 0);
	enet_peer_send(netPeer, 0, dataPacket);
}
