#include "shader.h"
#include <glad/glad.h>
#include "shader_utils.h"

static std::string error;

Shader::Shader() : shaderName("NO-NAME"), handle(0)
{
}

Shader::~Shader()
{
	glDeleteProgram(handle);
}

unsigned int Shader::getNativeHandle()
{
	return handle;
}