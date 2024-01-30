/******************************************************************************
This file is part of the Newcastle OpenGL Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Shader.h"
#include "glad\gl.h"

namespace NCL::Rendering {
	using UniqueOGLShader = std::unique_ptr<class OGLShader>;
	using SharedOGLShader = std::shared_ptr<class OGLShader>;

	class OGLShader : public Shader	{
	public:
		OGLShader(const std::string& vertex, const std::string& fragment, const std::string& geometry = "", const std::string& domain = "", const std::string& hull = "");
		~OGLShader();

		void ReloadShader() override;

		bool LoadSuccess() const {
			return programValid == GL_TRUE;
		}

		int GetProgramID() const {
			return programID;
		}	
			
		static void	PrintCompileLog(GLuint object);
		static void	PrintLinkLog(GLuint program);

		static bool Preprocessor(std::string& shaderFile);

	protected:
		void	DeleteIDs();

		GLuint	programID;
		GLuint	shaderIDs[ShaderStages::MAX_SIZE];
		int		shaderValid[ShaderStages::MAX_SIZE];
		int		programValid;
	};
}