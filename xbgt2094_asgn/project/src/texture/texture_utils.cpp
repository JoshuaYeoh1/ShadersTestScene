#include "texture_utils.h"
#include <glad/glad.h>
#include <stb_image/stb_image.h>
#include <iostream>

namespace TextureUtils
{
	Texture2D* loadTexture2D(const std::string& path, TextureConfig cfg)
	{
		stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.

		Texture2D* tex = 0;

		static int width, height, nrChannels;
		unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 4);
		if (data)
		{
			cfg.internalFormat = GL_RGBA;
			tex = Texture2D::createColourTexture(width, height, cfg, GL_RGBA, data);
			std::cout << "Loaded texture: " << path << std::endl;
		}
		else
		{
			std::cout << "Failed to load texture: " << path << std::endl;
		}

		// Free memory after we send the data to GPU.
		stbi_image_free(data);

		return tex;
	}

	Texture2D* loadTexture2D(const std::string& path)
	{
		return loadTexture2D(path, TextureConfig());
	}

	Texture2D* loadTexture2D_sRGBA(const std::string& path, TextureConfig cfg)
	{
		stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.

		Texture2D* tex = 0;

		static int width, height, nrChannels;
		unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 4);
		if (data)
		{
			cfg.internalFormat = GL_SRGB_ALPHA;
			tex = Texture2D::createColourTexture(width, height, cfg, GL_RGBA, data);
			std::cout << "Loaded texture (sRGBA): " << path << std::endl;
		}
		else
		{
			std::cout << "Failed to load texture (sRGBA): " << path << std::endl;
		}

		// Free memory after we send the data to GPU.
		stbi_image_free(data);

		return tex;
	}

	Texture2D* loadTexture2D_sRGBA(const std::string& path)
	{
		return loadTexture2D_sRGBA(path, TextureConfig());
	}

	Texture2D* blackTexture2D()
	{
		static unsigned char data[4] = { 0x00, 0x00, 0x00, 0xff };
		static Texture2D* blank = Texture2D::createColourTexture(1, 1,
			TextureConfig(TextureWrapMode::REPEAT, TextureWrapMode::REPEAT, TextureFilterMode::NEAREST, false),
			GL_RGBA, data);

		return blank;
	}

	Texture2D* whiteTexture2D()
	{
		static unsigned char data[4] = { 0xff, 0xff, 0xff, 0xff };
		static Texture2D* blank = Texture2D::createColourTexture(1, 1,
			TextureConfig(TextureWrapMode::REPEAT, TextureWrapMode::REPEAT, TextureFilterMode::NEAREST, false),
			GL_RGBA, data);

		return blank;
	}

	Texture2D* checkerTexture2D()
	{
		static unsigned char data[16] = {
			0xff, 0x00, 0xff, 0xff, 0x70, 0x00, 0x70, 0xff,
			0x70, 0x00, 0x70, 0xff, 0xff, 0x00, 0xff, 0xff
		};
		static Texture2D* blank = Texture2D::createColourTexture(2, 2,
			TextureConfig(TextureWrapMode::REPEAT, TextureWrapMode::REPEAT, TextureFilterMode::NEAREST, false),
			GL_RGBA, data);

		return blank;
	}

	Cubemap* loadCubemap(const std::string& path, const std::string& extension)
	{
		Cubemap* cm = 0;

		static std::string suffixes[6] = { "_xpos", "_xneg", "_ypos","_yneg", "_zpos", "_zneg" };

		std::cout << "Loading cubemap..." << std::endl;
		unsigned int textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		bool noError = true;
		static int width, height, nrChannels;
		for (unsigned int i = 0; i < 6; i++)
		{
			std::string filePath = path + suffixes[i] + "." + extension;
			std::cout << "\tLoading texture: " << filePath << "... ";
			unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 4);
			if (data)
			{
				std::cout << "Success" << std::endl;
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			}
			else
			{
				std::cout << "Failed!" << std::endl;
				noError = false;
				break;
			}
			stbi_image_free(data);
		}

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		if (noError)
		{
			cm = Cubemap::create(textureID);
		}
		else
		{
			glDeleteTextures(1, &textureID);
		}

		return cm;
	}
}