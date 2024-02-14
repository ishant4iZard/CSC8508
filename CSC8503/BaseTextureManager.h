#pragma once

#include"Singleton.h"
#include<map>
#include<string>

class BaseTextureManager : public Singleton<BaseTextureManager>
{
public:
	unsigned int GetTexture(const std::string& inFileName);
	virtual unsigned int AddTexture(const std::string& inFileName) = 0;
	virtual ~BaseTextureManager(void){}

protected:
	BaseTextureManager(void);
	std::map<std::string, unsigned int> textureMap;
};