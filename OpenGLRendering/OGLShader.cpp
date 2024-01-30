/******************************************************************************
This file is part of the Newcastle OpenGL Tutorial Series

Author:Rich Davison
Contact:richgdavison@gmail.com
License: MIT (see LICENSE file at the top of the source tree)
*/////////////////////////////////////////////////////////////////////////////
#include "OGLShader.h"
#include "Assets.h"

using namespace NCL;
using namespace NCL::Rendering;

using std::string;

GLuint shaderTypes[ShaderStages::MAX_SIZE] = {
	GL_VERTEX_SHADER,
	GL_FRAGMENT_SHADER,
	GL_GEOMETRY_SHADER,
	GL_TESS_CONTROL_SHADER,
	GL_TESS_EVALUATION_SHADER
};

string shaderNames[ShaderStages::MAX_SIZE] = {
	"Vertex",
	"Fragment",
	"Geometry",
	"Tess. Control",
	"Tess. Eval"
};

OGLShader::OGLShader(const string& vertex, const string& fragment, const string& geometry, const string& domain, const string& hull) :
	Shader(vertex, fragment, geometry, domain, hull) {

	for (int i = 0; i < ShaderStages::MAX_SIZE; ++i) {
		shaderIDs[i]	= 0;
		shaderValid[i]	= 0;
	}
	programID = 0;

	ReloadShader();
}

OGLShader::~OGLShader()	{
	DeleteIDs();
}

void OGLShader::ReloadShader() {
	DeleteIDs();
	programID = glCreateProgram();
	string fileContents = "";
	for (int i = 0; i < ShaderStages::MAX_SIZE; ++i) {
		if (!shaderFiles[i].empty()) {
			if (Assets::ReadTextFile(Assets::SHADERDIR + shaderFiles[i], fileContents)) {
				Preprocessor(fileContents);

				shaderIDs[i] = glCreateShader(shaderTypes[i]);

				std::cout << "Reading " << shaderNames[i] << " shader " << shaderFiles[i] << "\n";

				const char* stringData	 = fileContents.c_str();
				int			stringLength = (int)fileContents.length();
				glShaderSource(shaderIDs[i], 1, &stringData, &stringLength);
				glCompileShader(shaderIDs[i]);

				glGetShaderiv(shaderIDs[i], GL_COMPILE_STATUS, &shaderValid[i]);
		
				if (shaderValid[i] == GL_TRUE) {					
					glAttachShader(programID, shaderIDs[i]);
				}
				else {
					std::cout << shaderNames[i] << " shader " << shaderFiles[i] << " has failed!" << "\n";
				}
				PrintCompileLog(shaderIDs[i]);
			}
		}
	}	
	glLinkProgram(programID);
	glGetProgramiv(programID, GL_LINK_STATUS, &programValid);

	PrintLinkLog(programID);

	if (programValid != GL_TRUE) {
		std::cout << "This shader has failed!" << "\n";
	}
	else {
		std::cout << "Shader loaded!" << "\n";
	}
}

void	OGLShader::DeleteIDs() {
	if (!programID) {
		return;
	}
	for (int i = 0; i < ShaderStages::MAX_SIZE; ++i) {
		if (shaderIDs[i]) {
			glDetachShader(programID, shaderIDs[i]);
			glDeleteShader(shaderIDs[i]);
		}
	}
	glDeleteProgram(programID);
	programID = 0;
}

void	OGLShader::PrintCompileLog(GLuint object) {
	int logLength = 0;
	glGetShaderiv(object, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength) {
		std::string temp(logLength, ' ');
		glGetShaderInfoLog(object, logLength, NULL, temp.data());
		std::cout << "Compile Log:\n" << temp << "\n";
	}
}

void	OGLShader::PrintLinkLog(GLuint program) {
	int logLength = 0;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);

	if (logLength) {
		std::string temp(logLength, ' ');
		glGetProgramInfoLog(program, logLength, NULL, temp.data());
		std::cout << "Link Log:\n" << temp << "\n";
	}
}

bool	OGLShader::Preprocessor(string& shaderFile) {
	size_t lineStart	= 0;
	size_t lineEnd		= 0;

	std::set<std::string> includes;

	while(true) {
		lineEnd = shaderFile.find('\n', lineStart);
		if (lineEnd == string::npos) {
			return true;
		}

		size_t lineLength = lineEnd - lineStart;

		std::string_view substr = std::string_view(shaderFile).substr(lineStart, lineLength);

		size_t hasInclude = substr.find("#include");
		 
		if (hasInclude != string::npos) {
			size_t nameStart	= substr.find_first_of("\"");
			size_t nameEnd		= substr.find_last_of("\"");

			std::string filename = std::string(substr.substr(nameStart+1, nameEnd - nameStart-1));

			auto search = includes.find(filename);

			if (search != includes.end()) {
				std::cout << __FUNCTION__ << ": Preprocessor failure! Recursive inclusion of file " << filename << "\n";
				return false;
			}

			includes.insert(filename);

			std::string newFile;
			if (!Assets::ReadTextFile(Assets::SHADERDIR + filename, newFile)) {
				std::cout << __FUNCTION__ << ": Preprocessor failure! No include file " << filename << " can be found.\n";
				return false;
			}
			else {
				shaderFile.replace(lineStart, lineLength, newFile);
				continue; //Don't move on, the replaced file could have additional includes!
			}	
		}
		lineStart = lineEnd+1;
	}
	return true;
}
