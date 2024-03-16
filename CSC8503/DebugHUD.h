#pragma once
struct DebugHUDParams {
	float dt;
	long long timeCost;
	int numberOfCollisions;
	int numberOfObjects;
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