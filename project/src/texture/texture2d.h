#pragma once
#include <glad/glad.h>

enum class TextureWrapMode
{
	REPEAT,
	REPEAT_MIRRORED,
	CLAMP
};

enum class TextureFilterMode
{
	NEAREST,
	LINEAR
};

// This struct is to provide means to control texture settings when loading texture
struct TextureConfig
{
	GLint hWrap;
	GLint vWrap;
	GLint textureFilter;
	GLint internalFormat;

	bool mipmap;
	bool isDepth;

	TextureConfig()
		: hWrap(GL_REPEAT), vWrap(GL_REPEAT), textureFilter(GL_LINEAR), mipmap(false), isDepth(false) {}
	TextureConfig(TextureWrapMode wrapHorizontal, TextureWrapMode wrapVertical, TextureFilterMode filter, bool enableMipmap)
		: mipmap(enableMipmap), isDepth(false)
	{
		switch (wrapHorizontal)
		{
		case TextureWrapMode::REPEAT: hWrap = GL_REPEAT; break;
		case TextureWrapMode::REPEAT_MIRRORED: hWrap = GL_MIRRORED_REPEAT; break;
		case TextureWrapMode::CLAMP: hWrap = GL_CLAMP_TO_EDGE; break;
		}
		switch (wrapVertical)
		{
		case TextureWrapMode::REPEAT: vWrap = GL_REPEAT; break;
		case TextureWrapMode::REPEAT_MIRRORED: vWrap = GL_MIRRORED_REPEAT; break;
		case TextureWrapMode::CLAMP: vWrap = GL_CLAMP_TO_EDGE; break;
		}
		switch (filter)
		{
		case TextureFilterMode::NEAREST: textureFilter = GL_NEAREST; break;
		case TextureFilterMode::LINEAR: textureFilter = GL_LINEAR; break;
		}

		internalFormat = GL_RGBA;
	}
};

class Texture2D
{
private:

	TextureConfig cfg;
	bool mipmap;
	int width, height;
	unsigned int handle;

	Texture2D(int width, int height, TextureConfig cfg);
	Texture2D(int width, int height);

public:
	~Texture2D();

	bool hasMipMap();
	GLint getWrapModeHorizontal();
	GLint getWrapModeVertical();
	GLint getTextureFilter();
	void getSize(int* w, int* h);
	GLint getInternalFormat();

	unsigned int getNativeHandle();

	static Texture2D* createColourTexture(int width, int height, TextureConfig cfg, GLenum format, unsigned char* data);
	static Texture2D* createDepthTexture(int width, int height, GLint bits, bool hasBorder);
	static Texture2D* createFromNativeHandle(unsigned int handle);

	static Texture2D* copyColourTexture(Texture2D* source);
	static Texture2D* copyColourTexture(unsigned int handle);
};
