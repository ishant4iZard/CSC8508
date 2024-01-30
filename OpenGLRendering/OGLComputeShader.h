/******************************************************************************
This file is part of the Newcastle OpenGL Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*/////////////////////////////////////////////////////////////////////////////
#pragma once
#include "glad\gl.h"

namespace NCL::Rendering {

	using UniqueOGLComputeShader = std::unique_ptr<class OGLComputeShader>;
	using SharedOGLComputeShader = std::shared_ptr<class OGLComputeShader>;

	class OGLComputeShader	{
	public:
		OGLComputeShader(const std::string& s);
		~OGLComputeShader();

		GLuint GetProgramID() const {
			return programID;
		}

		void Bind() const;
		void Unbind();

		//how many thread groups should be launched?
		//number of threads within a group is determined shader side
		void Execute(unsigned int x, unsigned int y = 1, unsigned int z = 1) const;

		Maths::Vector3i GetThreadGroupSize() const {
			return threadsInGroup;
		}

	protected:
		GLuint		shaderID;
		GLuint		programID;
		int			programValid;
		Maths::Vector3i	threadsInGroup;
	};
}