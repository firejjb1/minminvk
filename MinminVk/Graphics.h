#pragma once

#include "Type.h"
#include "Device.h"
#include "Presentation.h"
#include "Pipeline.h"

#ifdef VULKAN_IMPL
#define GLFW_INCLUDE_VULKAN
#include "VulkanImpl.cpp"
#endif 


namespace Graphics
{
	SharedPtr<Device> device;
	SharedPtr<Presentation> presentation;
	SharedPtr<GraphicsPipeline> forwardPipeline;

	void InitGraphics(void * window)
	{
#ifdef VULKAN_IMPL
		// vulkan specific - Instance and Validation Layers
		VulkanImpl::CreateInstance();
		VulkanImpl::SetupDebugMessenger();
#endif
		presentation = MakeShared<Presentation>();
		device = MakeShared<Device>();

		presentation->Init(window);
		device->Init();
		presentation->InitSwapChain();

		forwardPipeline = MakeShared<GraphicsPipeline>(
			MakeShared<Shader>("trianglevert.spv", Shader::ShaderType::SHADER_VERTEX, "main"),
			MakeShared<Shader>("trianglefrag.spv", Shader::ShaderType::SHADER_FRAGMENT, "main")
		);
		forwardPipeline->Init();

	}

	void MainRender()
	{

	}

	void CleanUp()
	{

#ifdef VULKAN_IMPL
		for (auto& layout : VulkanImpl::pipelineLayouts)
		{
			vkDestroyPipelineLayout(device, layout, nullptr);
		}
#endif
		presentation->CleanUp();
		device->CleanUp();

#ifdef VULKAN_IMPL
		if (VulkanImpl::enableValidationLayers) {
			VulkanImpl::DestroyDebugUtilsMessengerEXT(VulkanImpl::instance, VulkanImpl::debugMessenger, nullptr);
		}
		vkDestroyInstance(VulkanImpl::instance, nullptr);
#endif
	}
};
