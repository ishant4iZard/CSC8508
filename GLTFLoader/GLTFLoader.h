#pragma once
#include <vector>
#include <map>
#include <string>
#include <functional>
#include <iostream>

#include "../NCLCoreClasses/Assets.h"
#include "../NCLCoreClasses/Mesh.h"
#include "../NCLCoreClasses/MeshAnimation.h"
#include "../NCLCoreClasses/Texture.h"
#include "../NCLCoreClasses/Matrix4.h"

namespace tinygltf {
	class Model;
}

namespace NCL::Assets {
	const std::string GLTFDIR(ASSETROOT + "GLTF/");
}

namespace NCL::Rendering {
	class MeshAnimation;
	class Texture;
	using namespace Maths;
	struct GLTFMaterialLayer {
		SharedTexture albedo;
		SharedTexture bump;
		SharedTexture occlusion;
		SharedTexture emission;
		SharedTexture metallic;

		GLTFMaterialLayer() {
		}
	};

	struct GLTFMaterial {
		std::vector< GLTFMaterialLayer > allLayers;
	};		

	struct GLTFNode {
		std::string name;
		uint32_t nodeID = 0;

		Mesh* mesh = nullptr;
		GLTFMaterial* material = nullptr;

		Matrix4 localMatrix;
		Matrix4 worldMatrix;

		GLTFNode* parent = nullptr;
		std::vector<GLTFNode*> children;
	};	

	struct GLTFScene {
		std::vector<GLTFNode>			sceneNodes;
		std::vector<SharedMesh>			meshes;
		std::vector<SharedTexture>		textures;
		std::vector<GLTFMaterial>		materials;
		std::vector<SharedMeshAnim>		animations;
		std::vector<GLTFMaterialLayer>	materialLayers;
	};

	class GLTFLoader	{
	public:
		typedef std::function<NCL::Rendering::Mesh* (void)>				MeshConstructionFunction;
		typedef std::function<NCL::Rendering::Texture* (std::string&)>	TextureConstructionFunction;

		static bool Load(const std::string& filename, GLTFScene& intoScene, MeshConstructionFunction meshConstructor, TextureConstructionFunction textureConstruction);
		
	protected:		
		GLTFLoader()  = delete;
		~GLTFLoader() = delete;

		struct GLTFSkin {
			std::vector<std::string>	localJointNames;
			std::map<int, int>			sceneToLocalLookup;
			std::map<int, int>			localToSceneLookup;
			std::vector<Maths::Matrix4>	worldBindPose;
			std::vector<Maths::Matrix4> worldInverseBindPose;
			Maths::Matrix4				globalTransformInverse;
		};

		static void LoadImages(tinygltf::Model& m, GLTFScene& scene, const std::string& rootFile, TextureConstructionFunction texFunc);
		static void LoadMaterials(tinygltf::Model& m, GLTFScene& scene);
		static void LoadSceneNodeData(tinygltf::Model& m, GLTFScene& scene);
		static void LoadVertexData(tinygltf::Model& m, GLTFScene& scene, GLTFLoader::MeshConstructionFunction meshConstructor);
		static void LoadSkinningData(tinygltf::Model& m, GLTFScene& scene, Mesh& geometry);
		static void LoadAnimationData(tinygltf::Model& m, GLTFScene& scene, Mesh& mesh, GLTFSkin& skin);
	};
}
