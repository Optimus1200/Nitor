#include <iostream>
#include <vector>

#include "Image.h"
#include "Texture.h"

namespace ntr
{
	TextureFilter Texture::defaultFilter = TextureFilter::BILINEAR;

	//#################################################################################################
	//
	// TEXTURE IMPLEMENTATION
	//
	//#################################################################################################

	Texture::Texture()
		: mID{ 0 }
		, mWidth{ 0 }
		, mHeight{ 0 }
		, mChannels{ 0 }
		, mFilter{ defaultFilter }
	{
	}

	Texture::Texture(const std::filesystem::path& filepath, TextureFilter filter)
		: mFilter{ filter }
	{
		Image image(filepath);

		mWidth = image.width();
		mHeight = image.height();
		mChannels = image.channels();

		GLenum format = 0;

		if (mChannels == 1)
		{
			format = GL_RED;
		}
		else if (mChannels == 3)
		{
			format = GL_RGB;
		}
		else if (mChannels == 4)
		{
			format = GL_RGBA;
		}

		init(format, image.pixels());
	}

	Texture::Texture(int width, int height, const glm::vec4& color, TextureFilter filter)
		: mFilter{ filter }
		, mWidth{ width }
		, mHeight{ height }
		, mChannels{ 4 }
	{
		const size_t SIZE = static_cast<size_t>(width * height * mChannels);
		std::vector<unsigned char> pixels(SIZE);

		for (size_t i = 0; i < SIZE; i+= 4)
		{
			pixels[i]	= static_cast<unsigned char>(color.x);
			pixels[i+1]	= static_cast<unsigned char>(color.y);
			pixels[i+2]	= static_cast<unsigned char>(color.z);
			pixels[i+3]	= static_cast<unsigned char>(color.w);
		}

		init(GL_RGBA, pixels.data());
	}
	
	Texture::Texture(Texture&& texture) noexcept
		: mID{ std::move(texture.mID) }
		, mWidth{ std::move(texture.mWidth) }
		, mHeight{ std::move(texture.mHeight) }
		, mChannels{ std::move(texture.mChannels) }
		, mFilter{ std::move(texture.mFilter) }
	{
		texture.mID = 0;
		texture.mWidth = 0;
		texture.mHeight = 0;
	}

	Texture& Texture::operator=(Texture&& texture) noexcept
	{
		std::swap(mID, texture.mID);
		std::swap(mWidth, texture.mWidth);
		std::swap(mHeight, texture.mHeight);
		std::swap(mChannels, texture.mChannels);
		std::swap(mFilter, texture.mFilter);

		return *this;
	}

	Texture::~Texture()
	{
		glDeleteTextures(1, &mID);
	}

	int Texture::channels() const
	{
		return mChannels;
	}

	TextureFilter Texture::filter() const
	{
		return mFilter;
	}

	int Texture::height() const
	{
		return mHeight;
	}

	TextureHandle Texture::handle() const
	{
		return mID;
	}

	int Texture::width() const
	{
		return mWidth;
	}

	void Texture::init(GLenum format, const unsigned char* pixels)
	{
		glGenTextures(1, &mID);

		glBindTexture(GL_TEXTURE_2D, mID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, mWidth, mHeight, 0, format, GL_UNSIGNED_BYTE, pixels);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mFilter);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	//#################################################################################################
	//
	// DEPTH TEXTURE 2D IMPLEMENTATION
	//
	//#################################################################################################

	DepthTexture2D::DepthTexture2D(const GLint size)
	{
		glGenTextures(1, &mID);
		glBindTexture(GL_TEXTURE_2D, mID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		float borderColor[4] = { 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGenFramebuffers(1, &mFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mID, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	DepthTexture2D::~DepthTexture2D()
	{
		glDeleteFramebuffers(1, &mFBO);
		glDeleteTextures(1, &mID);
	}

	GLuint DepthTexture2D::fbo() const
	{
		return mFBO;
	}

	GLuint DepthTexture2D::id() const
	{
		return mID;
	}

	//#################################################################################################
	//
	// TEXTURE ARRAY IMPLEMENTATION
	//
	//#################################################################################################

	Texture2DArray::Texture2DArray(GLint resolution, size_t size)
	{
		glGenTextures(1, &mID);
		glBindTexture(GL_TEXTURE_2D_ARRAY, mID);
		glTexImage3D(
			GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT32F, resolution, resolution, (GLsizei)size,
			0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		const float BORDER_COLOR[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, BORDER_COLOR);
	}

	Texture2DArray::~Texture2DArray()
	{
		glDeleteTextures(1, &mID);
	}

	GLuint Texture2DArray::id() const
	{
		return mID;
	}

	//#################################################################################################
	//
	// DEPTH TEXTURE 3D IMPLEMENTATION
	//
	//#################################################################################################

	DepthTexture3D::DepthTexture3D(const GLint& size)
	{
		glGenTextures(1, &mID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, mID);
		
		for (GLuint i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glGenFramebuffers(1, &mFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, mID, 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	DepthTexture3D::~DepthTexture3D()
	{
		glDeleteFramebuffers(1, &mFBO);
		glDeleteTextures(1, &mID);
	}

	GLuint DepthTexture3D::fbo() const
	{
		return mFBO;
	}

	GLuint DepthTexture3D::id() const
	{
		return mID;
	}

} // namespace ntr