#pragma once

#include "Types.h"

#include <stb_image.h>

class Texture {
public:
	int width;
	int height;
	int numChannels;
	stbi_uc* pixels;

	// TODO: to make it deletable from outside of Texture scope. should not be needed.
	AllocatedImage newImage;
public:
	bool LoadImageFromFile(const char* file);
};