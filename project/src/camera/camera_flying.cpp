#include "camera_flying.h"
#include "../framework/simpleapp.h"
constexpr int LEFT_SHIFT = 340;

glm::mat4 CameraFlying::recalculateTransform() const
{
	glm::mat4 world = glm::translate(glm::mat4(1.0), position) * orient;
	return world;
}

void CameraFlying::translate(float x, float y, float z)
{
	position.x += x;
	position.y += y;
	position.z += z;
	isDirty = true;
}

void CameraFlying::translateLocal(float x, float y, float z)
{
	glm::vec3 localOrientOffset = orient * glm::vec4(x, y, z, 1.0);
	position += localOrientOffset;
	isDirty = true;
}

void CameraFlying::rotate(float angle, const glm::vec3& axis)
{
	orient = glm::rotate(glm::mat4(1.0), glm::radians(angle), axis) * orient;
	isDirty = true;
}

void CameraFlying::rotateLocal(float angle, const glm::vec3& axis)
{
	orient = orient * glm::rotate(glm::mat4(1.0), glm::radians(angle), axis);
	isDirty = true;
}

void CameraFlying::lookAtPoint(float x, float y, float z)
{
	glm::vec3 dir = glm::normalize(glm::vec3(x, y, z) - position);
	orient = glm::lookAt({ 0.0f,0.0f,0.0f }, dir, { 0.0f,1.0f,0.0f });

	// in this case, we can inverse by simply transposing
	orient = glm::transpose(orient);
	isDirty = true;
}

bool CameraFlying::consumeCursor()
{
	return true;
}

bool CameraFlying::releaseCursor()
{
	return false;
}

void CameraFlying::onLeftMouseDownInternal()
{
	captureMouse = true;
}

void CameraFlying::onLeftMouseUpInternal()
{
}

void CameraFlying::cancelInput()
{
	captureMouse = false;
}

void CameraFlying::updateInternal(float deltaTime)
{
	auto mousePos = App::getMousePosition();

	if (captureMouse)
	{
		glm::vec2 offset = lastCursorPos - mousePos;
		lastCursorPos = mousePos;

		rotate(offset.x * 0.1f, glm::vec3(0.0f, 1.0f, 0.0f));
		rotateLocal(offset.y * -0.1f, glm::vec3(1.0f, 0.0f, 0.0f));

		float mv = baseMoveSpeed;
		glm::vec3 moveOffset(0.0f);

		if (App::isKeyPressing(LEFT_SHIFT)) mv *= 4.0f; // Faster when LeftShift is held
		if (App::isKeyPressing('A')) moveOffset.x -= mv;
		if (App::isKeyPressing('D')) moveOffset.x += mv;
		if (App::isKeyPressing('W')) moveOffset.z -= mv;
		if (App::isKeyPressing('S')) moveOffset.z += mv;
		if (App::isKeyPressing('Q')) moveOffset.y += mv;
		if (App::isKeyPressing('E')) moveOffset.y -= mv;

		moveOffset *= deltaTime;
		translateLocal(moveOffset.x, 0.0f, moveOffset.z);
		translate(0.0f, moveOffset.y, 0.0f);
	}
	else
	{
		lastCursorPos = mousePos;
	}
}