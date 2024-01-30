/******************************************************************************
This file is part of the Newcastle OpenGL Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*/////////////////////////////////////////////////////////////////////////////
#include "OGLTexture.h"
#include "OGLRenderer.h"

#include "TextureLoader.h"

using namespace NCL;
using namespace NCL::Rendering;

OGLTexture::OGLTexture()	{
	glGenTextures(1, &texID);
}

OGLTexture::OGLTexture(GLuint texToOwn) {
	texID = texToOwn;
}

OGLTexture::~OGLTexture()	{
	glDeleteTextures(1, &texID);
}

UniqueOGLTexture OGLTexture::TextureFromData(char* data, int width, int height, int channels) {
	UniqueOGLTexture tex = std::make_unique<OGLTexture>();
	tex->dimensions = { width, height };

	int dataSize = width * height * channels; //This always assumes data is 1 byte per channel

	int sourceType = GL_RGB;

	switch (channels) {
		case 1: sourceType = GL_RED	; break;
		case 2: sourceType = GL_RG	; break;
		case 3: sourceType = GL_RGB	; break;
		case 4: sourceType = GL_RGBA; break;
	}

	glBindTexture(GL_TEXTURE_2D, tex->texID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, sourceType, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	return tex;
}

UniqueOGLTexture OGLTexture::TextureFromFile(const std::string&name) {
	char* texData	= nullptr;
	int width		= 0;
	int height		= 0;
	int channels	= 0;
	int flags		= 0;
	TextureLoader::LoadTexture(name, texData, width, height, channels, flags);  

	UniqueOGLTexture glTex = TextureFromData(texData, width, height, channels);

	free(texData);

	return glTex;
}

UniqueOGLTexture OGLTexture::LoadCubemap(
	const std::string& xPosFile, 
	const std::string& xNegFile, 
	const std::string& yPosFile, 
	const std::string& yNegFile, 
	const std::string& zPosFile, 
	const std::string& zNegFile) {

	const std::string* filenames[6] = {&xPosFile,&xNegFile,&yPosFile,&yNegFile,&zPosFile,&zNegFile};
	int width[6] = { 0 };
	int height[6] = { 0 };
	int channels[6] = { 0 };
	int flags[6] = { 0 };

	std::vector<char*> texData(6, nullptr);

	for (int i = 0; i < 6; ++i) {
		TextureLoader::LoadTexture(*filenames[i], texData[i], width[i], height[i], channels[i], flags[i]);
		if (i > 0 && (width[i] != width[0] || height[0] != height[0])) {
			std::cout << __FUNCTION__ << " cubemap input textures don't match in size?\n";
			return nullptr;
		}
	}

	UniqueOGLTexture tex = std::make_unique<OGLTexture>();
	tex->dimensions = { width[0], height[0] };

	glBindTexture(GL_TEXTURE_CUBE_MAP, tex->GetObjectID());

	GLenum type = channels[0] == 4 ? GL_RGBA : GL_RGB;

	for (int i = 0; i < 6; ++i) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width[i], height[i], 0, type, GL_UNSIGNED_BYTE, texData[i]);
	}

	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return tex;
}