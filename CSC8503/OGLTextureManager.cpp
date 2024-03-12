#include "OGLTextureManager.h"
#include "stb/stb_image.h"

unsigned int OGLTextureManager::AddTexture(const std::string& inFileName)
{
	unsigned int tempTex = GetTexture(inFileName);

	if (tempTex == 0)
	{
		int tempWidth = 0;
		int tempHeight = 0;
		LoadTextureFromFile(inFileName.c_str(), &tempTex, &tempWidth, &tempHeight);
	}
	return tempTex;
}

OGLTextureManager::~OGLTextureManager(void)
{
	for (std::map<std::string, unsigned int>::iterator i = textureMap.begin(); i != textureMap.end(); i++)
	{
		glDeleteTextures(1, &i->second);
	}
}

bool OGLTextureManager::LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP); // This is required on WebGL for non power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP); // Same

    // Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    *out_texture = image_texture;
    *out_width = image_width;
    *out_height = image_height;

    return true;
}