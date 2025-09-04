#pragma once
#include <vector>

class CameraBase;
class LightBase;

class LightDebug
{
	friend class Positional;
	friend class PointLight;
	friend class SpotLight;

	LightDebug() = delete;

	static std::vector<LightBase*> CURRENT_LIGHTS;

	static void drawDebug(LightBase* light);

public:

	static void add(LightBase* light);
	static void clear();

	static void draw(CameraBase* camera);
};