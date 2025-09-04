#include "texture2d.h"
#include <iostream>

static void getTextureConfig(unsigned int handle, TextureConfig* cfg, int* width, int* height)
{
	GLint minFilter;

	glBindTexture(GL_TEXTURE_2D, handle);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, height);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &cfg->internalFormat);
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &cfg->hWrap);
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, &cfg->vWrap);
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &cfg->textureFilter);
	cfg->isDepth = cfg->internalFormat == GL_DEPTH_COMPONENT16 ||
		cfg->internalFormat == GL_DEPTH_COMPONENT24 ||
		cfg->internalFormat == GL_DEPTH_COMPONENT32 ||
		cfg->internalFormat == GL_DEPTH_COMPONENT32F;

	// hack to set if texture has mipmap set or not
	// only works because of createColourTexture works.
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &minFilter);
	cfg->mipmap = minFilter != GL_LINEAR && minFilter != GL_NEAREST;
}

Texture2D::Texture2D(int width, int height, TextureConfig cfg) : width(width), height(height), cfg(cfg) {}
Texture2D::Texture2D(int width, int height) : width(width), height(height) {}

Texture2D::~Texture2D()
{
	glDeleteTextures(1, &handle);
}

bool Texture2D::hasMipMap()
{
	return mipmap;
}

GLint Texture2D::getWrapModeHorizontal()
{
	return cfg.hWrap;
}

GLint Texture2D::getWrapModeVertical()
{
	return cfg.vWrap;
}

GLint Texture2D::getTextureFilter()
{
	return cfg.textureFilter;
}

void Texture2D::getSize(int* w, int* h)
{
	*w = width;
	*h = height;
}

GLint Texture2D::getInternalFormat()
{
	return cfg.internalFormat;
}

unsigned int Texture2D::getNativeHandle()
{
	return handle;
}

Texture2D* Texture2D::createColourTexture(int width, int height, TextureConfig cfg, GLenum format, unsigned char* data)
{
	Texture2D* tex = new Texture2D(width, height, cfg);

	glGenTextures(1, &(tex->handle));
	glBindTexture(GL_TEXTURE_2D, (tex->handle));

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, cfg.hWrap);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, cfg.vWrap);

	// With mipmapping, Minification Filter needs to use the mipmap version.
	// Otherwise, use the standard version.
	int minFilter = GL_NEAREST;
	switch (cfg.textureFilter)
	{
	default:
	case GL_NEAREST: minFilter = cfg.mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST; break;
	case GL_LINEAR: minFilter = cfg.mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR; break;
	}

	// Set Minification and Magnification filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, cfg.textureFilter);

	glTexImage2D(GL_TEXTURE_2D, 0, cfg.internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);

	if (cfg.mipmap)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	return tex;
}

Texture2D* Texture2D::createDepthTexture(int width, int height, GLint bits, bool hasBorder)
{
	GLint wrapMode = (hasBorder ? GL_CLAMP_TO_BORDER : GL_CLAMP_TO_EDGE);
	Texture2D* tex = new Texture2D(width, height);
	tex->cfg.hWrap = wrapMode;
	tex->cfg.vWrap = wrapMode;
	tex->cfg.textureFilter = GL_NEAREST;
	tex->cfg.mipmap = false;
	tex->cfg.internalFormat = bits;
	tex->cfg.isDepth = true;

	glGenTextures(1, &(tex->handle));
	glBindTexture(GL_TEXTURE_2D, (tex->handle));

	if (hasBorder)
	{
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, bits, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);
	return tex;
}

// Textures created by directly calling OpenGL API can be converted to Texture2D instance.
Texture2D* Texture2D::createFromNativeHandle(unsigned int handle)
{
	if (handle == 0) return nullptr;

	int width, height;
	TextureConfig cfg;
	GLint minFilter;

	glBindTexture(GL_TEXTURE_2D, handle);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &cfg.internalFormat);
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, &cfg.hWrap);
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, &cfg.vWrap);
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, &cfg.textureFilter);
	cfg.isDepth = cfg.internalFormat == GL_DEPTH_COMPONENT16 ||
		cfg.internalFormat == GL_DEPTH_COMPONENT24 ||
		cfg.internalFormat == GL_DEPTH_COMPONENT32 ||
		cfg.internalFormat == GL_DEPTH_COMPONENT32F;

	// hack to set if texture has mipmap set or not
	// only works because of createColourTexture works.
	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, &minFilter);
	cfg.mipmap = minFilter != GL_LINEAR && minFilter != GL_NEAREST;

	Texture2D* tex = new Texture2D(width, height, cfg);
	tex->handle = handle;

	glBindTexture(GL_TEXTURE_2D, 0);
	return tex;
}

// https://stackoverflow.com/questions/16100308/how-to-copy-texture1-to-texture2-efficiently
// WARNING: This creates A COPY, YOU NEED TO DELETE THIS AT THE END OF RENDER CALL!
// OTHERWISE YOU WILL GET MEMORY LEAK.
Texture2D* Texture2D::copyColourTexture(Texture2D* source)
{
	if (source && source->getNativeHandle() != 0)
	{
		return copyColourTexture(source->getNativeHandle());
	}
	else
	{
		std::cout << "Texture2D::copyColourTexture error!" << std::endl;
		return 0;
	}
}

// WARNING: This creates A COPY, YOU NEED TO DELETE THIS AT THE END OF RENDER CALL!
// OTHERWISE YOU WILL GET MEMORY LEAK.
Texture2D* Texture2D::copyColourTexture(unsigned int handle)
{
	if (handle != 0)
	{
		TextureConfig cfg;
		int w, h;

		getTextureConfig(handle, &cfg, &w, &h);

		if (cfg.isDepth)
		{
			std::cout << "Texture2D::copyColourTexture error!" << std::endl;
			return nullptr;
		}

		Texture2D* tex = Texture2D::createColourTexture(w, h, cfg, GL_RGBA, nullptr);

		static unsigned int fbo = 0;
		if (fbo == 0)
		{
			glGenFramebuffers(1, &fbo);
		}

		GLint curFBO;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &curFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, handle, 0);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, tex->getNativeHandle(), 0);
		glDrawBuffer(GL_COLOR_ATTACHMENT1);
		glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER, curFBO);
		return tex;
	}
	else
	{
		std::cout << "Texture2D::copyColourTexture error!" << std::endl;
		return nullptr;
	}
}