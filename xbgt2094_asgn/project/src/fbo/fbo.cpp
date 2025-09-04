#include "fbo.h"
#include <iostream>
#include <string>
#include "../texture/texture_utils.h"

static std::string to_string(DepthFormat fmt)
{
	std::string result;

#pragma warning( push )
#pragma warning( default : 4061)
	switch (fmt)
	{
	case DepthFormat::ZERO: result = "ZERO"; break;
	case DepthFormat::FLOAT16: result = "FLOAT16"; break;
	case DepthFormat::FLOAT24: result = "FLOAT24"; break;
	case DepthFormat::FLOAT32: result = "FLOAT32"; break;
	}
#pragma warning ( pop )

	return result;
}

static std::string to_string(ColourFormat fmt)
{
	std::string result;

#pragma warning( push )
#pragma warning( default : 4061)
	switch (fmt)
	{
	case ColourFormat::RGB: result = "RGB"; break;
	case ColourFormat::RGBA: result = "RGBA"; break;
	case ColourFormat::RGB_16F: result = "RGB_16F"; break;
	case ColourFormat::RGBA_16F: result = "RGBA_16F"; break;
	case ColourFormat::RGB_32F: result = "RGB_32F"; break;
	case ColourFormat::RGBA_32F: result = "RGBA_32F"; break;
	}
#pragma warning ( pop )

	return result;
}

unsigned int FBO::getNativeHandle()
{
	return handle;
}

void FBO::resize(int width, int height)
{
	size.x = width;
	size.y = height;
	resize_internal(width, height);
}

glm::uvec2 FBO::getSize() const
{
	return this->size;
}

ColourDepthFBO::ColourDepthFBO(ColourDepthFrameBufferConfig cfg) : FBO(cfg.size)
{
	this->cfg = cfg;
	create();
}

ColourDepthFBO::~ColourDepthFBO()
{
	for (auto& tex : buffer_ColourTextures)
		delete tex;

	buffer_ColourTextures.clear();

	delete buffer_DepthTexture;

	std::cout << "Deleting framebuffer handle: " << handle << std::endl;
	glDeleteFramebuffers(1, &handle);
}

void ColourDepthFBO::create()
{
	int colourTexturesToGen = cfg.colourAttachments.size();
	if (colourTexturesToGen == 0)
	{
		std::cout << "Error creating ColourDepthFBO: Colour attachments size is 0." << std::endl;
		return;
	}

	glGenFramebuffers(1, &handle);
	glBindFramebuffer(GL_FRAMEBUFFER, handle);

	// Initialize all colour attachments
	buffer_ColourTextures.resize(colourTexturesToGen);

	int width = cfg.size.x;
	int height = cfg.size.y;

	unsigned int* tmp = new unsigned int[colourTexturesToGen];

	for (int i = 0; i < colourTexturesToGen; i++)
	{
		tmp[i] = GL_COLOR_ATTACHMENT0 + i;
		TextureConfig texCfg(TextureWrapMode::CLAMP, TextureWrapMode::CLAMP, cfg.colourAttachments[i].filterMode, false);
		texCfg.internalFormat = (GLint)cfg.colourAttachments[i].format;

		Texture2D* tex = Texture2D::createColourTexture(width, height, texCfg, GL_RGB, nullptr);
		glFramebufferTexture2D(GL_FRAMEBUFFER, tmp[i], GL_TEXTURE_2D, tex->getNativeHandle(), 0);
		buffer_ColourTextures[i] = tex;
	}

	// If we have more than one colour attachment, need to tell OpenGL that we have more than one fragment output.
	glDrawBuffers(colourTexturesToGen, tmp);
	glReadBuffer(GL_BACK);

	// free allocated memory
	delete[] tmp;

	if (cfg.depthFormat != DepthFormat::ZERO)
	{
		buffer_DepthTexture = Texture2D::createDepthTexture(width, height, (GLint)cfg.depthFormat, false);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buffer_DepthTexture->getNativeHandle(), 0);
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	else
		std::cout << "Framebuffer is complete!" << std::endl;

	debugInit();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ColourDepthFBO::debugInit()
{
	int colourTexturesToGen = cfg.colourAttachments.size();
	std::string depthFormat = to_string(cfg.depthFormat);

	std::string fboType;
	if (colourTexturesToGen > 0)
	{
		fboType = "COLOUR ";
		fboType.append(((cfg.depthFormat != DepthFormat::ZERO) ? "AND DEPTH" : "ONLY"));
	}
	else
	{
		fboType = "INVALID";
	}

	std::cout << "ColourDepthFBO handle: " << handle << std::endl
		<< "\tSize            : " << cfg.size.x << "x" << cfg.size.y << std::endl
		<< "\tType            : " << fboType << std::endl
		<< "\tColour textures : " << colourTexturesToGen << std::endl
		<< "\tDepth format    : " << depthFormat << std::endl;
}

Texture2D* ColourDepthFBO::getColourAttachment(unsigned int index)
{
	size_t size = buffer_ColourTextures.size();
	if (size == 0)
	{
		std::cout << "Framebuffer has no colour attachments!" << std::endl;
		return 0;
	}

	if (index >= size)
	{
		std::cout << "Framebuffer colour index out of range. Highest index: " << size - 1 << std::endl;
		return 0;
	}

	return buffer_ColourTextures[index];
}

Texture2D* ColourDepthFBO::getColorAttachment(unsigned int index)
{
	return getColourAttachment(index);
}

const std::vector<Texture2D*>& ColourDepthFBO::getColourAttachments() const
{
	return buffer_ColourTextures;
}

const std::vector<Texture2D*>& ColourDepthFBO::getColorAttachments() const
{
	return buffer_ColourTextures;
}

Texture2D* ColourDepthFBO::getDepthAttachment()
{
	return buffer_DepthTexture;
}

// When window is resized, this needs to be called.
void ColourDepthFBO::resize_internal(int width, int height)
{
	cfg.size.x = width;
	cfg.size.y = height;

	int count = cfg.colourAttachments.size();
	for (int i = 0; i < count; i++)
	{
		Texture2D* tex = buffer_ColourTextures[i];
		GLint fmt = (GLint)cfg.colourAttachments[i].format;

		glBindTexture(GL_TEXTURE_2D, tex->getNativeHandle());
		glTexImage2D(GL_TEXTURE_2D, 0, fmt, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	}

	if (cfg.depthFormat != DepthFormat::ZERO)
	{
		glBindTexture(GL_TEXTURE_2D, buffer_DepthTexture->getNativeHandle());
		glTexImage2D(GL_TEXTURE_2D, 0, (GLint)cfg.depthFormat, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}

DepthFBO::DepthFBO(DepthFrameBufferConfig cfg) : FBO(cfg.size)
{
	this->cfg = cfg;

	create();
}

DepthFBO::~DepthFBO()
{
	delete buffer_DepthTexture;

	std::cout << "Deleting framebuffer handle: " << handle << std::endl;
	glDeleteFramebuffers(1, &handle);
}

void DepthFBO::create()
{
	if (cfg.depthFormat == DepthFormat::ZERO)
	{
		std::cout << "Error creating DepthFBO: cfg.depthFormat cannot be DepthFormat::ZERO" << std::endl;

		return;
	}

	glGenFramebuffers(1, &handle);
	glBindFramebuffer(GL_FRAMEBUFFER, handle);

	int width = cfg.size.x;
	int height = cfg.size.y;
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	buffer_DepthTexture = Texture2D::createDepthTexture(width, height, (GLint)cfg.depthFormat, false);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, buffer_DepthTexture->getNativeHandle(), 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	else
		std::cout << "Framebuffer is complete!" << std::endl;

	debugInit();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DepthFBO::debugInit()
{
	std::string depthFormat = to_string(cfg.depthFormat);

	std::string fboType;
	if (cfg.depthFormat != DepthFormat::ZERO)
	{
		fboType = "DEPTH ONLY";
	}
	else
	{
		fboType = "INVALID";
	}

	std::cout << "DepthFBO handle: " << handle << std::endl
		<< "\tSize            : " << cfg.size.x << "x" << cfg.size.y << std::endl
		<< "\tType            : " << fboType << std::endl
		<< "\tDepth format    : " << depthFormat << std::endl;
}

Texture2D* DepthFBO::getDepthAttachment()
{
	return buffer_DepthTexture;
}

// When window is resized, this needs to be called.
void DepthFBO::resize_internal(int width, int height)
{
	cfg.size.x = width;
	cfg.size.y = height;

	if (cfg.depthFormat != DepthFormat::ZERO)
	{
		glBindTexture(GL_TEXTURE_2D, buffer_DepthTexture->getNativeHandle());
		glTexImage2D(GL_TEXTURE_2D, 0, (GLint)cfg.depthFormat, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
}