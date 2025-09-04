#include "simplerenderer.h"
#include "simpleapp.h"
#include <glad/glad.h>
#include <iostream>

static Shader* currentShader;
static unsigned int handle;

void SimpleRenderer::bindShader(Shader* shader)
{
	if (shader != nullptr)
	{
		handle = shader->getNativeHandle();
		glUseProgram(handle);
	}
	else
	{
		handle = 0;
		glUseProgram(0);
	}

	currentShader = shader;
}

void SimpleRenderer::setShaderProp_Bool(const std::string& name, bool v)
{
	glUniform1i(glGetUniformLocation(handle, name.c_str()), (int)v);
}

void SimpleRenderer::setShaderProp_Integer(const std::string& name, int i)
{
	glUniform1i(glGetUniformLocation(handle, name.c_str()), i);
}

void SimpleRenderer::setShaderProp_UnsignedInteger(const std::string& name, unsigned int i)
{
	glUniform1ui(glGetUniformLocation(handle, name.c_str()), i);
}

void SimpleRenderer::setShaderProp_Float(const std::string& name, float f)
{
	glUniform1f(glGetUniformLocation(handle, name.c_str()), f);
}

void SimpleRenderer::setShaderProp_Vec2(const std::string& name, const glm::vec2& v)
{
	glUniform2fv(glGetUniformLocation(handle, name.c_str()), 1, &v[0]);
}

void SimpleRenderer::setShaderProp_Vec2(const std::string& name, float x, float y)
{
	glUniform2f(glGetUniformLocation(handle, name.c_str()), x, y);
}

void SimpleRenderer::setShaderProp_Vec3(const std::string& name, const glm::vec3& v)
{
	glUniform3fv(glGetUniformLocation(handle, name.c_str()), 1, &v[0]);
}

void SimpleRenderer::setShaderProp_Vec3(const std::string& name, float x, float y, float z)
{
	glUniform3f(glGetUniformLocation(handle, name.c_str()), x, y, z);
}

void SimpleRenderer::setShaderProp_Vec4(const std::string& name, const glm::vec4 v)
{
	glUniform4fv(glGetUniformLocation(handle, name.c_str()), 1, &v[0]);
}

void SimpleRenderer::setShaderProp_Vec4(const std::string& name, float x, float y, float z, float w)
{
	glUniform4f(glGetUniformLocation(handle, name.c_str()), x, y, z, w);
}

void SimpleRenderer::setShaderProp_Mat2(const std::string& name, const glm::mat2& mat)
{
	glUniformMatrix2fv(glGetUniformLocation(handle, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void SimpleRenderer::setShaderProp_Mat3(const std::string& name, const glm::mat3& mat)
{
	glUniformMatrix3fv(glGetUniformLocation(handle, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void SimpleRenderer::setShaderProp_Mat4(const std::string& name, const glm::mat4& mat)
{
	glUniformMatrix4fv(glGetUniformLocation(handle, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void SimpleRenderer::setTexture_0(Texture2D* texture)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture->getNativeHandle());
}

void SimpleRenderer::setTexture_1(Texture2D* texture)
{
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture->getNativeHandle());
}

void SimpleRenderer::setTexture_2(Texture2D* texture)
{
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texture->getNativeHandle());
}

void SimpleRenderer::setTexture_3(Texture2D* texture)
{
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, texture->getNativeHandle());
}

void SimpleRenderer::setTexture_4(Texture2D* texture)
{
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, texture->getNativeHandle());
}

void SimpleRenderer::setTexture_5(Texture2D* texture)
{
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, texture->getNativeHandle());
}

void SimpleRenderer::setTexture_6(Texture2D* texture)
{
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, texture->getNativeHandle());
}

void SimpleRenderer::setTexture_7(Texture2D* texture)
{
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, texture->getNativeHandle());
}

void SimpleRenderer::setTexture_X(int id, Texture2D* texture)
{
	// Ensure the index is within the valid range for texture units (0 to GL_TEXTURE31)
	if (id < 0 || id > 31) {
		std::cerr << "Error: Texture unit index out of range (0-31)." << std::endl;
		return;
	}

	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_2D, texture->getNativeHandle());
}

void SimpleRenderer::setTexture_X(int id, DepthFBO* depthFBO)
{
	// Ensure the index is within the valid range for texture units (0 to GL_TEXTURE31)
	if (id < 0 || id > 31) {
		std::cerr << "Error: Texture unit index out of range (0-31)." << std::endl;
		return;
	}

	glActiveTexture(GL_TEXTURE0 + id);
	glBindTexture(GL_TEXTURE_2D, depthFBO->getNativeHandle());
}

void SimpleRenderer::setTexture_skybox(Cubemap* cubemap)
{
	if (cubemap == 0)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		return;
	}
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->getNativeHandle());
}

void SimpleRenderer::drawMesh(Mesh* mesh)
{
	unsigned int VAO = 0;

	if (mesh != nullptr)
	{
		VAO = mesh->VAO;
	}

	if (VAO != 0) {
		unsigned int vSize = mesh->vertices.size();
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, vSize);
		glBindVertexArray(0);
	}
	else {
		std::cout << "Mesh not set!" << std::endl;
	}
}

void SimpleRenderer::bindFBO(FBO* fbo)
{
	if (fbo != 0)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, fbo->getNativeHandle());

		// the viewport matrix needs to follow the width and height of the FBO
		// so we resize the viewport
		auto size = fbo->getSize();

		glViewport(0, 0, size.x, size.y);
		return;
	}

	SimpleRenderer::bindFBO_Default();
}

void SimpleRenderer::bindFBO_Default()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// default framebuffer uses the window size
	auto size = App::getViewportSize();
	glViewport(0, 0, size.x, size.y);
}