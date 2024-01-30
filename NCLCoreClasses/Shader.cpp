#include "Shader.h"
using namespace NCL;
using namespace Rendering;

using std::string;

Shader::Shader(const string& vertex, const string& fragment, const string& geometry, const string& domain, const string& hull)
{
	shaderFiles[ShaderStages::Vertex]	= vertex;
	shaderFiles[ShaderStages::Fragment]	= fragment;
	shaderFiles[ShaderStages::Geometry]	= geometry;
	shaderFiles[ShaderStages::Domain]	= domain;
	shaderFiles[ShaderStages::Hull]		= hull;
}

Shader::~Shader()
{
}