#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "../texture/texture2d.h"

enum class DepthFormat
{
	ZERO = 0,
	FLOAT16 = GL_DEPTH_COMPONENT16,
	FLOAT24 = GL_DEPTH_COMPONENT24,
	FLOAT32 = GL_DEPTH_COMPONENT32
};

enum class ColourFormat
{
	RGB = GL_RGB,
	RGBA = GL_RGBA,
	RGB_16F = GL_RGB16F,
	RGBA_16F = GL_RGBA16F,
	RGB_32F = GL_RGB32F,
	RGBA_32F = GL_RGBA32F
};

struct ColourAttachmentData
{
public:
	ColourFormat format;
	TextureFilterMode filterMode;

	ColourAttachmentData(ColourFormat fmt, TextureFilterMode filter) : format(fmt), filterMode(filter) {}
};

struct FrameBufferConfig
{
	glm::uvec2 size;
};

struct DepthFrameBufferConfig : public FrameBufferConfig
{
	DepthFormat depthFormat;
};

struct ColourDepthFrameBufferConfig : public DepthFrameBufferConfig
{
	std::vector<ColourAttachmentData> colourAttachments;
};

class FBO
{
private:
	glm::uvec2 size;

protected:
	unsigned int handle;

	virtual void create() = 0;
	virtual void resize_internal(int width, int height) = 0;

	FBO(glm::uvec2 s) : size(s) {}

public:
	unsigned int getNativeHandle();
	virtual void resize(int width, int height);
	glm::uvec2 getSize() const;
};

class ColourDepthFBO : public FBO
{
	friend class FBOUtils;

	ColourDepthFrameBufferConfig cfg;

	std::vector<Texture2D*> buffer_ColourTextures;
	Texture2D* buffer_DepthTexture;

	void debugInit();
	void create() override;

	ColourDepthFBO(ColourDepthFrameBufferConfig cfg);

protected:
	void resize_internal(int width, int height);

public:
	~ColourDepthFBO();

	Texture2D* getColourAttachment(unsigned int index);
	Texture2D* getColorAttachment(unsigned int index);
	const std::vector<Texture2D*>& getColourAttachments() const;
	const std::vector<Texture2D*>& getColorAttachments() const;
	Texture2D* getDepthAttachment();
};

class DepthFBO : public FBO
{
	friend class FBOUtils;

	DepthFrameBufferConfig cfg;

	Texture2D* buffer_DepthTexture;

	void debugInit();
	void create() override;

	DepthFBO(DepthFrameBufferConfig cfg);

protected:
	void resize_internal(int width, int height);

public:
	~DepthFBO();

	Texture2D* getDepthAttachment();
};