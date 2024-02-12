#pragma once
#include "LevelEnum.h"
class LevelFileLoaderBase
{
protected:
	std::string levelFileNameList[(int)GameLevelNumber::MAX_LEVEL];
	virtual void LoadLevelFiles() = 0;
};