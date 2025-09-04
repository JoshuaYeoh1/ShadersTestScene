#include "renderable_entity.h"
#include <glm/gtc/matrix_transform.hpp>

RenderableEntity::RenderableEntity() : mesh(0), shader(0)
{
	active = true;

	diffuseTex = TextureUtils::checkerTexture2D();
	specularTex = TextureUtils::whiteTexture2D();
	normalTex = TextureUtils::whiteTexture2D();
	emissiveTex = TextureUtils::blackTexture2D();	// Set blank texture for safety
	aoTex = TextureUtils::whiteTexture2D();

	shininess = 128;
	alphaClip = 0.1;
	doubleSided = false;
	breathingSpeed = 0;
	tint = glm::vec3(1);
	opacity = 1;
}

glm::mat4 RenderableEntity::getModelMatrix() const
{
	glm::mat4 model =
		glm::translate(glm::mat4(1.0), position) *				// Translate last
		glm::toMat4(glm::quat(glm::radians(rotation))) *		// Rotation second; Make quaternion with rotation in radians, and then convert to mat4
		glm::scale(glm::mat4(1.0), scale);					// Scale first

	if(parent)
	{
		return parent->getModelMatrix() * model;
	}
	return model;
}

glm::vec3 RenderableEntity::getPosition() const
{
	return glm::vec3(getModelMatrix()[3]);
}