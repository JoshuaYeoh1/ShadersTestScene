#pragma once
// Based on LearnOpenGL.com with some changes.
#include <string>
#include <glm/glm.hpp>

class Shader
{
private:
	friend class ShaderUtils;
	std::string shaderName;
	unsigned int handle;
	Shader();

public:
	~Shader();
	unsigned int getNativeHandle();
};
