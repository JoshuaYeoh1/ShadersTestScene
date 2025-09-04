#pragma once
#include <glm/glm.hpp>
#include "camera_projection.h"

class CameraBase
{
private:
	mutable glm::mat4 world;
	mutable glm::mat4 view;
	mutable glm::mat4 vp;

	bool processInput = true;

protected:

	mutable CameraProjectionData projData;
	mutable glm::vec3 position;
	mutable bool isDirty;

	virtual glm::mat4 recalculateTransform() const = 0;

	virtual bool consumeCursor();
	virtual bool releaseCursor();
	virtual void onLeftMouseDownInternal() = 0;
	virtual void onLeftMouseUpInternal() = 0;
	virtual void updateInternal(float deltaTime) = 0;

public:
	CameraBase() : position(0.0f, 0.0f, 0.0f), isDirty(true) {}

	const glm::mat4 getProjectionMatrix() const;
	const glm::mat4 getMatrixVP() const;

	const glm::vec3 getPosition() const;

	const float getNearClip() const;
	const float getFarClip() const;
	const float getFieldOfView() const;
	const float getOrthoSize() const;

	void enableInput(bool enable);

	void setPosition(float x, float y, float z);

	void setProjectionType(CameraProjectionType projType);
	void setFieldOfView(float fovDegrees);
	void setOrthoSize(float orthoSize);
	void setNearClip(float zNear);
	void setFarClip(float zFar);
	void setViewportSize(unsigned int viewportWidth, unsigned int viewportHeight);

	virtual inline const glm::mat4& getViewMatrix() const
	{
		return view;
	}

	virtual void update(float deltaTime);

	virtual void onMouseScrolled(float xoffset, float yoffset);
	void onLeftMouseDown();
	void onLeftMouseUp();
	bool shouldConsumeCursor();
	bool shouldReleaseCursor();
	virtual void cancelInput() = 0;
};