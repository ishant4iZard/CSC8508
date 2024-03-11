#pragma once

namespace NCL::Rendering {
	class Mesh;
	class Texture;
	class Shader;
}

namespace NCL::CSC8503 {
	class GameTechRendererInterface
	{
	public:
		virtual Mesh* LoadMesh(const std::string& name) = 0;
		virtual Texture* LoadTexture(const std::string& name) = 0;
		virtual Shader* LoadShader(const std::string& vertex, const std::string& fragment) = 0;
	};
}

