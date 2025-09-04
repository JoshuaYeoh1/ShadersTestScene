#ifdef XBGT2094_ENABLE_IMGUI
#include "lighting/light_utils.h"
#include "imgui/imgui.h"
#include <glm/gtc/type_ptr.hpp>

// specific internal_imgui_drawControls selected by imgui_drawControls

void LightUtils::internal_imgui_drawControls(DirectionalLight* light)
{
	ImGui::Indent(10);

	ImGui::Text("Active");
	ImGui::Checkbox("##active", &light->active);

	if (light->active)
	{
		ImGui::Text("Colour");
		ImGui::ColorEdit3("##colour", &light->colour[0]);

		ImGui::Text("Intensity");
		ImGui::DragFloat("##intensity", &light->intensity, 0.1);

		ImGui::Text("Direction");
		ImGui::DragFloat3("##direction", &light->direction[0], 0.1);
	}

	ImGui::Indent(-10);

	ImGui::Separator();
}

void LightUtils::internal_imgui_drawControls(PointLight* light)
{
	ImGui::Indent(10);

	ImGui::Text("Active");
	ImGui::Checkbox("##active", &light->active);

	if (light->active)
	{
		ImGui::Text("Colour");
		ImGui::ColorEdit3("##colour", glm::value_ptr(light->colour));

		ImGui::Text("Intensity");
		ImGui::DragFloat("##intensity", &light->intensity, 0.1);

		ImGui::Text("Range");
		ImGui::DragFloat("##range", &light->range, 0.1);

		ImGui::Text("Position");
		ImGui::DragFloat3("##position", &light->position[0], 0.1);
	}

	ImGui::Indent(-10);

	ImGui::Separator();
}

void LightUtils::internal_imgui_drawControls(SpotLight* light)
{
	ImGui::Indent(10);

	ImGui::Text("Active");
	ImGui::Checkbox("##active", &light->active);

	if (light->active)
	{
		ImGui::Text("Colour");
		ImGui::ColorEdit3("##colour", &light->colour[0]);

		ImGui::Text("Intensity");
		ImGui::DragFloat("##intensity", &light->intensity, 0.1);

		ImGui::Text("Direction");
		ImGui::DragFloat3("##direction", &light->direction[0], 0.1);

		ImGui::Text("Range");
		ImGui::DragFloat("##range", &light->range, 0.1);

		ImGui::Text("Position");
		ImGui::DragFloat3("##position", &light->position[0], 0.1);

		ImGui::Text("Angles");
		ImGui::DragFloat2("##angles", &light->angles[0], 0.1);
	}

	ImGui::Indent(-10);

	ImGui::Separator();
}

#endif