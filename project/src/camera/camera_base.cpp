#include "camera_base.h"

bool CameraBase::consumeCursor()
{
	return false;
}

bool CameraBase::releaseCursor()
{
	return true;
}

const glm::mat4 CameraBase::getProjectionMatrix() const
{
	return projData.getProjectionMatrix();
}

const glm::mat4 CameraBase::getMatrixVP() const
{
	return vp;
}

const glm::vec3 CameraBase::getPosition() const
{
	return position;
}

const float CameraBase::getNearClip() const
{
	return projData.zNear;
}

const float CameraBase::getFarClip() const
{
	return projData.zFar;
}

const float CameraBase::getFieldOfView() const
{
	return projData.fov;
}

const float CameraBase::getOrthoSize() const
{
	return projData.orthoSize;
}

void CameraBase::enableInput(bool enable)
{
	processInput = enable;
}

void CameraBase::setPosition(float x, float y, float z)
{
	position.x = x;
	position.y = y;
	position.z = z;
	isDirty = true;
}

void CameraBase::setProjectionType(CameraProjectionType projType)
{
	projData.setProjectionType(projType);
}

void CameraBase::setFieldOfView(float fovDegrees)
{
	projData.setFieldOfView(fovDegrees);
}

void CameraBase::setOrthoSize(float orthoSize)
{
	projData.setOrthoSize(orthoSize);
}

void CameraBase::setNearClip(float zNear)
{
	projData.setNearClip(zNear);
}

void CameraBase::setFarClip(float zFar)
{
	projData.setFarClip(zFar);
}

void CameraBase::setViewportSize(unsigned int viewportWidth, unsigned int viewportHeight)
{
	projData.setViewportSize(viewportWidth, viewportHeight);
	isDirty = true;
}

void CameraBase::update(float deltaTime)
{
	if (processInput)
	{
		updateInternal(deltaTime);
	}

	if (isDirty)
	{
		world = recalculateTransform();
		view = glm::inverse(world);

		vp = getProjectionMatrix() * view;
		isDirty = false;
	}
}

void CameraBase::onMouseScrolled(float xoffset, float yoffset) {}

void CameraBase::onLeftMouseDown()
{
	if (!processInput) return;
	onLeftMouseDownInternal();
};

void CameraBase::onLeftMouseUp()
{
	if (!processInput) return;
	onLeftMouseUpInternal();
};

bool CameraBase::shouldConsumeCursor()
{
	if (!processInput) return false;

	return consumeCursor();
}

bool CameraBase::shouldReleaseCursor()
{
	if (!processInput) return true;

	return releaseCursor();
}