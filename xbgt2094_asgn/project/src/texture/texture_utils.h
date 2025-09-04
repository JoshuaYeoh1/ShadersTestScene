#pragma once
#include <vector>
#include <string>
#include "texture2d.h"
#include "cubemap.h"

namespace TextureUtils
{
	Texture2D* loadTexture2D(const std::string& path, TextureConfig cfg);
	Texture2D* loadTexture2D(const std::string& path);

	Texture2D* loadTexture2D_sRGBA(const std::string& path, TextureConfig cfg);
	Texture2D* loadTexture2D_sRGBA(const std::string& path);

	Texture2D* blackTexture2D();
	Texture2D* whiteTexture2D();
	Texture2D* checkerTexture2D();

	Cubemap* loadCubemap(const std::string& path, const std::string& extension);
}