/*
Part of Newcastle University's Game Engineering source code.

Use as you see fit!

Comments and queries to: richard-gordon.davison AT ncl.ac.uk
https://research.ncl.ac.uk/game/
*/
#pragma once

using std::vector;

namespace NCL::Maths {
	class Vector2i;
	class Vector3i;
	class Vector4i;

	class Vector2;
	class Vector3;
	class Vector4;

	class Matrix4;
};

namespace NCL::Rendering {
	class Mesh;

	class MshLoader	{

	enum class GeometryChunkTypes {
		VPositions = 1 << 0,
		VNormals = 1 << 1,
		VTangents = 1 << 2,
		VColors = 1 << 3,
		VTex0 = 1 << 4,
		VTex1 = 1 << 5,
		VWeightValues = 1 << 6,
		VWeightIndices = 1 << 7,
		Indices = 1 << 8,
		JointNames = 1 << 9,
		JointParents = 1 << 10,
		BindPose = 1 << 11,
		BindPoseInv = 1 << 12,
		Material = 1 << 13,
		SubMeshes = 1 << 14,
		SubMeshNames = 1 << 15
	};

	enum class GeometryChunkData {
		dFloat, //Just float data
		dShort, //Translate from -32k to 32k to a float
		dByte,	//Translate from -128 to 127 to a float
	};

	public:		
		static bool LoadMesh(const std::string& filename, Mesh& destinationMesh);

	protected:
		static void* ReadVertexData(GeometryChunkData dataType, GeometryChunkTypes chunkType, int numVertices);
		static void ReadTextInts(std::ifstream& file, vector<Maths::Vector2i>& element, int numVertices);
		static void ReadTeReadTextIntsxtFloats(std::ifstream& file, vector<Maths::Vector3i>& element, int numVertices);
		static void ReadTextInts(std::ifstream& file, vector<Maths::Vector4i>& element, int numVertices);
		static void ReadTextFloats(std::ifstream& file, vector<Maths::Vector2>& element, int numVertices);
		static void ReadTextFloats(std::ifstream& file, vector<Maths::Vector3>& element, int numVertices);
		static void ReadTextFloats(std::ifstream& file, vector<Maths::Vector4>& element, int numVertices);
		static void ReadIntegers(std::ifstream& file, vector<unsigned int>& elements, int intCount);

		static void ReadRigPose(std::ifstream& file, vector<Maths::Matrix4>& into);
		static void ReadJointParents(std::ifstream& file, std::vector<int>& parentIDs);
		static void ReadJointNames(std::ifstream& file, std::vector<std::string>& names);
		static void ReadSubMeshes(std::ifstream& file, int count, std::vector<struct SubMesh>& subMeshes);
		static void ReadSubMeshNames(std::ifstream& file, int count, std::vector<std::string>& names);

		MshLoader() {}
		~MshLoader() {}
	};
}