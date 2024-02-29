#pragma once
#include <OGLTexture.h>
#include <iostream>
namespace NCL
{
	namespace CSC8503
	{
		class OGLDefferedLightFbo
		{
		public:
			OGLDefferedLightFbo(const int& inWidht, const int& inHeight)
			{
				width = inWidht;
				height = inHeight;
				CreateDefferedLightFrameBuffer();
			}

			~OGLDefferedLightFbo()
			{
				glDeleteFramebuffers(1, &fbo);
			}

			GLuint GetFbo() { return fbo; }
			GLuint GetdiffuseLightTex() { return diffuseLightTex; }
			GLuint GetSpecularLightTex() { return specularLightTex; }
		protected:
			bool CreateDefferedLightFrameBuffer()
			{
				if (!CreateLightTex(diffuseLightTex)) return false;
				if (!CreateLightTex(specularLightTex)) return false;

				glGenFramebuffers(1, &fbo);

				return BindHdrFrameBuffer(fbo);
			}

			bool BindHdrFrameBuffer(GLuint& inFbo)
			{
				glBindFramebuffer(GL_FRAMEBUFFER, inFbo);

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
					GL_TEXTURE_2D, diffuseLightTex, 0);

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
					GL_TEXTURE_2D, specularLightTex, 0);

				GLenum draw_buffer[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
				glDrawBuffers(2, draw_buffer);

				GLenum status;
				status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
				switch (status)
				{
				case GL_FRAMEBUFFER_COMPLETE:
					std::cout << "[OGLDefferedLightFbo::BindHdrFrameBuffer] FrameBuffer generated successfully." << std::endl;
					break;
				default:
					std::cout << "[OGLDefferedLightFbo::BindHdrFrameBuffer] Failed to generate framebuffer." << std::endl;
					return false;
				}
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				return  true;
			}

			bool CreateLightTex(GLuint& outBuffer)
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
		protected:
			GLuint fbo;
			GLuint diffuseLightTex;
			GLuint specularLightTex;
			int width;
			int height;
		};
	}
}