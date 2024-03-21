#include "GameClient.h"
#include "./enet/enet.h"
#include "NetworkObject.h"


using namespace NCL;
using namespace CSC8503;

GameClient::GameClient()	{
	netHandle = enet_host_create(nullptr, 1, 1, 0, 0);
	CurrentConnetNetID = -1;
	clientState = EClientState::CLIENT_STATE_NONE;
}

GameClient::~GameClient()	{
	enet_host_destroy(netHandle);
}

bool GameClient::Connect(uint8_t a, uint8_t b, uint8_t c, uint8_t d, int portNum) {
	ENetAddress address;
	address.port = portNum;
	address.host = (d << 24) | (c << 16) | (b << 8) | (a);
	
	netPeer = enet_host_connect(netHandle, &address, 2, 0);
	
	if (netPeer != nullptr)
	{
		clientState = EClientState::CLIENT_STATE_CONNECTING;
		std::cout << "Netpeer existed!\n";
	}
	return netPeer != nullptr; 
}

void GameClient::UpdateClient(float dt) {
	if (netHandle == nullptr) {
		return;
	}

	timeGap += dt;

	// Handle all incoming packets
	ENetEvent event;
	while (enet_host_service(netHandle, &event, 0) > 0) {
		if (event.type == ENET_EVENT_TYPE_CONNECT) {
			CurrentConnetNetID = netPeer->outgoingPeerID;
			clientState = EClientState::CLIENT_STATE_CONNECTED;
			std::cout << "Connected to server !" << std::endl;

			ClientHelloPacket newPacket;
			newPacket.PlayerListIndex = PlayerIndex;
			newPacket.PeerID = CurrentConnetNetID;
			SendPacket(newPacket);
		}
		else if (event.type == ENET_EVENT_TYPE_DISCONNECT)
		{	
			clientState = EClientState::CLIENT_STATE_DISCONNECTED;
			std::cout << "Failed to connected to server !" << std::endl;
		}
		else if (event.type == ENET_EVENT_TYPE_RECEIVE) {
			//std::cout << "Client: Packet recieved ..." << std::endl;
			GamePacket* packet = (GamePacket*)event.packet -> data;
			ProcessPacket(packet);
			IncomingPacketSize += packet->GetTotalSize();
		}
		enet_packet_destroy(event.packet);
		timeGap = 0;
	}

	// Check the server State
	if (timeGap > 2.0f)
	{
		clientState = EClientState::CLIENT_STATE_DISCONNECTED;
	}
}

void GameClient::SendPacket(GamePacket&  payload) {
	ENetPacket* dataPacket = enet_packet_create(&payload,
		payload.GetTotalSize(), 0);

	OutgoingPacketSize += payload.GetTotalSize();

	enet_peer_send(netPeer, 0, dataPacket);
}
