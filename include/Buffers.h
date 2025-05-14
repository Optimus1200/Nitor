#ifndef NTR_BUFFERS_H
#define NTR_BUFFERS_H

#include <glad/glad.h>

namespace ntr
{
	class FrameBuffer
	{
	public:

		FrameBuffer();
		FrameBuffer(const FrameBuffer& fb) = delete;
		FrameBuffer& operator=(const FrameBuffer& fb) = delete;
		~FrameBuffer();

		GLuint id() const;

		void bind() const;
		void unbind() const;

	private:

		GLuint mID;
	};
}

#endif
