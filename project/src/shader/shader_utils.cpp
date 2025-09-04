#include "shader_utils.h"
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <stdio.h>

static std::string errorString;

static unsigned int compileSourcesToShaderProgram(const std::string& vString, const std::string& fString);
static unsigned int assembleProgram(unsigned int vShader, unsigned int fShader);
static unsigned int compileShader(const GLenum shaderType, const char* shaderCode, const char* shaderTypeString);
static std::string readFile(const std::string& path);
static bool checkProgramLinkingStatus(unsigned int programId, std::string* errorOut);
static bool checkShaderCompilationStatus(unsigned int shaderId, const char* shaderTypeString, std::string* errorOut);

void ShaderUtils::injectData(Shader* shaderPtr, const unsigned int shaderId, const std::string& shaderName)
{
	if (shaderId == 0) return;

	glDeleteProgram(shaderPtr->getNativeHandle());
	shaderPtr->handle = shaderId;
	shaderPtr->shaderName = shaderName;
}

Shader* ShaderUtils::createShaderInternal(const std::string& shaderName, const std::string& vString, const std::string& fString)
{
	Shader* shader = 0;

	printf("Loading '%s' shader program... ", shaderName.c_str());

	try
	{
		shader = new Shader();
		unsigned int newShaderId = compileSourcesToShaderProgram(vString, fString);
		injectData(shader, newShaderId, shaderName);
		printf("\x1b[32mSuccess\x1b[0m\n");
	}
	catch (std::string err)
	{
		delete shader;
		printf("\x1b[31mFailed\n\x1b[33m%s\x1b[0m", err.c_str());
	}

	return shader;
}

void ShaderUtils::loadShader(Shader** shaderPtr, const std::string& shaderName, const std::string& vertexFilePath, const std::string& fragmentFilePath)
{
	validateShaderObject(shaderPtr);

	printf("Loading '%s' shader program... ", shaderName.c_str());

	try
	{
		std::string vString = readFile(vertexFilePath);
		std::string fString = readFile(fragmentFilePath);

		unsigned int newShaderId = compileSourcesToShaderProgram(vString, fString);
		injectData(*shaderPtr, newShaderId, shaderName);
		printf("\x1b[32mSuccess\x1b[0m\n");
	}
	catch (std::string err)
	{
		printf("\x1b[31mFailed\n\x1b[33m%s\x1b[0m", err.c_str());
	}
}

void ShaderUtils::loadShader_String(Shader** shaderPtr, const std::string& shaderName, const std::string& vString, const std::string& fString)
{
	validateShaderObject(shaderPtr);

	printf("Loading '%s' shader program... ", shaderName.c_str());

	try
	{
		unsigned int newShaderId = compileSourcesToShaderProgram(vString, fString);
		injectData(*shaderPtr, newShaderId, shaderName);
		printf("\x1b[32mSuccess\x1b[0m\n");
	}
	catch (std::string err)
	{
		printf("\x1b[31mFailed\n\x1b[33m%s\x1b[0m", err.c_str());
	}
}

void ShaderUtils::loadShader_VFile_FString(Shader** shaderPtr, const std::string& shaderName, const std::string& vertexFilePath, const std::string& fString)
{
	validateShaderObject(shaderPtr);

	printf("Loading '%s' shader program... ", shaderName.c_str());

	try
	{
		std::string vString = readFile(vertexFilePath);

		unsigned int newShaderId = compileSourcesToShaderProgram(vString, fString);
		injectData(*shaderPtr, newShaderId, shaderName);
		printf("\x1b[32mSuccess\x1b[0m\n");
	}
	catch (std::string err)
	{
		printf("\x1b[31mFailed\n\x1b[33m%s\x1b[0m", err.c_str());
	}
}

void ShaderUtils::loadShader_VString_FFile(Shader** shaderPtr, const std::string& shaderName, const std::string& vString, const std::string& fragmentFilePath)
{
	validateShaderObject(shaderPtr);

	printf("Loading '%s' shader program... ", shaderName.c_str());

	try
	{
		std::string fString = readFile(fragmentFilePath);

		unsigned int newShaderId = compileSourcesToShaderProgram(vString, fString);
		injectData(*shaderPtr, newShaderId, shaderName);
		printf("\x1b[32mSuccess\x1b[0m\n");
	}
	catch (std::string err)
	{
		printf("\x1b[31mFailed\n\x1b[33m%s\x1b[0m", err.c_str());
	}
}

static unsigned int compileSourcesToShaderProgram(const std::string& vString, const std::string& fString)
{
	// Clear any data written to the string so we can write errors if any.
	errorString.clear();

	try
	{
		unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vString.c_str(), "VERTEX");
		unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fString.c_str(), "FRAGMENT");

		return assembleProgram(vertexShader, fragmentShader);
	}
	catch (std::string err)
	{
		throw err;
	}
}

static unsigned int assembleProgram(unsigned int vShader, unsigned int fShader)
{
	// Create program
	unsigned int programId = glCreateProgram();

	// Attach shaders to this program and start linking
	glAttachShader(programId, vShader);
	glAttachShader(programId, fShader);

	// You can also look for OpenGL documentation for proper explanation on this part.
	glLinkProgram(programId);

	// if fail, then return 0 (null)
	if (!checkProgramLinkingStatus(programId, &errorString))
	{
		glDeleteProgram(programId);
		throw errorString;
	}

	return programId;
}

static unsigned int compileShader(const GLenum shaderType, const char* shaderCode, const char* shaderTypeString)
{
	// Request Shader to be created.
	unsigned int shaderId = glCreateShader(shaderType);

	// Pass the GLSL code to the Shader created above.
	glShaderSource(shaderId, 1, &shaderCode, NULL);

	// Compile the GLSL code
	glCompileShader(shaderId);

	// Check if compilation succeed or not.
	// If succeed, then return shaderId
	// If fail, delete the Shader, and return 0 (null)
	if (!checkShaderCompilationStatus(shaderId, shaderTypeString, &errorString))
	{
		glDeleteShader(shaderId);
		throw errorString;
	}

	return shaderId;
}

static std::string readFile(const std::string& path)
{
	std::ifstream shaderFile;
	shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	// Try to open the file and read from it.
	// If failed, then ERROR and return 0 (null)
	// If successful, then try CompileShader
	try
	{
		std::stringstream stream;
		shaderFile.open(path);
		stream << shaderFile.rdbuf();
		shaderFile.close();
		return stream.str();
	}
	catch (std::ifstream::failure& e)
	{
		std::ostringstream s;
		s << "ERROR: File " << path.c_str() << " not successfully read!\n" << e.what() << std::endl;
		throw s.str();
	}
}

static bool checkProgramLinkingStatus(unsigned int programId, std::string* errorOut)
{
	int success;
	char infoLog[1024];

	glGetProgramiv(programId, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(programId, 1024, NULL, infoLog);

		std::ostringstream s;
		s << "Error linking program:\n" << infoLog;
		errorOut->append(s.str());
		return false;
	}

	return true;
}

static bool checkShaderCompilationStatus(unsigned int shaderId, const char* shaderTypeString, std::string* errorOut)
{
	int success;
	char infoLog[1024];

	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shaderId, 1024, NULL, infoLog);

		std::ostringstream s;
		s << "Error compiling " << shaderTypeString << " shader:\n" << infoLog;
		errorOut->append(s.str());
		return false;
	}

	return true;
}