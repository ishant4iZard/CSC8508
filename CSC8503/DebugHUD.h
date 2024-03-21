#pragma once
struct DebugHUDParams {
	float dt;
	long long physicsTimeCost;
	long long renderTimeCost;
	int numberOfCollisions;
	int numberOfObjects;
	float outgoingPacketSize;
	float incomingPacketSize;
};

class UIBase;

class DebugHUD {
public :
	DebugHUD();
	~DebugHUD();
	void DrawDebugHUD(DebugHUDParams p);

private :
	UIBase* ui;
};