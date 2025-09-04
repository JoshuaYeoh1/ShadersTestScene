#pragma once
#include <glm/glm.hpp>

class Positional
{
private:

	mutable bool isDirty = true;
protected:
	friend class LightDebug;
	mutable glm::vec3 position;
	mutable glm::vec3 rotation;

	mutable glm::mat4 transform = glm::mat4(1.0f);
	mutable glm::mat4 parentTransform = glm::mat4(1.0f);

	void recalculate() const;

	// Most likely you want getPosition() instead of this
	const glm::mat4& getTransformMatrix() const;
public:

	void setPosition(const glm::vec3& position) const;

	void setParentModelMatrix(const glm::mat4& parentTransformMatrix) const;

	const glm::vec3 getPosition() const;
};

class Directional
{
private:
	mutable bool isDirty = true;
protected:
	glm::vec3 direction = glm::vec3(0.0f, 0.0f, 1.0f);

	mutable glm::vec3 rotation;
	mutable glm::mat4 transform = glm::mat4(1.0f);

public:

	void setDirection(const glm::vec3& direction);

	void setRotation();

	virtual const glm::vec3 getDirection() const;
};