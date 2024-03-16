#pragma once
struct DebugHUDParams {
	float dt;
	float timeCost;
	int numberOfCollisions;
	int numberOfColliders;
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