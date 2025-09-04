#include <glad/glad.h>
#include <iostream>
#include "debugmesh.h"

DebugVertex::DebugVertex()
	: position(0.0f), colour(1.0f) {}
DebugVertex::DebugVertex(glm::vec3 position)
	: position(position), colour(1.0f) {}
DebugVertex::DebugVertex(glm::vec3 position, glm::vec3 colour)
	:position(position), colour(glm::vec4(colour, 1.0f)) {}
DebugVertex::DebugVertex(glm::vec3 position, glm::vec4 colour)
	:position(position), colour(colour) {}

DebugMesh::DebugMesh(std::vector<DebugVertex> vertices) :vertices(vertices)
{
	setup();
}

void DebugMesh::setup()
{
	this->vertexCount = vertices.size();

	glGenVertexArrays(1, &this->VAO);
	glBindVertexArray(this->VAO);

	glGenBuffers(1, &this->VBO);
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

	// Upload mesh data to the GPU
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(DebugVertex), &vertices[0], GL_STATIC_DRAW);

	// Specify the layout of the vertices we just uploaded
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)0);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, colour));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void DebugMesh::draw() const
{
	if (VAO != 0) {
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, vertexCount);
	}
	else {
		std::cout << "Mesh not set!" << std::endl;
	}

	glBindVertexArray(0);
}

DebugMeshCone::DebugMeshCone(float angle, float range, const glm::vec3& color)
{
	float radius = sin(glm::radians(angle * 0.5f)) * range;

	static int slices = 24;
	static float slice_radians = 6.2831853f / slices;
	for (int i = 0; i < slices; i++)
	{
		float x0 = cos(slice_radians * i);
		float y0 = sin(slice_radians * i);
		float x1 = cos(slice_radians * (i + 1));
		float y1 = sin(slice_radians * (i + 1));

		vertices.insert(vertices.end(),
			{
				DebugVertex({0.0f, 0.0f, 0.0f}, color), DebugVertex({ x0 * radius, y0 * radius, range}, color),
				DebugVertex({x0 * radius, y0 * radius, range}, color), DebugVertex({ x1 * radius, y1 * radius, range}, color)
			});
	}

	vertexCount = vertices.size();

	setup();
}

void DebugMeshCone::changeParams(float angle, float range)
{
	float radius = sin(glm::radians(angle * 0.5f)) * range;

	static int slices = 24;
	static float slice_radians = 6.2831853f / slices;
	for (int i = 0; i < slices; i++)
	{
		float x0 = cos(slice_radians * i);
		float y0 = sin(slice_radians * i);
		float x1 = cos(slice_radians * (i + 1));
		float y1 = sin(slice_radians * (i + 1));

		int ii = i * 4;
		vertices[1 + ii].position = { x0 * radius, y0 * radius, range };
		vertices[2 + ii].position = { x0 * radius, y0 * radius, range };
		vertices[3 + ii].position = { x1 * radius, y1 * radius, range };
	}

	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(DebugVertex), &vertices[0]);
}

DebugMesh* DebugMeshUtils::makeGrid(const int halfSize)
{
	std::vector<DebugVertex> vertices;

	for (int i = -halfSize; i < halfSize; i++)
	{
		for (int j = -halfSize; j <= halfSize; j++)
		{
			float a0 = i;
			float a1 = i + 1;
			float b0 = j;
			float b1 = j + 1;

			vertices.insert(vertices.end(),
				{
					DebugVertex({a0, 0.0f, b0},{0.5f, 0.5f, 0.5f, 1.0f}),
					DebugVertex({a1, 0.0f, b0},{0.5f, 0.5f, 0.5f, 1.0f}),
					DebugVertex({b0, 0.0f, a0},{0.5f, 0.5f, 0.5f, 1.0f}),
					DebugVertex({b0, 0.0f, a1},{0.5f, 0.5f, 0.5f, 1.0f})
				});
		}
	}

	return new DebugMesh(vertices);
}

DebugMesh* DebugMeshUtils::makeAxis()
{
	std::vector<DebugVertex> vertices;

	vertices.insert(vertices.end(),
		{
			DebugVertex({0.0f, 0.0f, 0.0f},{1.0f, 0.3f, 0.3f, 1.0f}),
			DebugVertex({1.0f, 0.0f ,0.0f},{1.0f, 0.3f, 0.3f, 1.0f}),
			DebugVertex({0.0f, 0.0f, 0.0f},{0.3f, 1.0f, 0.3f, 1.0f}),
			DebugVertex({0.0f, 1.0f ,0.0f},{0.3f, 1.0f, 0.3f, 1.0f}),
			DebugVertex({0.0f, 0.0f, 0.0f},{0.3f, 0.3f, 1.0f, 1.0f}),
			DebugVertex({0.0f, 0.0f ,1.0f},{0.3f, 0.3f, 1.0f, 1.0f}),
		});

	return new DebugMesh(vertices);
}

DebugMesh* DebugMeshUtils::makeWireBox(const glm::vec3& color)
{
	std::vector<DebugVertex> vertices;

	vertices.insert(vertices.end(),
		{
			DebugVertex({-1.0f, -1.0f, -1.0f}, color), DebugVertex({ 1.0f, -1.0f, -1.0f}, color),
			DebugVertex({-1.0f,  1.0f, -1.0f}, color), DebugVertex({ 1.0f,  1.0f, -1.0f}, color),
			DebugVertex({-1.0f, -1.0f, -1.0f}, color), DebugVertex({-1.0f,  1.0f, -1.0f}, color),
			DebugVertex({ 1.0f, -1.0f, -1.0f}, color), DebugVertex({ 1.0f,  1.0f, -1.0f}, color),
			DebugVertex({-1.0f, -1.0f,  1.0f}, color), DebugVertex({ 1.0f, -1.0f,  1.0f}, color),
			DebugVertex({-1.0f,  1.0f,  1.0f}, color), DebugVertex({ 1.0f,  1.0f,  1.0f}, color),
			DebugVertex({-1.0f, -1.0f,  1.0f}, color), DebugVertex({-1.0f,  1.0f,  1.0f}, color),
			DebugVertex({ 1.0f, -1.0f,  1.0f}, color), DebugVertex({ 1.0f,  1.0f,  1.0f}, color),
			DebugVertex({-1.0f, -1.0f, -1.0f}, color), DebugVertex({-1.0f, -1.0f,  1.0f}, color),
			DebugVertex({ 1.0f, -1.0f, -1.0f}, color), DebugVertex({ 1.0f, -1.0f,  1.0f}, color),
			DebugVertex({-1.0f,  1.0f, -1.0f}, color), DebugVertex({-1.0f,  1.0f,  1.0f}, color),
			DebugVertex({ 1.0f,  1.0f, -1.0f}, color), DebugVertex({ 1.0f,  1.0f,  1.0f}, color),
		});

	return new DebugMesh(vertices);
}

DebugMesh* DebugMeshUtils::makeWireSphere(const glm::vec3& color)
{
	std::vector<DebugVertex> vertices;

	static int slices = 128;
	static float slice_radians = 6.2831853f / slices;
	for (int i = 0; i < slices; i++)
	{
		float x0 = cos(slice_radians * i);
		float y0 = sin(slice_radians * i);
		float x1 = cos(slice_radians * (i + 1));
		float y1 = sin(slice_radians * (i + 1));

		vertices.insert(vertices.end(),
			{
				DebugVertex({  x0,   y0, 0.0f}, color), DebugVertex({  x1,   y1, 0.0f}, color),
				DebugVertex({  x0, 0.0f,   y0}, color), DebugVertex({  x1, 0.0f,   y1}, color),
				DebugVertex({0.0f,   y0,   x0}, color), DebugVertex({0.0f,   y1,   x1}, color),
			});
	}

	return new DebugMesh(vertices);
}

DebugMeshCone* DebugMeshUtils::makeCone(float angle, float range, const glm::vec3& color)
{
	return new DebugMeshCone(angle, range, color);
}