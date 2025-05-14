#include <filesystem>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Image.h"

namespace ntr
{
    void Image::flipVerticallyOnLoad(bool b)
    {
        stbi_set_flip_vertically_on_load(b);
    }

    Image::Image(const std::filesystem::path& filepath)
        : mPixels{ stbi_load(filepath.string().c_str(), &mWidth, &mHeight, &mChannels, 0) }
	{

	}

    Image::Image(Image&& image) noexcept
        : mWidth{ std::move(image.mWidth) }
        , mHeight{ std::move(image.mHeight) }
        , mChannels{ std::move(image.mChannels) }
        , mPixels{ std::move(image.mPixels) }
    {
        image.mPixels = nullptr;
    }

    Image& Image::operator=(Image&& image) noexcept
    {
        std::swap(mWidth, image.mWidth);
        std::swap(mHeight, image.mHeight);
        std::swap(mChannels, image.mChannels);
        std::swap(mPixels, image.mPixels);

        return *this;
    }

	Image::~Image()
	{
        stbi_image_free(mPixels);
	}

    int Image::channels() const
    {
        return mChannels;
    }

    int Image::height() const
    {
        return mHeight;
    }

    const unsigned char* Image::pixels() const
    {
        return mPixels;
    }

    int Image::width() const
    {
        return mWidth;
    }
} // namespace ntr