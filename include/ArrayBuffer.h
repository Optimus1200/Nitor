#ifndef NTR_ARRAY_BUFFER_H
#define NTR_ARRAY_BUFFER_H

#include <glad/glad.h>

namespace ntr
{
	// Stores an array inside a Shader Storage Buffer Object (SSBO)
	template <typename T>
	class ArrayBuffer
	{
	public:

		GLuint binding;

		ArrayBuffer(size_t size, const T* data = {}, GLuint binding = 0, GLenum usage = GL_DYNAMIC_COPY);
		ArrayBuffer(const ArrayBuffer& arr) = delete;
		ArrayBuffer& operator=(const ArrayBuffer& arr) = delete;
		~ArrayBuffer();

		size_t size() const;

		void bind() const;
		void unbind() const;
		// Updates data in the range [start, end).
		void update(size_t start, size_t end, const T* data);
		void update(size_t index, const T& data);

	private:

		GLuint	mID;
		size_t	mSize;
	};
}

#include "ArrayBuffer.hpp"

#endif
