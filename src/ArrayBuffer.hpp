#ifndef NTR_ARRAY_BUFFER_HPP
#define NTR_ARRAY_BUFFER_HPP

#include "ArrayBuffer.h"

namespace ntr
{
	template<typename T>
	inline ArrayBuffer<T>::ArrayBuffer(size_t size, const T* data, GLuint binding, GLenum usage)
		: mSize{ size }
		, binding{ binding }
	{
		glGenBuffers(1, &mID);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, mID);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(T) * size, data, usage);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, mID);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
	
	template<typename T>
	inline ArrayBuffer<T>::~ArrayBuffer()
	{
		glDeleteBuffers(1, &mID);
	}

	template<typename T>
	inline size_t ArrayBuffer<T>::size() const
	{
		return mSize;
	}
	
	template<typename T>
	inline void ArrayBuffer<T>::bind() const
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, mID);
	}
	
	template<typename T>
	inline void ArrayBuffer<T>::unbind() const
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}
	
	template<typename T>
	inline void ArrayBuffer<T>::update(size_t start, size_t end, const T* data)
	{
		GLintptr	offset	= sizeof(T) * start;
		GLsizeiptr	size	= sizeof(T) * (end - start);

		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
	}

	template<typename T>
	inline void ArrayBuffer<T>::update(size_t index, const T& data)
	{
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(T) * index, sizeof(T), &data);
	}
}

#endif