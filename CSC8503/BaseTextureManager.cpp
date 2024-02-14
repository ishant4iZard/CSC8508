#include "BaseTextureManager.h"

unsigned int BaseTextureManager::GetTexture(const std::string& inFileName)
{
	std::map<std::string, unsigned int>::iterator itr = textureMap.find(inFileName);
	if (itr != textureMap.end())
	{
		return itr->second;
	}
	return 0;
}

BaseTextureManager::BaseTextureManager(void)
{
}