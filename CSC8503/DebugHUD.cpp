#include <string>

#include "DebugHUD.h"
#include "Vector2.h"
#include "UIBase.h"

#ifdef _WIN32
#include<windows.h>
#include <psapi.h>
#include "UIWindows.h"
#else
#include "UIPlaystation.h"
#endif 

using namespace NCL;
using namespace Maths;

std::string getMemoryUsageMB() {
#ifdef _WIN32
	double memoryUsageMB = 0.0;
	PROCESS_MEMORY_COUNTERS_EX pmc;
	if (GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
		memoryUsageMB = static_cast<double>(pmc.WorkingSetSize) / (1024 * 1024); // Convert bytes to MB
	}


	return std::to_string(memoryUsageMB);
#else
	return "N/A";
#endif
}

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
	ui->DrawStringText("FPS: " + std::to_string(1.0f / p.dt), Vector2(2, 72));
	ui->DrawStringText("Frame Rate: " + std::to_string(p.dt), Vector2(2, 75));
	ui->DrawStringText("Physics Time Cost: " + std::to_string(p.physicsTimeCost) + " micro seconds", Vector2(2, 78));
	ui->DrawStringText("Render Time Cost: " + std::to_string(p.renderTimeCost) + " micro seconds", Vector2(2, 81));
	ui->DrawStringText("Number Of Collisions: " + std::to_string(p.numberOfCollisions), Vector2(2, 84));
	ui->DrawStringText("Number Of Objects: " + std::to_string(p.numberOfObjects), Vector2(2, 87));

#ifdef _WIN32
	ui->DrawStringText("Outgoing packets: " + std::to_string(p.outgoingPacketSize) + " kb/s", Vector2(2, 90));
	ui->DrawStringText("Incoming packets: " + std::to_string(p.incomingPacketSize) + " kb/s", Vector2(2, 93));
#endif


#ifdef _WIN32
	ui->DrawStringText("Memory Footprint: " + getMemoryUsageMB() + " mb", Vector2(2, 96));
#endif
}
