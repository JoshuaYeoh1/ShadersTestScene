#pragma once
#include "lights.h"
#include <string>

class LightUtils
{
#ifdef XBGT2094_ENABLE_IMGUI
private:
	static void internal_imgui_drawControls(PointLight* light);
	static void internal_imgui_drawControls(DirectionalLight* light);
	static void internal_imgui_drawControls(SpotLight* light);
#endif
public:
	static PointLight* createPointLight(std::string name);
	static DirectionalLight* createDirectionalLight(std::string name);
	static SpotLight* createSpotLight(std::string name);

#ifdef XBGT2094_ENABLE_IMGUI
	static void imgui_drawControls(LightBase* light);
#endif
};