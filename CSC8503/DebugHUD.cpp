#include <string>

#include "DebugHUD.h"
#include "Vector2.h"
#include "UIBase.h"
#ifdef _WIN32
#include "UIWindows.h"
#else
#include "UIPlaystation.h"
#endif 

using namespace NCL;
using namespace Maths;

DebugHUD::DebugHUD()
{
#ifdef _WIN32
	ui = UIWindows::GetInstance();
#else
	ui = UIPlaystation::GetInstance();
#endif 
}

DebugHUD::~DebugHUD()
{
	ui = nullptr;
}

void DebugHUD::DrawDebugHUD(DebugHUDParams p)
{
	ui->DrawStringText("Frame Rate: " + std::to_string(1.0f / p.dt), Vector2(2, 2));
	ui->DrawStringText("Time Cost: " + std::to_string(p.timeCost), Vector2(2, 5));
	ui->DrawStringText("Number Of Collisions: " + std::to_string(p.numberOfCollisions), Vector2(2, 8));
	ui->DrawStringText("Number Of Colliders: " + std::to_string(p.numberOfColliders), Vector2(2, 11));
}
