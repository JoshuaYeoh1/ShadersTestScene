#pragma once
#include <vector>
#include <glm/glm.hpp>

struct DebugVertex
{
	glm::vec3 position;
	glm::vec4 colour;

	DebugVertex();
	DebugVertex(glm::vec3 position);
	DebugVertex(glm::vec3 position, glm::vec3 colour);
	DebugVertex(glm::vec3 position, glm::vec4 colour);
};

struct DebugMesh
{
	friend class DebugMeshUtils;

protected:
	unsigned int VAO;
	unsigned int VBO;
	unsigned int vertexCount;
	std::vector<DebugVertex> vertices;

	DebugMesh() :VAO(0), VBO(0), vertexCount(0) {};
	DebugMesh(std::vector<DebugVertex> vertices);
	void setup();
public:
	void draw() const;
};

struct DebugMeshCone : public DebugMesh
{
	friend class DebugMeshUtils;

protected:
	DebugMeshCone(float angle, float range, const glm::vec3& color);

public:
	void changeParams(float angle, float range);
};

class DebugMeshUtils
{
public:
	static DebugMesh* makeGrid(const int halfSize = 5);
	static DebugMesh* makeAxis();
	static DebugMesh* makeWireBox(const glm::vec3& color);
	static DebugMesh* makeWireSphere(const glm::vec3& color);

	static DebugMeshCone* makeCone(float angle, float range, const glm::vec3& color);
};