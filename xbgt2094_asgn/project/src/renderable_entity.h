#pragma once
#include <glm/gtx/quaternion.hpp>
#include "framework/framework.h"
#include <string>

struct RenderableEntity
{
public:
	std::string name;

	Mesh* mesh;
	Shader* shader;

	bool active;

	// Transformations
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);	// NOTE: ROTATIONS ARE IN DEGREES!
	glm::vec3 scale = glm::vec3(1.0f);

	// Materials
	// ----------------------------
	Texture2D* diffuseTex;
	Texture2D* specularTex;
	Texture2D* normalTex;
	Texture2D* emissiveTex;
	Texture2D* aoTex;
	// ----------------------------

	float shininess;
	float alphaClip;
	bool doubleSided;
	float breathingSpeed;
	glm::vec3 tint;
	float opacity;

	RenderableEntity();
	glm::mat4 getModelMatrix() const;
	glm::vec3 getPosition() const;
	
	RenderableEntity* parent = nullptr;
};
