#ifndef NTR_IMAGE_H
#define NTR_IMAGE_H

#include <filesystem>

namespace ntr
{
	// Pixels stored in DRAM
	class Image
	{
	public:
		
		static void flipVerticallyOnLoad(bool b);

		Image(const std::filesystem::path& filepath);

		Image(const Image& image)				= delete;
		Image& operator=(const Image& image)	= delete;

		Image(Image&& image) noexcept;
		Image& operator=(Image&& image) noexcept;

		~Image();

		int						channels() const;
		int						height() const;
		const unsigned char*	pixels() const;
		int						width() const;

	private:

		int						mWidth;
		int						mHeight;
		int						mChannels;
		unsigned char*			mPixels;
	};
} // namespace ntr

#endif