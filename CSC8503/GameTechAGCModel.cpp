#include "GameTechAGCModel.h"
#define SAFE_DELETE(a) if (a != NULL) delete a; a = NULL;
#define SAFE_DELETE_ARRAY(a, MAX) for (auto i = 0; i < MAX; i++) { if (a[i] != NULL) delete a[i]; a[i] = NULL; }
NCL::CSC8503::GameTechAGCModel::GameTechAGCModel()
{
}

NCL::CSC8503::GameTechAGCModel::~GameTechAGCModel()
{
	SAFE_DELETE(pbrVertexShader);
	SAFE_DELETE(pbrPixelShader);
	SAFE_DELETE(defaultTexture);
	SAFE_DELETE(skyboxTexture);
	SAFE_DELETE_ARRAY(computerShaderMap, (int)ComputeShaderName::MAX_SHADER);
	SAFE_DELETE_ARRAY(vertexShaderList, (int)VertexShaderName::MAX_SHADER);
	SAFE_DELETE_ARRAY(pixelShaderList, (int)PixelShaderName::MAX_SHADER);
}
