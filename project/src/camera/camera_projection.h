#pragma once
#include <glm/glm.hpp>

enum class CameraProjectionType
{
	ORTHOGRAPHIC,
	PERSPECTIVE
};

class CameraProjectionData
{
public:
	friend class CameraBase;

	const glm::mat4 getProjectionMatrix();

	void setFieldOfView(float fovDegrees);
	void setOrthoSize(float orthoSize);
	void setNearClip(float zNear);
	void setFarClip(float zFar);
	void setViewportSize(unsigned int viewportWidth, unsigned int viewportHeight);
	void setProjectionType(CameraProjectionType projType);

private:
	glm::mat4 projection;
	CameraProjectionType type;

	float fov = 60.0f;
	float orthoSize = 5.0f;
	float zNear = 0.1f;
	float zFar = 400.0f;
	unsigned int viewportWidth, viewportHeight;
	bool isDirty = false;

	void recalc();
	void calcProj_Perspective(float aspectRatio);
	void calcProj_Ortho(float aspectRatio);
};