#include "mesh.h"
#include <glad/glad.h>
#include <iostream>
#include "mikktspace.h"
#include <glm/gtx/string_cast.hpp>

static SMikkTSpaceContext context;
static SMikkTSpaceInterface iface;

static int get_num_faces_fn(const SMikkTSpaceContext* context);
static int get_num_vertices_of_face_fn(const SMikkTSpaceContext* context, int iFace);
static void get_position_fn(const SMikkTSpaceContext* context, float outpos[], int iFace, int iVert);
static void get_normal_fn(const SMikkTSpaceContext* context, float outnormal[], int iFace, int iVert);
static void get_uv_fn(const SMikkTSpaceContext* context, float outuv[], int iFace, int iVert);
static void set_tspace_basic_fn(const SMikkTSpaceContext* context, const float tangentu[], float fSign, int iFace, int iVert);

static void calcTangents(Mesh* mesh)
{
	iface.m_getNumFaces = get_num_faces_fn;
	iface.m_getNumVerticesOfFace = get_num_vertices_of_face_fn;
	iface.m_getNormal = get_normal_fn;
	iface.m_getPosition = get_position_fn;
	iface.m_getTexCoord = get_uv_fn;

	iface.m_setTSpaceBasic = set_tspace_basic_fn;

	context.m_pInterface = &iface;
	context.m_pUserData = mesh;

	genTangSpaceDefault(&context);
}

Vertex::Vertex()
	: position(0.0f), normal(0.0f), uv(0.0f), colour(1.0f), tangent(0.0f) {}
Vertex::Vertex(glm::vec3 position)
	: position(position), normal(0.0f), uv(0.0f), colour(1.0f), tangent(0.0f) {}
Vertex::Vertex(glm::vec3 position, glm::vec3 normal)
	: position(position), normal(normal), uv(0.0f), colour(1.0f), tangent(0.0f) {}
Vertex::Vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 uv)
	: position(position), normal(normal), uv(uv), colour(1.0f), tangent(0.0f) {}
Vertex::Vertex(glm::vec3 position, glm::vec3 normal, glm::vec2 uv, glm::vec3 colour)
	: position(position), normal(normal), uv(uv), colour(colour), tangent(0.0f) {}

Mesh::Mesh(std::vector<Vertex> vertices) : vertices(vertices)
{
	calcTangents(this);
	setup();
}

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}

void Mesh::setup()
{
	// Create VAO and VBO
	// VAO: Vertex Array Object
	// VBO: Vertex Buffer Object
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// Upload mesh data to the GPU
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	// Specify the layout of the vertices we just uploaded
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, colour));
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

static int get_num_faces_fn(const SMikkTSpaceContext* context)
{
	Mesh* mesh = static_cast<Mesh*>(context->m_pUserData);

	float f_size = (float)mesh->vertices.size() / 3.f;
	int i_size = (int)mesh->vertices.size() / 3;

	assert((f_size - (float)i_size) == 0.f);

	return i_size;
}

static int get_num_vertices_of_face_fn(const SMikkTSpaceContext* context, int iFace)
{
	//Mesh* mesh = static_cast<Mesh*>(context->m_pUserData);

	return 3;
}

static void get_position_fn(const SMikkTSpaceContext* context, float outpos[], int iFace, int iVert)
{
	Mesh* mesh = static_cast<Mesh*>(context->m_pUserData);

	auto index = iFace * 3 + iVert;
	auto vertex = mesh->vertices[index];

	outpos[0] = vertex.position.x;
	outpos[1] = vertex.position.y;
	outpos[2] = vertex.position.z;
}

static void get_normal_fn(const SMikkTSpaceContext* context, float outnormal[], int iFace, int iVert)
{
	Mesh* mesh = static_cast<Mesh*>(context->m_pUserData);

	auto index = iFace * 3 + iVert;
	auto vertex = mesh->vertices[index];

	outnormal[0] = vertex.normal.x;
	outnormal[1] = vertex.normal.y;
	outnormal[2] = vertex.normal.z;
}

static void get_uv_fn(const SMikkTSpaceContext* context, float outuv[], int iFace, int iVert)
{
	Mesh* mesh = static_cast<Mesh*>(context->m_pUserData);

	auto index = iFace * 3 + iVert;
	auto vertex = mesh->vertices[index];

	outuv[0] = vertex.uv.x;
	outuv[1] = vertex.uv.y;
}

static void set_tspace_basic_fn(const SMikkTSpaceContext* context, const float tangentu[], float fSign, int iFace, int iVert)
{
	Mesh* mesh = static_cast<Mesh*>(context->m_pUserData);

	auto index = iFace * 3 + iVert;
	auto* vertex = &mesh->vertices[index];

	vertex->tangent.x = tangentu[0];
	vertex->tangent.y = tangentu[1];
	vertex->tangent.z = tangentu[2];
	vertex->tangent.w = fSign;
}