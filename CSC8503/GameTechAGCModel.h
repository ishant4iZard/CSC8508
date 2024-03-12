#pragma once
#include <map>
namespace NCL
{
	namespace Rendering
	{
		class Mesh;
		class Texture;
		class Shader;
	}
	namespace PS5
	{
		class AGCTexture;
		class AGCShader;
	}

	enum class ComputeShaderName : unsigned long long
	{
		SKINNING = 0,
		GAMMA,
		MAX_SHADER
	};

	enum class VertexShaderName : unsigned long long
	{
		DEFAULT = 0,
		SHADOW,
		SKY_BOX,
		DEBUG_LINE,
		DEBUG_TEXT,
		PBR,
		MAX_SHADER
	};

	enum class PixelShaderName : unsigned long long
	{
		DEFAULT = 0,
		SHADOW,
		SKY_BOX,
		DEBUG_LINE,
		DEBUG_TEXT,
		PBR,
		MAX_SHADER
	};

	class GameTechAGCModel
	{
	public:
		GameTechAGCModel();
		~GameTechAGCModel();

		NCL::PS5::AGCTexture* GetDefaultTex() const { return defaultTexture; }
		NCL::PS5::AGCTexture* GetSkyboxTex() const { return skyboxTexture; }

		void SetDefaultTex(NCL::PS5::AGCTexture* inTex) { defaultTexture = inTex; }
		void SetSkyboxTex(NCL::PS5::AGCTexture* inSkyBoxTex) { skyboxTexture = inSkyBoxTex; }

		void SetComputeShader(const ComputeShaderName& inName, NCL::PS5::AGCShader* inShader) { computerShaderMap[(unsigned long long)inName] = inShader; }
		NCL::PS5::AGCShader* GetComputeShader(const ComputeShaderName& inName) { return computerShaderMap[(unsigned long long)inName]; }

		void SetVertexShader(const VertexShaderName& inName, NCL::PS5::AGCShader* inShader) { vertexShaderList[(unsigned long long)inName] = inShader; }
		NCL::PS5::AGCShader* GetVertexShader(const VertexShaderName& inName) { return vertexShaderList[(unsigned long long)inName]; }

		void SetPixelShader(const PixelShaderName& inName, NCL::PS5::AGCShader* inShader) { pixelShaderList[(unsigned long long)inName] = inShader; }
		NCL::PS5::AGCShader* GetPixelShader(const PixelShaderName& inName) { return pixelShaderList[(unsigned long long)inName]; }

	protected:
		NCL::PS5::AGCShader* pbrVertexShader;
		NCL::PS5::AGCShader* pbrPixelShader;

		NCL::PS5::AGCTexture* defaultTexture;
		NCL::PS5::AGCTexture* skyboxTexture;

		NCL::PS5::AGCShader* computerShaderMap[(unsigned long long)ComputeShaderName::MAX_SHADER];
		NCL::PS5::AGCShader* vertexShaderList[(unsigned long long)VertexShaderName::MAX_SHADER];
		NCL::PS5::AGCShader* pixelShaderList[(unsigned long long)PixelShaderName::MAX_SHADER];

	};
}