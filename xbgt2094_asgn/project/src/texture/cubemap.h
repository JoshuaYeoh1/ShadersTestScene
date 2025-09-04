#pragma once
#include <glad/glad.h>

class Cubemap
{
private:

	unsigned int handle;

	Cubemap(unsigned int handle);

public:
	~Cubemap();
	unsigned int getNativeHandle();

	static Cubemap* create(unsigned int handle);
};
