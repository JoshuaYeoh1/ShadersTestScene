#pragma once
#include <vector>
#include <glm/glm.hpp>

// Adapted from https://learnopengl.com/Model-Loading/Assimp
// allows sequential layout, making mesh setup easier
struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
	glm::vec3 colour;
	glm::vec4 tangent;

	Vertex();
	Vertex(glm::vec3 position);
	Vertex(glm::vec3 position, glm::vec3 normal);
	Vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 uv);
	Vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 uv, glm::vec3 colour);
};

class Mesh
{
	friend class SimpleRenderer;
	friend class MeshUtils;
public:
	std::vector<Vertex> vertices;

	~Mesh();

private:
	unsigned int VAO, VBO;

	Mesh();
	Mesh(std::vector<Vertex> vertices);
	void setup();
};
