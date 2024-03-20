#pragma once
struct DebugHUDParams {
	float dt;
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