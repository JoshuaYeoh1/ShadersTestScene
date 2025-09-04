#pragma once
#include "camera_base.h"

class CameraOrbit : public CameraBase
{
private:

	double lastXpos, lastYpos;
	bool firstFrame = false;
	float dragSensitivity = 0.5f;

	float height;
	float zoom;
	float pitch, yaw;

	//glm::mat4 orient;
	glm::mat4 recalculateTransform() const override;

	bool consumeCursor() override;
	bool releaseCursor() override;
	void onLeftMouseDownInternal() override;
	void onLeftMouseUpInternal() override;
	void updateInternal(float deltaTime) override;

	bool captureMouse = false;

public:
	CameraOrbit() : pitch(0), yaw(0) {}

	void setZoom(float z);
	void setHeight(float h);
	void addPitchYaw(float pAdd, float yAdd);
	void addPitch(float add);
	void addYaw(float add);

	void cancelInput() override;

	void onMouseScrolled(float xoffset, float yoffset) override;
};