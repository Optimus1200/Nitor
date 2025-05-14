#include "Buffers.h"

ntr::FrameBuffer::FrameBuffer()
{
	glGenFramebuffers(1, &mID);
}

ntr::FrameBuffer::~FrameBuffer()
{
	glDeleteFramebuffers(1, &mID);
}

GLuint ntr::FrameBuffer::id() const
{
	return mID;
}

void ntr::FrameBuffer::bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, mID);
}

void ntr::FrameBuffer::unbind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
