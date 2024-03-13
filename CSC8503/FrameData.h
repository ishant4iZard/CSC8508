#pragma once
#include "BumpAllocator.h"
#include <Buffer.h>

namespace NCL 
{
	namespace CSC8503 
	{
		struct FrameData {
			sce::Agc::Core::Buffer constantBuffer; //Store data that is common for all shaders like projection matrix, camera position
			sce::Agc::Core::Buffer objectBuffer;
			sce::Agc::Core::Buffer debugLineBuffer;
			sce::Agc::Core::Buffer debugTextBuffer;

			BumpAllocator data;

			int globalDataOffset = 0;	//Where does the global data start in the buffer?
			int objectStateOffset = 0;	//Where does the object states start?
			int debugLinesOffset = 0;	//Where do the debug lines start?
			int debugTextOffset = 0;	//Where do the debug text verts start?

			size_t lineVertCount = 0;
			size_t textVertCount = 0;
		};
	}
}