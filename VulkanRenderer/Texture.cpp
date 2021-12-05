#include "Texture.h"

#include "Core/Log.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

bool Texture::LoadImageFromFile(const char* file) {
	pixels = stbi_load(file, &width, &height, &numChannels, STBI_rgb_alpha);

	if (!pixels) {
		Log::Error("Failed to load texture file {}",  file);
		return false;
	}

	return true;
}
