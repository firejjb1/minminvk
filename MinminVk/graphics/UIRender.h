#pragma once

#include <imgui_impl_vulkan.h>
#include <graphics/presentation.h>

namespace Graphics
{
	struct UIRender
	{
		int poolID;

		UIRender(ImGui_ImplVulkan_InitInfo &init_info, SharedPtr<Presentation> presentation);
	};
}