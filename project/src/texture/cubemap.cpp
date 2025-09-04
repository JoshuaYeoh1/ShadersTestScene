#include <iostream>
#include "cubemap.h"

Cubemap::Cubemap(unsigned int handle) : handle(handle) {}

Cubemap::~Cubemap()
{
	glDeleteTextures(1, &handle);
}

unsigned int Cubemap::getNativeHandle()
{
	return handle;
}

Cubemap* Cubemap::create(unsigned int handle)
{
	return new Cubemap(handle);
}