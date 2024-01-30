/******************************************************************************
This file is part of the Newcastle OpenGL Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Texture.h"
#include "glad\gl.h"

namespace NCL::Rendering {		
	using UniqueOGLTexture = std::unique_ptr<class OGLTexture>;
	using SharedOGLTexture = std::shared_ptr<class OGLTexture>;

	class OGLTexture : public Texture	{
	public:
		OGLTexture();
		OGLTexture(GLuint texToOwn);
		~OGLTexture();

		static UniqueOGLTexture TextureFromData(char* data, int width, int height, int channels);

		static UniqueOGLTexture TextureFromFile(const std::string&name);

		static UniqueOGLTexture LoadCubemap(
			const std::string& xPosFile,
			const std::string& xNegFile,
			const std::string& yPosFile,
			const std::string& yNegFile,
			const std::string& zPosFile,
			const std::string& zNegFile);

		GLuint GetObjectID() const	{
			return texID;
		}
	protected:						
		GLuint texID;
	};
}