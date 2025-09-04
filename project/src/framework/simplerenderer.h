#pragma once
#include "../shader/shader.h"
#include "../mesh/mesh.h"
#include "../texture/texture2d.h"
#include "../texture/cubemap.h"
#include "../fbo/fbo.h"

class SimpleRenderer
{
public:
	SimpleRenderer() = delete;

	static void bindShader(Shader* shader);

	static void setShaderProp_Bool(const std::string& name, bool v);
	static void setShaderProp_Integer(const std::string& name, int i);
	static void setShaderProp_UnsignedInteger(const std::string& name, unsigned int i);
	static void setShaderProp_Float(const std::string& name, float f);

	static void setShaderProp_Vec2(const std::string& name, const glm::vec2& v);
	static void setShaderProp_Vec2(const std::string& name, float x, float y);
	static void setShaderProp_Vec3(const std::string& name, const glm::vec3& v);
	static void setShaderProp_Vec3(const std::string& name, float x, float y, float z);
	static void setShaderProp_Vec4(const std::string& name, const glm::vec4 v);
	static void setShaderProp_Vec4(const std::string& name, float x, float y, float z, float w);

	static void setShaderProp_Mat2(const std::string& name, const glm::mat2& mat);
	static void setShaderProp_Mat3(const std::string& name, const glm::mat3& mat);
	static void setShaderProp_Mat4(const std::string& name, const glm::mat4& mat);

	static void setTexture_0(Texture2D* texture);
	static void setTexture_1(Texture2D* texture);
	static void setTexture_2(Texture2D* texture);
	static void setTexture_3(Texture2D* texture);
	static void setTexture_4(Texture2D* texture);
	static void setTexture_5(Texture2D* texture);
	static void setTexture_6(Texture2D* texture);
	static void setTexture_7(Texture2D* texture);
	static void setTexture_X(int id, Texture2D* texture);
	static void setTexture_X(int id, DepthFBO* depthFBO);

	static void setTexture_skybox(Cubemap* cubemap);

	static void drawMesh(Mesh* mesh);

	static void bindFBO(FBO* fbo);
	static void bindFBO_Default();
};
