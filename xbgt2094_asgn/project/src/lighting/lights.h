#pragma once
#include <string>
#include "lights_common.h"

enum class LightType
{
	DIRECTIONAL = 0,
	POINT = 1,
	SPOT = 2
};

class LightBase
{
protected:
	bool active;
	glm::vec3 colour;
	float intensity;
	std::string name;

public:
	virtual LightType getType() = 0;
	LightBase(std::string name) : active(true), name(name), colour(1.0f), intensity(1.0f) {}

	void setActive(const bool& active);
	void setColour(const glm::vec3& colour);
	void setColor(const glm::vec3& color); // Alias of setColour(...)

	void setIntensity(float intensity);

	const bool& getActive();
	const glm::vec3& getColour();
	const glm::vec3& getColor(); // Alias of getColour()

	const glm::vec3& getColourIntensified();
	const glm::vec3& getColorIntensified(); // Alias of getColourIntensified()

	const float getIntensity();

	const std::string& getName();
};

class DirectionalLight : public LightBase, public Directional
{
private:
	friend class LightDebug;
	friend class LightUtils;
	DirectionalLight(std::string name) : LightBase(name) {}

public:
	LightType getType() override;
};

class PointLight : public LightBase, public Positional
{
private:
	friend class LightDebug;
	friend class LightUtils;

protected:
	float range;

	PointLight(std::string name) : LightBase(name), range(5), parentMatrix(nullptr) {}

public:
	LightType getType() override;

	void setRange(const float range);
	const float getRange() const;

	// This is the range to be sent to shader
	const float getInverseSquaredRange() const;

	glm::mat4* parentMatrix;
};

class SpotLight : public PointLight, public Directional
{
private:
	friend class LightDebug;
	friend class LightUtils;

protected:
	glm::vec2 angles;

	SpotLight(std::string name) : PointLight(name), angles(0.0f), parentMatrix(nullptr) { }

public:
	LightType getType() override;

	void setInputAngles(float inner, float outer);
	void setInput_InnerAngle(float inner);
	void setInput_OuterAngle(float outer);

	const glm::vec2 getInputAngles() const;
	const float getInput_InnerAngle() const;
	const float getInput_OuterAngle() const;

	const const glm::vec3 getDirection() const override;

	// Send this one to shader
	const glm::vec2 getCalculatedAngles() const;

	glm::mat4* parentMatrix;
};
