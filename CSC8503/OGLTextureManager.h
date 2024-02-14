#pragma once
#include "BaseTextureManager.h"
#include <glad/gl.h>

class OGLTextureManager : public BaseTextureManager
{
public:
	unsigned int AddTexture(const std::string& inFileName) override;
	~OGLTextureManager(void) override;
protected:
	bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height);
};