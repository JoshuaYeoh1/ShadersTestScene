#pragma once
#include <string>
#include <glm/glm.hpp>
#include "mesh.h"

class MeshUtils
{
public:
	static Mesh* makeQuad(float size);
	static Mesh* makeQuad(float width, float height);
	static Mesh* makeEquiTriangle(float edgeLength);
	static Mesh* makeDisk(float radius, int slices);
	static Mesh* makePlane(glm::vec2 size, glm::ivec2 partitions, glm::ivec2 tiling);
	static Mesh* loadObjFile(const std::string& filePath);
	static Mesh* makeSkybox();
};
