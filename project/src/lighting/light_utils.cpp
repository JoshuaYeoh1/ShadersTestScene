#include "light_utils.h"
#include "light_debug.h"

PointLight* LightUtils::createPointLight(std::string name)
{
	PointLight* light = new PointLight(name);

	LightDebug::add(light);
	return light;
}

DirectionalLight* LightUtils::createDirectionalLight(std::string name)
{
	DirectionalLight* light = new DirectionalLight(name);

	LightDebug::add(light);
	return light;
}

SpotLight* LightUtils::createSpotLight(std::string name)
{
	SpotLight* light = new SpotLight(name);

	LightDebug::add(light);
	return light;
}

#ifdef XBGT2094_ENABLE_IMGUI
#include "../imgui/imgui.h"

void LightUtils::imgui_drawControls(LightBase* light)
{
	// Need to push ID to ensure internal IDs used in custom UI are different
	ImGui::PushID(light);

	if (ImGui::CollapsingHeader(light->getName().c_str(), ImGuiTreeNodeFlags_None))
	{
		switch (light->getType())
		{
		case LightType::DIRECTIONAL:
		{
			DirectionalLight* d = static_cast<DirectionalLight*>(light);
			internal_imgui_drawControls(d);
		}
		break;
		case LightType::POINT:
		{
			PointLight* p = static_cast<PointLight*>(light);
			internal_imgui_drawControls(p);
		}
		break;
		case LightType::SPOT:
		{
			SpotLight* s = static_cast<SpotLight*>(light);
			internal_imgui_drawControls(s);
		}
		break;
		}

		ImGui::Spacing();
	}

	ImGui::PopID();
}

#endif