#pragma once
#include <OGLTexture.h>
#include <iostream>
namespace NCL
{
	namespace CSC8503
	{
		class OglPostProcessingFbo
		{
		public:
			OglPostProcessingFbo(const int& inWidht, const int& inHeight)
			{
				width = inWidht;
				height = inHeight;
				CreateFrameBuffer();
			}
			~OglPostProcessingFbo()
			{
				glDeleteFramebuffers(1, &fbo);
			}
			GLuint GetFbo() { return fbo; }
			GLuint GetColorBuffer() const { return colorBuffer; }
		protected:
			bool CreateFrameBuffer()
			{
				glGenFramebuffers(1, &fbo);

				if (!CreateColorBuffer(colorBuffer)) return false;

				return BindHdrFrameBuffer(fbo);
			}

			bool CreateColorBuffer(GLuint& outBuffer)
			{
				glGenTextures(1, &outBuffer);
				glBindTexture(GL_TEXTURE_2D, outBuffer);

				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

				//Setting internal format as GL_RGBA32F let's us store information from 0-255 
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0,
					GL_RGBA, GL_FLOAT, NULL);

				if (!outBuffer) return false;
				return true;
			}

			bool BindHdrFrameBuffer(GLuint& inFbo)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, inFbo);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					GL_TEXTURE_2D, colorBuffer, 0);

				GLenum draw_buffer[1] = { GL_COLOR_ATTACHMENT0 };
				glDrawBuffers(1, draw_buffer);

				GLenum status;
				status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
				switch (status)
				{
				case GL_FRAMEBUFFER_COMPLETE:
					std::cout << "[Renderer::CreateHdrFrameBuffer] FrameBuffer generated successfully." << std::endl;
					break;
				default:
					std::cout << "[Renderer::CreateHdrFrameBuffer] Failed to generate framebuffer." << std::endl;
					return false;
				}
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				return  true;
			}

			bool CreateDepthBuffer(GLuint& outBuffer)
			{
				glGenTextures(1, &outBuffer);
				glBindTexture(GL_TEXTURE_2D, outBuffer);

				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

				glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height,
					0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

				if (!outBuffer) return false;
				return true;
			}
		protected:
			GLuint fbo;
			GLuint colorBuffer;
			GLuint depthBuffer;
			int width;
			int height;
		};
	}
}