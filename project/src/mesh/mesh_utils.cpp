#include "mesh_utils.h"
#include <glad/glad.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>
#include <iostream>
#include <glm/gtc/constants.hpp>

Mesh* MeshUtils::makeQuad(float size)
{
	if (size <= 0) size = 1.0f;
	float halfSize = size * 0.5;

	std::vector<Vertex> vertices = {
		Vertex({-halfSize,  halfSize, 0.0f},{0.0f, 0.0f, 1.0f},{0.0f, 1.0f}, {1,1,1}),
		Vertex({-halfSize, -halfSize, 0.0f},{0.0f, 0.0f, 1.0f},{0.0f, 0.0f}, {1,1,1}),
		Vertex({ halfSize,  halfSize, 0.0f},{0.0f, 0.0f, 1.0f},{1.0f, 1.0f}, {1,1,1}),
		Vertex({ halfSize,  halfSize, 0.0f},{0.0f, 0.0f, 1.0f},{1.0f, 1.0f}, {1,1,1}),
		Vertex({-halfSize, -halfSize, 0.0f},{0.0f, 0.0f, 1.0f},{0.0f, 0.0f}, {1,1,1}),
		Vertex({ halfSize, -halfSize, 0.0f},{0.0f, 0.0f, 1.0f},{1.0f, 0.0f}, {1,1,1}),
	};

	return new Mesh(vertices);
}

Mesh* MeshUtils::makeQuad(float width, float height)
{
	if (width <= 0) width = 1.0f;
	if (height <= 0) height = 1.0f;

	float halfW = width * 0.5;
	float halfH = height * 0.5;

	std::vector<Vertex> vertices = {
		Vertex({-halfW,  halfH, 0.0f},{0.0f, 0.0f, 1.0f},{0.0f, 1.0f}, {1,1,1}),
		Vertex({-halfW, -halfH, 0.0f},{0.0f, 0.0f, 1.0f},{0.0f, 0.0f}, {1,1,1}),
		Vertex({ halfW,  halfH, 0.0f},{0.0f, 0.0f, 1.0f},{1.0f, 1.0f}, {1,1,1}),
		Vertex({ halfW,  halfH, 0.0f},{0.0f, 0.0f, 1.0f},{1.0f, 1.0f}, {1,1,1}),
		Vertex({-halfW, -halfH, 0.0f},{0.0f, 0.0f, 1.0f},{0.0f, 0.0f}, {1,1,1}),
		Vertex({ halfW, -halfH, 0.0f},{0.0f, 0.0f, 1.0f},{1.0f, 0.0f}, {1,1,1}),
	};

	return new Mesh(vertices);
}

Mesh* MeshUtils::makeEquiTriangle(float edgeLength)
{
	std::vector<Vertex> vertices = {
		Vertex({0.0f,  0.8660254f * edgeLength, 0.0f},{0.0f, 0.0f, 1.0f},{0.5f, 0.8660254f}, { 1.0f, 1.0f, 1.0f }),
		Vertex({-0.5f * edgeLength, 0.0f, 0.0f},{0.0f, 0.0f, 1.0f},{0.0f, 0.0f}, { 1.0f, 1.0f, 1.0f }),
		Vertex({0.5f * edgeLength, 0.0f, 0.0f},{0.0f, 0.0f, 1.0f},{1.0f, 0.0f}, { 1.0f, 1.0f, 1.0f })
	};

	return new Mesh(vertices);
}

Mesh* MeshUtils::makeDisk(float radius, int slices)
{
	if (radius <= 0.0f) radius = 1.0f;
	if (slices < 3) slices = 3;

	std::vector<Vertex> vertices;

	float theta = 2.0f / slices * glm::pi<float>();
	for (int i = 0; i < slices; i++) {
		float cos0 = cosf(i * theta);
		float sin0 = sinf(i * theta);
		float cos1 = cosf((i + 1) * theta);
		float sin1 = sinf((i + 1) * theta);
		vertices.push_back(Vertex({ 0.0f,0.0f,0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f }));
		vertices.push_back(Vertex({ radius * cos0, radius * sin0, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.5f + 0.5f * cos0, 0.5f + 0.5f * sin0 }, { 1.0f, 1.0f, 1.0f }));
		vertices.push_back(Vertex({ radius * cos1, radius * sin1, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.5f + 0.5f * cos1, 0.5f + 0.5f * sin1 }, { 1.0f, 1.0f, 1.0f }));
	}

	return new Mesh(vertices);
}

Mesh* MeshUtils::makePlane(glm::vec2 size, glm::ivec2 partitions, glm::ivec2 tiling)
{
	if (partitions.x == 0) partitions.x = 1;
	if (partitions.y == 0) partitions.y = 1;
	if (size.x <= 0) size.x = 1.0f;
	if (size.y <= 0) size.y = size.x;

	float width = size.x, depth = size.y;
	float columns = partitions.x, rows = partitions.y;

	float cellW = width / columns;
	float cellD = depth / rows;

	float halfWidth = width * 0.5f, halfDepth = depth * 0.5f;

	unsigned int triCount = columns * rows * 6;

	std::vector<float> vertices(triCount * 3);
	std::vector<float> normals(triCount * 3);
	std::vector<float> texcoords(triCount * 2);

	std::vector<Vertex> vertexes(triCount);

	size_t v = -1;

	float rowRecip = 1.0f / rows;
	float colRecip = 1.0f / columns;

	for (int h = 0; h < rows; h++)
	{
		for (int w = 0; w < columns; w++)
		{
			float u0 = tiling.x * (w * colRecip);
			float v0 = tiling.y * (h * rowRecip);
			float u1 = tiling.x * ((w + 1) * colRecip);
			float v1 = tiling.y * ((h + 1) * rowRecip);
			float w0x = cellW * w;
			float h0y = cellD * h;
			float w1x = cellW * (w + 1);
			float h1y = cellD * (h + 1);

			unsigned int vertex1 = ++v;
			vertexes[vertex1].position = { -halfWidth + w0x ,0.0f, halfDepth - h1y };
			vertexes[vertex1].uv = { u0, v1 };
			vertexes[vertex1].normal = { 0.0f, 1.0f, 0.0f };

			unsigned int vertex2 = ++v;
			vertexes[vertex2].position = { -halfWidth + w0x ,0.0f, halfDepth - h0y };
			vertexes[vertex2].uv = { u0, v0 };
			vertexes[vertex2].normal = { 0.0f, 1.0f, 0.0f };

			unsigned int vertex3 = ++v;
			vertexes[vertex3].position = { -halfWidth + w1x ,0.0f, halfDepth - h1y };
			vertexes[vertex3].uv = { u1, v1 };
			vertexes[vertex3].normal = { 0.0f, 1.0f, 0.0f };

			unsigned int vertex4 = ++v;
			Vertex v3 = vertexes[vertex3];
			vertexes[vertex4].position = v3.position;
			vertexes[vertex4].uv = v3.uv;
			vertexes[vertex4].normal = v3.normal;

			unsigned int vertex5 = ++v;
			Vertex v2 = vertexes[vertex2];
			vertexes[vertex5].position = v2.position;
			vertexes[vertex5].uv = v2.uv;
			vertexes[vertex5].normal = v2.normal;

			unsigned int vertex6 = ++v;
			vertexes[vertex6].position = { -halfWidth + w1x ,0.0f, halfDepth - h0y };
			vertexes[vertex6].uv = { u1, v0 };
			vertexes[vertex6].normal = { 0.0f, 1.0f, 0.0f };
		}
	}

	return new Mesh(vertexes);
}

Mesh* MeshUtils::loadObjFile(const std::string& filePath)
{
	tinyobj::ObjReaderConfig reader_config;
	reader_config.mtl_search_path = "";
	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(filePath, reader_config)) {
		if (!reader.Error().empty()) {
			std::cerr << "TinyObjReader: " << reader.Error();
		}

		return 0;
	}
	if (!reader.Warning().empty()) {
		std::cout << "TinyObjReader: " << reader.Warning();
	}

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	auto& materials = reader.GetMaterials();

	std::vector<Vertex> vertices;

	// Loop over shapes
	for (size_t s = 0; s < shapes.size(); s++)
	{
		// Loop over faces(polygon)
		size_t index_offset = 0;

		int i = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
		{
			size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++)
			{
				Vertex vertex;
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
				tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
				tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

				vertex.position = { vx,vy,vz };

				if (idx.normal_index >= 0) {
					tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
					tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
					tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];

					vertex.normal = { nx,ny,nz };
				}

				if (idx.texcoord_index >= 0) {
					tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
					tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];

					vertex.uv = { tx,ty };
				}

				// Optional: vertex colors
				tinyobj::real_t r = attrib.colors[3 * size_t(idx.vertex_index) + 0];
				tinyobj::real_t g = attrib.colors[3 * size_t(idx.vertex_index) + 1];
				tinyobj::real_t b = attrib.colors[3 * size_t(idx.vertex_index) + 2];

				vertex.colour = { r,g,b };

				vertices.push_back(vertex);
			}

			i++;
			index_offset += fv;
		}
	}

	Mesh* mesh = new Mesh(vertices);
	return mesh;
}

Mesh* MeshUtils::makeSkybox()
{
	static std::vector<Vertex> vertices =
	{
		{{-1.0f,  1.0f, -1.0f}},
		{{-1.0f, -1.0f, -1.0f}},
		{{ 1.0f, -1.0f, -1.0f}},
		{{ 1.0f, -1.0f, -1.0f}},
		{{ 1.0f,  1.0f, -1.0f}},
		{{-1.0f,  1.0f, -1.0f}},

		{{-1.0f, -1.0f,  1.0f}},
		{{-1.0f, -1.0f, -1.0f}},
		{{-1.0f,  1.0f, -1.0f}},
		{{-1.0f,  1.0f, -1.0f}},
		{{-1.0f,  1.0f,  1.0f}},
		{{-1.0f, -1.0f,  1.0f}},

		{{ 1.0f, -1.0f, -1.0f}},
		{{ 1.0f, -1.0f,  1.0f}},
		{{ 1.0f,  1.0f,  1.0f}},
		{{ 1.0f,  1.0f,  1.0f}},
		{{ 1.0f,  1.0f, -1.0f}},
		{{ 1.0f, -1.0f, -1.0f}},

		{{-1.0f, -1.0f,  1.0f}},
		{{-1.0f,  1.0f,  1.0f}},
		{{ 1.0f,  1.0f,  1.0f}},
		{{ 1.0f,  1.0f,  1.0f}},
		{{ 1.0f, -1.0f,  1.0f}},
		{{-1.0f, -1.0f,  1.0f}},

		{{-1.0f,  1.0f, -1.0f}},
		{{ 1.0f,  1.0f, -1.0f}},
		{{ 1.0f,  1.0f,  1.0f}},
		{{ 1.0f,  1.0f,  1.0f}},
		{{-1.0f,  1.0f,  1.0f}},
		{{-1.0f,  1.0f, -1.0f}},

		{{-1.0f, -1.0f, -1.0f}},
		{{-1.0f, -1.0f,  1.0f}},
		{{ 1.0f, -1.0f, -1.0f}},
		{{ 1.0f, -1.0f, -1.0f}},
		{{-1.0f, -1.0f,  1.0f}},
		{{ 1.0f, -1.0f,  1.0f}}
	};

	return new Mesh(vertices);
}