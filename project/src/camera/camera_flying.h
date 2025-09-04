#pragma once
#include "camera_base.h"
#include <glm/gtc/matrix_transform.hpp>

class CameraFlying : public CameraBase
{
protected:

	double lastXpos, lastYpos;

	glm::mat4 orient;

	float baseMoveSpeed = 4.0f;
	glm::vec2 lastCursorPos;

	glm::mat4 recalculateTransform() const override;

	bool consumeCursor() override;
	bool releaseCursor() override;
	void onLeftMouseDownInternal() override;
	void onLeftMouseUpInternal() override;

	bool captureMouse = false;

public:
	CameraFlying() : orient(glm::mat4(1.0))
	{
		projData.setProjectionType(CameraProjectionType::PERSPECTIVE);
		projData.setFieldOfView(60.0f);
		projData.setNearClip(0.1f);
		projData.setFarClip(400.0f);
	}

	void translate(float x, float y, float z);
	void translateLocal(float x, float y, float z);
	void rotate(float angle, const glm::vec3& axis);
	void rotateLocal(float angle, const glm::vec3& axis);

	void lookAtPoint(float x, float y, float z);

	void cancelInput() override;
	void updateInternal(float deltaTime) override;
};
