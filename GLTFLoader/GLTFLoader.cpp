#include "GLTFLoader.h"
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_USE_CPP14
#define TINYGLTF_NO_INCLUDE_STB_IMAGE
#define TINYGLTF_NO_INCLUDE_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_NO_STB_IMAGE 
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"

#include "../NCLCoreClasses/Quaternion.h"
#include "../NCLCoreClasses/Maths.h"

#include "../NCLCoreClasses/TextureLoader.h"
#include "../NCLCoreClasses/Texture.h"

#include "../NCLCoreClasses/MeshAnimation.h"
#include "../NCLCoreClasses/Mesh.h"


#include <filesystem>
#include <cmath>
#include <stack>
#include "../NCLCoreClasses/Vector4.h"
#include "../NCLCoreClasses/Vector4i.h"
#include "../NCLCoreClasses/Matrix3.h"
#include "../NCLCoreClasses/Vector2.h"
#include "../NCLCoreClasses/Vector3.h"

using namespace tinygltf;
using namespace NCL;
using namespace NCL::Maths;
using namespace NCL::Rendering;
using NCL::Rendering::Texture;

const std::string GLTFAttributeTags[] = {
	"POSITION",
	"COLOR",
	"TEXCOORD_0",
	"NORMAL",
	"TANGENT",
	"WEIGHTS_0",
	"JOINTS_0",
};

template <class toType, class fromType>
void ReadDataInternal(toType* destination, const Accessor& accessor, const Model& model, int firstElement, int elementCount) {
	const BufferView& v	= model.bufferViews[accessor.bufferView];
	const Buffer& b		= model.buffers[v.buffer];
	const unsigned char* data = &(b.data[v.byteOffset + accessor.byteOffset]);
	int stride		= accessor.ByteStride(v);

	int inAxisCount = 1;

	if (accessor.type == TINYGLTF_TYPE_VEC2) {
		inAxisCount = 2;
	}
	else if (accessor.type == TINYGLTF_TYPE_VEC3) {
		inAxisCount = 3;
	}
	else if (accessor.type == TINYGLTF_TYPE_VEC4) {
		inAxisCount = 4;
	}

	size_t realCount = std::min((size_t)elementCount, accessor.count);

	for (int i = firstElement; i < (firstElement + realCount); ++i) {
		fromType* aData = (fromType*)(data + (stride * i));
		for (int j = 0; j < inAxisCount; ++j) {
			*destination = (toType)*aData;
			destination++;
			aData++;
		}
	}
}

template <class toType>
void	ReadData(const Accessor& accessor, const Model& model, void* d, int firstElement = 0, int elementCount = -1) {
	toType* destination = (toType*)d;
	if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
		ReadDataInternal<toType, unsigned int>(destination, accessor, model, firstElement, elementCount);
	}
	else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
		ReadDataInternal<toType, unsigned char>(destination, accessor, model, firstElement, elementCount);
	}
	else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
		ReadDataInternal<toType, unsigned short>(destination, accessor, model, firstElement, elementCount);
	}
	else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
		ReadDataInternal<toType, float>(destination, accessor, model, firstElement, elementCount);
	}
	else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_DOUBLE) {
		ReadDataInternal<toType, double>(destination, accessor, model, firstElement, elementCount);
	}
}

template <class vecType, class elemType>
void CopyVectorData(std::vector<vecType>& vec, size_t destStart, const Accessor& accessor, const Model& model) {
	ReadData<elemType>(accessor, model, &vec[destStart]);
}

void GetInterpolationData(float forTime, int& indexA, int& indexB, float& t, const Accessor& accessor, const Model& model) {
	const auto& inBufferView = model.bufferViews[accessor.bufferView];
	const auto& inBuffer	 = model.buffers[inBufferView.buffer];

	const unsigned char* inData = inBuffer.data.data() + inBufferView.byteOffset + accessor.byteOffset;
	int stride = accessor.ByteStride(inBufferView);

	float a = 0.0f;
	float b = 0.0f;

	if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_DOUBLE) {
		double* ddata = (double*)inData;
		for (int i = 0; i < accessor.count; ++i) {
			if (ddata[i] > forTime) {
				indexA = i > 1 ? (i - 1) : 0;
				indexB = i;
				b = (float)*((double*)(inData + (stride * indexB)));
				a = (float)*((double*)(inData + (stride * indexA)));
				break;
			}
		}
	}
	else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
		float* fdata = (float*)inData;
		for (int i = 0; i < accessor.count; ++i) {
			if (fdata[i] > forTime || i == accessor.count - 1) {
				indexA = i > 0 ? (i - 1) : 0;
				indexB = i;
				b = *(float*)(inData + (stride * indexB));
				a = *(float*)(inData + (stride * indexA));
				break;
			}
		}
	}
	if (indexA == indexB) {
		t = 0.0f;
	}
	else {
		t = std::clamp((forTime - a) / (b - a), 0.0f, 1.0f);
	}
}

Quaternion GetSlerpQuaterion(float t, int indexA, int indexB, const Accessor& accessor, Model& model) {
	Quaternion qa;
	Quaternion qb;

	ReadData<float>(accessor, model, (float*)&qa, indexA, 1);
	ReadData<float>(accessor, model, (float*)&qb, indexB, 1);

	if (indexA == indexB) {
		return qa;
	}
	return Quaternion::Slerp(qa, qb, t);
}

template <class vecType>
vecType GetInterpolatedVector(float t, int indexA, int indexB, const Accessor& accessor, Model& model) {
	vecType a;
	vecType b;
	ReadData<float>(accessor, model, (float*)&a, indexA, 1);
	ReadData<float>(accessor, model, (float*)&b, indexB, 1);

	if (indexA == indexB) {
		return a;
	}

	return (a * (1.0f - t)) + (b * t);
}

bool GLTFLoader::Load(const std::string& filename, GLTFScene& intoScene, GLTFLoader::MeshConstructionFunction inMeshConstructor, TextureConstructionFunction inTextureConstruction) {
	TinyGLTF gltf;
	Model	 model;

	if (!gltf.LoadASCIIFromFile(&model, nullptr, nullptr, NCL::Assets::GLTFDIR + filename)) {
		return false;
	}

	LoadImages(model, intoScene, filename, inTextureConstruction);
	LoadMaterials(model, intoScene);
	LoadSceneNodeData(model, intoScene);

	LoadVertexData(model, intoScene, inMeshConstructor);

	return true;
}

void GLTFLoader::LoadImages(tinygltf::Model& m, GLTFScene& scene, const std::string& rootFile, TextureConstructionFunction texFunc) {
	std::map<std::string, NCL::Rendering::SharedTexture> loadedTexturesMap;

	std::filesystem::path p			= rootFile;
	std::filesystem::path subPath	= p.parent_path();

	for (const auto& i : m.images) {
		std::filesystem::path imagePath = std::filesystem::path(NCL::Assets::GLTFDIR);
		imagePath += std::filesystem::path(p.parent_path());
		imagePath.append(i.uri);
		std::string pathString = imagePath.string();
	
		SharedTexture tex = SharedTexture(texFunc(pathString));

		scene.textures.push_back(tex);
		loadedTexturesMap.insert({ i.uri,tex });
	}
}

void GLTFLoader::LoadMaterials(tinygltf::Model& m, GLTFScene& scene) {
	scene.materialLayers.reserve(m.materials.size());
	for (const auto& m : m.materials) {
		GLTFMaterialLayer layer;
		layer.albedo	= m.pbrMetallicRoughness.baseColorTexture.index			>= 0 ? scene.textures[m.pbrMetallicRoughness.baseColorTexture.index]		  : nullptr;
		layer.metallic	= m.pbrMetallicRoughness.metallicRoughnessTexture.index >= 0 ? scene.textures[m.pbrMetallicRoughness.metallicRoughnessTexture.index] : nullptr;
			
		layer.bump		= m.normalTexture.index		>= 0 ? scene.textures[m.normalTexture.index]	 : nullptr;
		layer.occlusion = m.occlusionTexture.index	>= 0 ? scene.textures[m.occlusionTexture.index] : nullptr;
		layer.emission	= m.emissiveTexture.index	>= 0 ? scene.textures[m.emissiveTexture.index]  : nullptr;
		
		scene.materialLayers.push_back(layer);
	}
}

void GLTFLoader::LoadVertexData(tinygltf::Model& model, GLTFScene& scene, GLTFLoader::MeshConstructionFunction meshConstructor) {
	for (const auto& m : model.meshes) {
		SharedMesh mesh = SharedMesh(meshConstructor());

		GLTFMaterial material;

		if (m.primitives.empty()) {
			continue;
		}
		size_t totalVertexCount = 0;

		bool hasAttribute[VertexAttribute::MAX_ATTRIBUTES] = { false };

		for (const auto& p : m.primitives) {
			for (int i = 0; i < VertexAttribute::MAX_ATTRIBUTES; ++i) {
				hasAttribute[i] |= p.attributes.find(GLTFAttributeTags[i]) != p.attributes.end();
			}

			auto hasVerts = p.attributes.find(GLTFAttributeTags[VertexAttribute::Positions]);

			if (hasVerts != p.attributes.end()) {
				totalVertexCount += model.accessors[hasVerts->second].count;
			}
		}
		std::vector<Vector3> vPositions(totalVertexCount);
		std::vector<Vector3> vNormals(hasAttribute[VertexAttribute::Normals] ? totalVertexCount : 0);
		std::vector<Vector4> vTangents(hasAttribute[VertexAttribute::Tangents] ? totalVertexCount : 0);
		std::vector<Vector2> vTexCoords(hasAttribute[VertexAttribute::TextureCoords] ? totalVertexCount : 0);

		std::vector<Vector4>  vJointWeights(hasAttribute[VertexAttribute::JointWeights] ? totalVertexCount : 0);
		std::vector<Vector4i> vJointIndices(hasAttribute[VertexAttribute::JointIndices] ? totalVertexCount : 0);

		std::vector<unsigned int>		vIndices;

		size_t vArrayPos = 0;

		//now load up the actual vertex data
		for (const auto& p : m.primitives) {
			std::map<std::string, int>::const_iterator vPrims[VertexAttribute::MAX_ATTRIBUTES];

			for (int i = 0; i < VertexAttribute::MAX_ATTRIBUTES; ++i) {
				vPrims[i] = p.attributes.find(GLTFAttributeTags[i]);
			}

			size_t baseVertex = vArrayPos;
			size_t firstIndex = vIndices.size();

			if (vPrims[VertexAttribute::JointWeights] != p.attributes.end()) {
				Accessor& a = model.accessors[vPrims[VertexAttribute::JointWeights]->second];
				CopyVectorData<Vector4, float>(vJointWeights, vArrayPos, a, model);
			}
			if (vPrims[VertexAttribute::JointIndices] != p.attributes.end()) {
				Accessor& a = model.accessors[vPrims[VertexAttribute::JointIndices]->second];
				CopyVectorData<Vector4i, unsigned int>(vJointIndices, vArrayPos, a, model);
			}
			if (vPrims[VertexAttribute::Normals] != p.attributes.end()) {
				Accessor& a = model.accessors[vPrims[VertexAttribute::Normals]->second];
				CopyVectorData<Vector3, float>(vNormals, vArrayPos, a, model);
			}
			if (vPrims[VertexAttribute::Tangents] != p.attributes.end()) {
				Accessor& a = model.accessors[vPrims[VertexAttribute::Tangents]->second];
				CopyVectorData<Vector4, float>(vTangents, vArrayPos, a, model);
			}
			if (vPrims[VertexAttribute::TextureCoords] != p.attributes.end()) {
				Accessor& a = model.accessors[vPrims[VertexAttribute::TextureCoords]->second];
				CopyVectorData<Vector2, float>(vTexCoords, vArrayPos, a, model);
			}
			if (vPrims[VertexAttribute::Positions] != p.attributes.end()) {
				Accessor& a = model.accessors[vPrims[VertexAttribute::Positions]->second];
				CopyVectorData<Vector3, float>(vPositions, vArrayPos, a, model);
				vArrayPos += a.count;
			}

			if (p.indices >= 0) {
				Accessor& a = model.accessors[p.indices];

				size_t start = vIndices.size();
				vIndices.resize(start + a.count);

				CopyVectorData<unsigned int, unsigned int>(vIndices, start, a, model);
			}

			GLTFMaterialLayer matLayer;

			if (p.material >= 0) { //fcan ever be false?
				matLayer = scene.materialLayers[p.material];
			}
			material.allLayers.push_back(matLayer);

			mesh->AddSubMesh((int)firstIndex, (int)(vIndices.size() - firstIndex), (int)baseVertex);
		}
		mesh->SetVertexPositions(vPositions);
		mesh->SetVertexIndices(vIndices);
		mesh->SetVertexNormals(vNormals);
		mesh->SetVertexTangents(vTangents);
		mesh->SetVertexTextureCoords(vTexCoords);

		mesh->SetVertexSkinIndices(vJointIndices);
		mesh->SetVertexSkinWeights(vJointWeights);

		scene.meshes.push_back(mesh);
		scene.materials.push_back(material);

		LoadSkinningData(model, scene, *mesh);
	}
}

void GLTFLoader::LoadSceneNodeData(tinygltf::Model& m, GLTFScene& scene) {
	scene.sceneNodes.resize(m.nodes.size());

	for (int i = 0; i < m.nodes.size(); ++i) {
		auto& sceneNode = scene.sceneNodes[i];
		auto& fileNode = m.nodes[i];

		sceneNode.name = fileNode.name;
		sceneNode.nodeID = i;

		Matrix4 mat;
		if (!fileNode.matrix.empty()) { //node can be defined with a matrix - never targeted by animations!
			float* data = (float*)(&(mat.array));
			for (int j = 0; j < fileNode.matrix.size(); ++j) {
				*data = (float)fileNode.matrix[j];
				++data;
			}
		}
		else {
			Matrix4 rotation;
			Matrix4 translation;
			Matrix4 scale;

			if (!fileNode.scale.empty()) {
				Vector3 s = { (float)fileNode.scale[0], (float)fileNode.scale[1], (float)fileNode.scale[2] };
				scale = Matrix4::Scale(s);
			}
			if (!fileNode.translation.empty()) {
				Vector3 t = { (float)fileNode.translation[0], (float)fileNode.translation[1], (float)fileNode.translation[2] };
				translation = Matrix4::Translation(t);
			}
			if (!fileNode.rotation.empty()) {
				Quaternion q = Quaternion(fileNode.rotation[0], fileNode.rotation[1], fileNode.rotation[2], fileNode.rotation[3]).Normalised();

				rotation = Quaternion::RotationMatrix<Matrix4>(q);
			}
			mat = translation * rotation * scale;
		}
		sceneNode.localMatrix = mat;
		sceneNode.worldMatrix = mat; //will be sorted out later!

		sceneNode.children.resize(fileNode.children.size());

		for (int j = 0; j < fileNode.children.size(); ++j) {
			sceneNode.children[j] = &(scene.sceneNodes[fileNode.children[j]]);
			sceneNode.children[j]->parent = &sceneNode;
		}
	}
	//There's seemingly no guarantee that a child node comes after its parent...(RiggedSimple demonstrates this)
	//So instead we need to traverse any top-level nodes and build up the world matrices from there
	std::stack<GLTFNode*> nodesToVisit;
	for (int i = 0; i < scene.sceneNodes.size(); ++i) {
		if (scene.sceneNodes[i].parent == nullptr) {
			nodesToVisit.push(&scene.sceneNodes[i]);	//a top level node!
		}
	}
	while (!nodesToVisit.empty()) {
		auto& sceneNode =	nodesToVisit.top();
		nodesToVisit.pop();
		for (int i = 0; i < sceneNode->children.size(); ++i) {
			auto cNode = sceneNode->children[i];
			cNode->worldMatrix = sceneNode->worldMatrix * cNode->localMatrix;
			nodesToVisit.push(cNode);
		}
	}
}

void GLTFLoader::LoadSkinningData(tinygltf::Model& model, GLTFScene& scene, Mesh& mesh) {
	if (model.skins.empty()) {
		return;
	}
	int skinID = 0;
	for (auto& skin : model.skins) {
		GLTFSkin skinData;

		int rootIndex = 0; //TODO: Is it guaranteed that the first node is the root...
		for (int i = 0; i < model.nodes.size(); ++i) {
			if (model.nodes[i].skin == skinID) {
				rootIndex = i;
				break;
			}
		}
		skinData.globalTransformInverse.Inverse();
			//= Matrix4::Inverse(scene.sceneNodes[rootIndex].worldMatrix);

		skinData.worldInverseBindPose.resize(skin.joints.size());

		int index = skin.inverseBindMatrices;
		if (index >= 0) {
			Accessor& a = model.accessors[index];

			const auto& inBufferView = model.bufferViews[a.bufferView];
			const auto& inBuffer = model.buffers[inBufferView.buffer];

			const unsigned char* inData = inBuffer.data.data() + inBufferView.byteOffset + a.byteOffset;
			Matrix4* matData = (Matrix4*)inData;

			size_t count = inBufferView.byteLength / sizeof(Matrix4);

			for (int i = 0; i < count; ++i) {
				skinData.worldInverseBindPose[i] = *matData;
				matData++;
			}
		}
		else {
			//???
		}

		//Build the correct list for the parent lookup later
		for (int i = 0; i < skin.joints.size(); ++i) {
			auto& node = model.nodes[skin.joints[i]];
			skinData.localJointNames.push_back(node.name);
			skinData.sceneToLocalLookup.insert({ skin.joints[i], i });
			skinData.localToSceneLookup.insert({i, skin.joints[i] });
			skinData.worldBindPose.push_back(scene.sceneNodes[skin.joints[i]].worldMatrix);
		}

		std::vector<int>	localParentList;

		for (int i = 0; i < skin.joints.size(); ++i) {
			GLTFNode& node = scene.sceneNodes[skin.joints[i]];

			if (node.parent == nullptr) {
				localParentList.push_back(-1);
			}
			else {
				auto result = skinData.sceneToLocalLookup.find(node.parent->nodeID);
				if (result == skinData.sceneToLocalLookup.end()) {
					localParentList.push_back(-1);
				}
				else {
					localParentList.push_back(result->second);
				}
			}
		}
		mesh.SetJointNames(skinData.localJointNames);
		mesh.SetJointParents(localParentList);
		mesh.SetBindPose(skinData.worldBindPose);
		mesh.SetInverseBindPose(skinData.worldInverseBindPose);
		LoadAnimationData(model, scene, mesh, skinData);
		skinID++;
	}
}

void GLTFLoader::LoadAnimationData(tinygltf::Model& model, GLTFScene& scene, Mesh& mesh, GLTFSkin& skinData) {
	size_t jointCount = mesh.GetBindPose().size();
	std::vector<int> jointParents = mesh.GetJointParents();

	for (const auto& anim : model.animations) {
		float animLength = 0.0f;
		for (int i = 0; i < anim.samplers.size(); ++i) {
			int timeSrc = anim.samplers[i].input;
			animLength = std::max(animLength, (float)(model.accessors[timeSrc].maxValues[0]));
		}
		float time = 0.0f;
		float frameRate = 1.0f / 30.0f;
		unsigned int frameCount = 0;

		std::vector<Matrix4> localMatrices;
		localMatrices.reserve(jointCount * (animLength / frameRate));

		std::vector<int> inAnim(jointCount, 0); //TODO: reduce to only nodes that matter

		static int TRANSLATION_BIT	= 1;
		static int ROTATION_BIT		= 2;
		static int SCALE_BIT		= 4;

		while (time <= animLength) {
			localMatrices.reserve((frameCount+1) * jointCount);

			std::map<int, Vector3> frameJointTranslations;
			std::map<int, Vector3> frameJointScales;
			std::map<int, Quaternion> frameJointRotations;

			std::fill(inAnim.begin(), inAnim.end(), 0);

			for (const auto& channel : anim.channels) {
				const auto& sampler = anim.samplers[channel.sampler];
				const auto& input	= model.accessors[sampler.input];
				const auto& output	= model.accessors[sampler.output];

				int indexA	= 0;
				int indexB	= 0;
				float t		= 0.0f;
				GetInterpolationData(time, indexA, indexB, t, input, model);

				int localNodeID = skinData.sceneToLocalLookup[channel.target_node];

				if (channel.target_path == "translation") {
					frameJointTranslations.insert({ localNodeID, GetInterpolatedVector<Vector3>(t, indexA, indexB, output, model) });
					inAnim[localNodeID] |= TRANSLATION_BIT;
				}
				else if (channel.target_path == "rotation") {
					Quaternion q = GetSlerpQuaterion(t, indexA, indexB, output, model);
					frameJointRotations.insert({ localNodeID, q });
					inAnim[localNodeID] |= ROTATION_BIT;
				}
				else if (channel.target_path == "scale") {
					frameJointScales.insert({ localNodeID, GetInterpolatedVector<Vector3>(t, indexA, indexB, output, model) });
					inAnim[localNodeID] |= SCALE_BIT;
				}
			}

			int startMatrix = localMatrices.size();

			//We'll assume that nodes aren't animated by default
			for (int i = 0; i < mesh.GetJointCount(); ++i) {
				int localNodeID = i;
				int sceneNodeID = skinData.localToSceneLookup[localNodeID];
				GLTFNode& node = scene.sceneNodes[sceneNodeID];
				localMatrices.push_back(node.worldMatrix);
			}

			for (int i = 0; i < mesh.GetJointCount(); ++i) {
				int localNodeID = i;
				int in = inAnim[localNodeID];

				if (in > 0) { //This node has a modifier of some sort
					int sceneNodeID = skinData.localToSceneLookup[localNodeID];

					GLTFNode& node = scene.sceneNodes[sceneNodeID];

					Vector3 translation;
					Vector3 scale(1, 1, 1);
					Quaternion rotation;

					if (in & TRANSLATION_BIT) {
						translation = frameJointTranslations[localNodeID];
					}
					if (in & ROTATION_BIT) {
						rotation = frameJointRotations[localNodeID];
					}
					if (in & SCALE_BIT) {
						scale = frameJointScales[localNodeID];
					}
					Matrix4 transform = Matrix4::Translation(translation) * 					
						Quaternion::RotationMatrix<Matrix4>(rotation) *
						Matrix4::Scale(scale);

					if (node.parent) {//It's a local transform!
						//We need to work out this frame's matrix for the parent node - may have been animated
						int localParentID = skinData.sceneToLocalLookup[node.parent->nodeID];
						transform = localMatrices[startMatrix + localParentID] * transform;
					}
					localMatrices[startMatrix + i] = transform;
				}
			}
			time += frameRate;
			frameCount++;
		}

		for (int i = 0; i < localMatrices.size(); ++i) {
			localMatrices[i] = skinData.globalTransformInverse * localMatrices[i];
		}

		scene.animations.push_back(std::make_unique<MeshAnimation>((unsigned int)jointCount, frameCount, frameRate, localMatrices));
	}
}