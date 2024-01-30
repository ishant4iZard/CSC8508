/******************************************************************************
This file is part of the Newcastle OpenGL Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*/////////////////////////////////////////////////////////////////////////////
#include "OGLComputeShader.h"
#include "OGLShader.h"
#include "Assets.h"

using namespace NCL;
using namespace Rendering;
using std::string;

OGLComputeShader::OGLComputeShader(const std::string& s)	{
	string fileContents = "";
	Assets::ReadTextFile(Assets::SHADERDIR + s, fileContents);

	programID	= glCreateProgram();
	shaderID	= glCreateShader(GL_COMPUTE_SHADER);

	const char* stringData		= fileContents.c_str();
	int			stringLength	= (int)fileContents.length();
	glShaderSource(shaderID, 1, &stringData, &stringLength);
	glCompileShader(shaderID);
	glAttachShader(programID, shaderID);

	glLinkProgram(programID);
	glGetProgramiv(programID, GL_LINK_STATUS, &programValid);

	std::cout << "Loading compute shader " << s << "\n";
	if (programValid != GL_TRUE) {
		std::cout << "Compute shader has failed!\n";
	}
	else {
		std::cout << "Compute shader loaded!\n";		
		glGetProgramiv(programID, GL_COMPUTE_WORK_GROUP_SIZE, (int*) & threadsInGroup);
	}
	OGLShader::PrintCompileLog(shaderID);
};

OGLComputeShader::~OGLComputeShader()	{
	glDetachShader(programID, shaderID);
	glDeleteShader(shaderID);
	glDeleteProgram(programID);
}

void OGLComputeShader::Execute(unsigned int x, unsigned int y, unsigned int z) const {
	glDispatchCompute(x, y, z);
}

void OGLComputeShader::Bind() const {
	glUseProgram(programID);
}

void OGLComputeShader::Unbind() {
	glUseProgram(0);
}
