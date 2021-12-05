#pragma once

#include "Types.h"

#include <stb_image.h>

class Texture {
public:
	int width;
	int height;
	int numChannels;
	stbi_uc* pixels;

public:
	bool LoadImageFromFile(const char* file);
};