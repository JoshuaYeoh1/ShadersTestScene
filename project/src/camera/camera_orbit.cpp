#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include "camera_orbit.h"
#include "../framework/simpleapp.h"

#define MIN_ZOOM 1.0f
#define MAX_ZOOM 20.0f

#define MIN_YAW -90.0f
#define MAX_YAW 90.0f

glm::mat4 CameraOrbit::recalculateTransform() const
{
	glm::mat4 world = glm::rotate(glm::mat4(1), glm::radians(-pitch), { 0.0f, 1.0f, 0.0f })
		* glm::rotate(glm::mat4(1), glm::radians(-yaw), { 1.0f, 0.0f, 0.0f })
		* glm::translate(glm::mat4(1), glm::vec3(0.0f, height, zoom));

	position = world[3];
	return world;
}

void CameraOrbit::setZoom(float z) {
	zoom = std::min(MAX_ZOOM, std::max(MIN_ZOOM, z));
	isDirty = true;
}

void CameraOrbit::setHeight(float h) {
	height = h;
	isDirty = true;
}

void CameraOrbit::addPitchYaw(float pAdd, float yAdd)
{
	addPitch(pAdd);
	addYaw(yAdd);
}

void CameraOrbit::addPitch(float add)
{
	pitch += add;
	isDirty = true;
}

void CameraOrbit::addYaw(float add)
{
	yaw = std::min(MAX_YAW, std::max(MIN_YAW, yaw + add));
	isDirty = true;
}

bool CameraOrbit::consumeCursor()
{
	return true;
}

bool CameraOrbit::releaseCursor()
{
	return true;
}

void CameraOrbit::onLeftMouseDownInternal()
{
	captureMouse = true;
	firstFrame = true;
}

void CameraOrbit::onLeftMouseUpInternal()
{
	cancelInput();
}

void CameraOrbit::cancelInput()
{
	captureMouse = false;
}

void CameraOrbit::updateInternal(float deltaTime)
{
	//double xpos, ypos;
	//glfwGetCursorPos(window, &xpos, &ypos);
	auto mousePos = App::getMousePosition();

	if (captureMouse)
	{
		if (firstFrame) firstFrame = false;
		else addPitchYaw((mousePos.x - lastXpos) * dragSensitivity, (mousePos.y + lastYpos) * dragSensitivity);
		//else addPitchYaw((xpos - lastXpos) * dragSensitivity, (ypos - lastYpos) * dragSensitivity);

		//lastXpos = xpos; lastYpos = ypos;
		lastXpos = mousePos.x; lastYpos = mousePos.y;
	}
}

void CameraOrbit::onMouseScrolled(float xoffset, float yoffset)
{
	setZoom(zoom - yoffset);
}

#undef MIN_ZOOM
#undef MAX_ZOOM
#undef MIN_YAW
#undef MAX_YAW