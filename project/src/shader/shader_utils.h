#pragma once
#include "shader.h"

class ShaderUtils
{
	friend class SceneBase;
	friend class LightDebug;

private:
	static void injectData(Shader* shader, const unsigned int shaderId, const std::string& shaderName);

	static inline void validateShaderObject(Shader** shaderPtr)
	{
		if (*shaderPtr == 0)
		{
			*shaderPtr = new Shader();
		}
	}

	static Shader* createShaderInternal(const std::string& shaderName, const std::string& vString, const std::string& fString);

	static void loadShader_String(Shader** shaderPtr, const std::string& shaderName, const std::string& vString, const std::string& fString);
	static void loadShader_VFile_FString(Shader** shaderPtr, const std::string& shaderName, const std::string& vertexFilePath, const std::string& fString);
	static void loadShader_VString_FFile(Shader** shaderPtr, const std::string& shaderName, const std::string& vString, const std::string& fragmentFilePath);

public:
	static void loadShader(Shader** shaderPtr, const std::string& shaderName, const std::string& vertexFilePath, const std::string& fragmentFilePath);
};