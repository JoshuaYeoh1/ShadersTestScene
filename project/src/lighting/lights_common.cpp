#include "lights_common.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

void Positional::recalculate() const
{
	glm::quat q{ rotation };
	transform = parentTransform * glm::translate(glm::mat4(1), position);// *glm::mat4_cast(q);
}

void Positional::setPosition(const glm::vec3& position) const
{
	this->position = position;
	isDirty = true;
}

void Positional::setParentModelMatrix(const glm::mat4& modelMatrix) const
{
	this->parentTransform = modelMatrix;
	isDirty = true;
}

const glm::vec3 Positional::getPosition() const
{
	// ImGui changes position values directly, bypassing
	// this->isDirty flag
	//if (this->isDirty)
	//{
	//	recalculate();
	//}
	recalculate();

	return glm::vec3(transform[3]);
}

// You want getPosition() instead of this
const glm::mat4& Positional::getTransformMatrix() const
{
	// ImGui changes position values directly, bypassing
	// this->isDirty flag
	//if (this->isDirty)
	//{
	//	recalculate();
	//}
	recalculate();

	return transform;
}

void Directional::setDirection(const glm::vec3& direction)
{
	this->direction = glm::normalize(direction);
}

void Directional::setRotation()
{
	this->rotation = rotation;
	isDirty = true;
}

const glm::vec3 Directional::getDirection() const
{
	return glm::normalize(direction);
}