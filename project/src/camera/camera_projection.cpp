#include "camera_projection.h"
#include <glm/gtc/matrix_transform.hpp>

const glm::mat4 CameraProjectionData::getProjectionMatrix()
{
	if (isDirty)
	{
		recalc();
		isDirty = false;
	}

	return projection;
}

void CameraProjectionData::setFieldOfView(float fovDegrees)
{
	fov = fovDegrees;

	if (type == CameraProjectionType::PERSPECTIVE)
	{
		isDirty = true;
	}
}

void CameraProjectionData::setOrthoSize(float orthoSize)
{
	this->orthoSize = orthoSize;

	if (type == CameraProjectionType::ORTHOGRAPHIC)
	{
		isDirty = true;
	}
}

void CameraProjectionData::setNearClip(float zNear)
{
	this->zNear = zNear;
	isDirty = true;
}

void CameraProjectionData::setFarClip(float zFar)
{
	this->zFar = zFar;
	isDirty = true;
}

void CameraProjectionData::setViewportSize(unsigned int viewportWidth, unsigned int viewportHeight)
{
	this->viewportWidth = viewportWidth;
	this->viewportHeight = viewportHeight;
	isDirty = true;
}

void CameraProjectionData::setProjectionType(CameraProjectionType projType)
{
	if (projType == type) return;

	type = projType;
	isDirty = true;
}

void CameraProjectionData::recalc()
{
	if (viewportHeight == 0) return;

	float aspect = (float)viewportWidth / viewportHeight;

	switch (type)
	{
	default:
	case CameraProjectionType::ORTHOGRAPHIC:
		calcProj_Ortho(aspect);
		break;
	case CameraProjectionType::PERSPECTIVE:
		calcProj_Perspective(aspect);
		break;
	}
}

void CameraProjectionData::calcProj_Perspective(float aspectRatio)
{
	projection = glm::perspective(glm::radians(fov), aspectRatio, zNear, zFar);
}

void CameraProjectionData::calcProj_Ortho(float aspectRatio)
{
	float w = aspectRatio * orthoSize;
	projection = glm::ortho(-w, w, -orthoSize, orthoSize, zNear, zFar);
}