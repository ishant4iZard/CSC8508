#pragma once
#include "LevelFileLoaderBase.h"
#include <string>
class WindowsLevelLoader : public LevelFileLoaderBase
{
public:
	WindowsLevelLoader();

	void LoadLevelFiles() override;

	std::string GetLevelFilePath(GameLevelNumber inLevelNum);
};