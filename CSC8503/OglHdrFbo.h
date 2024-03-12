#pragma once
#include <OGLTexture.h>
#include <iostream>
namespace NCL
{
	namespace CSC8503
	{
		class OglHdrFbo
		{
		public:
			OglHdrFbo(const int& inWidht, const int& inHeight) 
			{
				width = inWidht;
				height = inHeight;
				CreateHdrFrameBuffer();
			}
			~OglHdrFbo()
			{
				glDeleteFramebuffers(1, &fbo);
			}
			GLuint GetFbo() { return fbo; }
			GLuint GetColorBuffer() { return colorBuffer; }
		protected:
			bool CreateHdrFrameBuffer()
			{
				if (!CreateHdrColorBuffer(colorBuffer)) return false;
				if (!CreateDepthBuffer(depthBuffer)) return false;

				glGenFramebuffers(1, &fbo);

				return BindHdrFrameBuffer(fbo);
			}

			bool BindHdrFrameBuffer(GLuint& inFbo)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, inFbo);

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
					GL_TEXTURE_2D, depthBuffer, 0);
				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
					GL_TEXTURE_2D, depthBuffer, 0);

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					GL_TEXTURE_2D, colorBuffer, 0);

				GLenum draw_buffer[1] = { GL_COLOR_ATTACHMENT0 };
				glDrawBuffers(1, draw_buffer);

				GLenum status;
				status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
				switch (status)
				{
				case GL_FRAMEBUFFER_COMPLETE:
					std::cout << "[OglHdrFbo::BindHdrFrameBuffer] FrameBuffer generated successfully." << std::endl;
					break;
				default:
					std::cout << "[OglHdrFbo::BindHdrFrameBuffer] Failed to generate framebuffer." << std::endl;
					return false;
				}
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				return  true;
			}

			bool CreateHdrColorBuffer(GLuint& outBuffer)
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
				glBindTexture(GL_TEXTURE_2D, 0);
				if (!outBuffer) return false;
				return true;
			}

			bool CreateDepthBuffer(GLuint& outBuffer)
			{
				glGenTextures(1, &outBuffer);
				glBindTexture(GL_TEXTURE_2D, outBuffer);

				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

				glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8,
					width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
				glBindTexture(GL_TEXTURE_2D, 0);

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