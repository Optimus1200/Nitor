#ifndef NTR_MATERIAL_H
#define NTR_MATERIAL_H

#include "Pointer.h"
#include "Texture.h"

namespace ntr
{
	struct Material
	{
		TextureHandle albedo	= Texture::EMPTY;
		TextureHandle normal	= Texture::EMPTY;
		TextureHandle roughness	= Texture::EMPTY;
		TextureHandle metallic	= Texture::EMPTY;
		TextureHandle occlusion	= Texture::EMPTY;

		static const ScopedPointer<Material> EMPTY;
	};
}

#endif
