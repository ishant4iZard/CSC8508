#pragma once
#include <cstdint>
#include <string.h>

namespace NCL 
{
	namespace CSC8503 
	{
		/*
		Handling buffers in AGC isn't too bad, as they are a small wrapper around an existing
		memory allocation. Here I have a small struct that will fill out a memory allocation with
		all of the data required by the frame. We can then make Buffers out of this at any
		offset we want to send to our shaders - in this case we're going to use one bug allocation
		to hold both the constants used by shaders, as well as all of the debug vertices, and object
		matrices. No fancy suballocations here, the allocator is as simple as it gets - it just
		advances or 'bumps' a pointer along. Perfect for recording a frame's data to memory!
		*/
		struct BumpAllocator {
			char* dataStart;//Start of our allocated memory
			char* data;		//Current write point of our memory
			size_t			bytesWritten;

			template<typename T>
			void WriteData(T value) {
				memcpy(data, &value, sizeof(T));
				data += sizeof(T);
				bytesWritten += sizeof(T);
			}
			void WriteData(void* inData, size_t byteCount) {
				memcpy(data, inData, byteCount);
				data += byteCount;
				bytesWritten += byteCount;
			}

			void AlignData(size_t alignment) {
				char* oldData = data;
				data = (char*)((((uintptr_t)data + alignment - 1) / (uintptr_t)alignment) * (uintptr_t)alignment);
				bytesWritten += data - oldData;
			}

			void Reset() {
				bytesWritten = 0;
				data = dataStart;
			}
		};
	}
}