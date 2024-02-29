#pragma once
#include <OGLTexture.h>
namespace NCL
{
	namespace CSC8503
	{
		class OGLGBuffer
		{
		protected:
			GLuint fbo;
			/// <summary>
			/// Store albedo in RGB and metal in Alpha
			/// </summary>
			GLuint diffuseMettalic;
			GLuint normalRoughness;
			GLuint baseReflectiveAmbientOcclusion;
			GLuint depthStencil;

			int widht;
			int height;
		protected:
			bool CreateFrameBuffer();
			bool CreateTextureBuffer(GLuint& outBuffer, const GLuint& inBitSpace, const GLuint& inBufferType, const GLuint& inDataType);
			bool BindFrameBuffer();
		public:
			OGLGBuffer(const int& inWidth, const int& inHeight);
			~OGLGBuffer();

			GLuint GetFBO() { return fbo; }
			GLuint GetDiffuseMettalic() { return diffuseMettalic; }
			GLuint GetNormalRoughness() { return normalRoughness; }
			GLuint GetBaseRefectivityAO() { return baseReflectiveAmbientOcclusion; }
			GLuint GetDepth() { return depthStencil; }
		};
	}
}