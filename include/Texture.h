#ifndef NTR_TEXTURE_HPP
#define NTR_TEXTURE_HPP

#include <filesystem>
#include <string>

#include <assimp/material.h>

#include <glad/glad.h>

#include <glm/vec4.hpp>

#include "Buffers.h"

namespace ntr
{
	using TextureHandle = GLuint;

	enum TextureFilter : GLint
	{
		BILINEAR = GL_LINEAR,
		NEAREST = GL_NEAREST
	};

	// Pixels stored in GPU VRAM.
	class Texture
	{
	public:

		static TextureFilter defaultFilter;

		static constexpr TextureHandle EMPTY = 0;

		Texture();

		Texture(const std::filesystem::path& filepath, TextureFilter filter = defaultFilter);
		Texture(int width, int height, const glm::vec4& color, TextureFilter filter = defaultFilter);

		Texture(const Texture& texture)				= delete;
		Texture& operator=(const Texture& texture)	= delete;

		Texture(Texture&& texture)				noexcept;
		Texture& operator=(Texture&& texture)	noexcept;

		~Texture();

		int						channels() const;
		TextureFilter			filter() const;
		int						height() const;
		TextureHandle			handle() const;
		int						width() const;

	private:

		GLuint					mID;
		int						mWidth;
		int						mHeight;
		int						mChannels;
		TextureFilter			mFilter;

		void init(GLenum format, const unsigned char* pixels);
	};

	class DepthTexture2D
	{
	public:

		DepthTexture2D(const GLint size);
		DepthTexture2D(const DepthTexture2D& texture) = delete;
		DepthTexture2D& operator=(const DepthTexture2D& texture) = delete;
		~DepthTexture2D();

		GLuint fbo() const;
		GLuint id() const;

	private:

		GLuint mFBO;
		GLuint mID;
	};

	class Texture2DArray
	{
	public:

		Texture2DArray(GLint resolution, size_t size);
		Texture2DArray(const Texture2DArray& arr) = delete;
		Texture2DArray& operator=(const Texture2DArray& arr) = delete;
		~Texture2DArray();

		GLuint id() const;

	private:

		GLuint mID;
	};

	class DepthTexture3D
	{
	public:

		float nearPlane = 1.0f;
		float farPlane = 25.0f;

		DepthTexture3D(const GLint& size);
		DepthTexture3D(const DepthTexture3D& texture) = delete;
		DepthTexture3D& operator=(const DepthTexture3D& texture) = delete;
		~DepthTexture3D();

		GLuint fbo() const;
		GLuint id() const;

	private:

		GLuint mFBO;
		GLuint mID;
	};
} // namespace ntr

#endif