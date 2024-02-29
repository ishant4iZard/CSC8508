#include "OGLGBuffer.h"
#include <cstddef>
#include <iostream>
using namespace NCL;
using namespace CSC8503;

bool OGLGBuffer::CreateFrameBuffer()
{
	glGenFramebuffers(1, &fbo);

	if (!CreateTextureBuffer(diffuseMettalic, GL_RGBA16F, GL_RGBA, GL_FLOAT)) return false;
	if (!CreateTextureBuffer(normalRoughness, GL_RGBA16F, GL_RGBA, GL_FLOAT)) return false;
	if (!CreateTextureBuffer(baseReflectiveAmbientOcclusion, GL_RGBA, GL_RGBA, GL_FLOAT)) return false;

	if (!CreateTextureBuffer(depthStencil, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8)) return false;

	return BindFrameBuffer();
}

bool OGLGBuffer::CreateTextureBuffer(GLuint& outBuffer, const GLuint& inBitSpace, const GLuint& inBufferType, const GLuint& inDataType)
{
	glGenTextures(1, &outBuffer);
	glBindTexture(GL_TEXTURE_2D, outBuffer);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, inBitSpace, widht, height,
		0, inBufferType, inDataType, NULL);

	if (!outBuffer) return false;
	return true;
}

bool OGLGBuffer::BindFrameBuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D, depthStencil, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
		GL_TEXTURE_2D, depthStencil, 0);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D, diffuseMettalic, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
		GL_TEXTURE_2D, normalRoughness, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2,
		GL_TEXTURE_2D, baseReflectiveAmbientOcclusion, 0);

	GLenum draw_buffer[3] = { GL_COLOR_ATTACHMENT0,  GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
	glDrawBuffers(3, draw_buffer);

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

OGLGBuffer::OGLGBuffer(const int& inWidth, const int& inHeight)
{
	widht = inWidth;
	height = inHeight;
	CreateFrameBuffer();
}

OGLGBuffer::~OGLGBuffer()
{
	glDeleteFramebuffers(1, &fbo);
	//TODO delete textures
}