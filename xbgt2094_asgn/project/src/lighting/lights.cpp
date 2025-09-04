#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include "lights.h"
#include "../mesh/mesh_utils.h"
#include "../framework/simplerenderer.h"
#include "../fbo/fbo_utils.h"
#include <iostream>

void LightBase::setActive(const bool& active)
{
	this->active = active;
}

const bool& LightBase::getActive()
{
	return this->active;
}

void LightBase::setColour(const glm::vec3& colour)
{
	this->colour = colour;
}

const glm::vec3& LightBase::getColour()
{
	return this->colour;
}

void LightBase::setColor(const glm::vec3& color)
{
	this->colour = color;
}

const glm::vec3& LightBase::getColor()
{
	return this->colour;
}

void LightBase::setIntensity(float intensity)
{
	this->intensity = intensity;
}

const float LightBase::getIntensity()
{
	return this->intensity;
}

const glm::vec3& LightBase::getColourIntensified()
{
	return this->colour * this->intensity;
}

const glm::vec3& LightBase::getColorIntensified()
{
	return this->colour * this->intensity;
}

const std::string& LightBase::getName()
{
	return this->name;
}

LightType DirectionalLight::getType()
{
	return LightType::DIRECTIONAL;
}

LightType SpotLight::getType()
{
	return LightType::SPOT;
}

LightType PointLight::getType()
{
	return LightType::POINT;
}

void PointLight::setRange(const float range)
{
	this->range = range;
}

// Most likely you want to use getInverseSquaredRange()
// for shader performance reasons.
const float PointLight::getRange() const
{
	return range;
}

const float PointLight::getInverseSquaredRange() const
{
	return 1.0f / std::max(range * range, 0.0001f);
}

void SpotLight::setInputAngles(float inner, float outer)
{
	angles.x = inner;
	angles.y = outer;
}

void SpotLight::setInput_InnerAngle(float inner)
{
	angles.x = inner;
}

void SpotLight::setInput_OuterAngle(float outer)
{
	angles.y = outer;
}

// DO NOT PASS THIS TO SHADER
// USE getCalculatedAngles() INSTEAD
const glm::vec2 SpotLight::getInputAngles() const
{
	return angles;
}

const float SpotLight::getInput_InnerAngle() const
{
	return angles.x;
}

const float SpotLight::getInput_OuterAngle() const
{
	return angles.y;
}

const glm::vec3 SpotLight::getDirection() const
{
	return getTransformMatrix() * glm::vec4(glm::normalize(direction), 0.0f);
}

const glm::vec2 SpotLight::getCalculatedAngles() const
{
	float iCos = cosf(glm::radians(angles.x) * 0.5f);
	float oCos = cosf(glm::radians(angles.y) * 0.5f);
	float angleRangeInv = 1.0f / std::max(iCos - oCos, 0.001f);

	return glm::vec2(angleRangeInv, -oCos * angleRangeInv);
}