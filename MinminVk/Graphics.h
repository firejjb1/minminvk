#pragma once

#include "Type.h"
#include "Device.h"
#include "Presentation.h"

#ifdef VULKAN_IMPL
#define GLFW_INCLUDE_VULKAN
#include "VulkanImpl.cpp"
#endif 


namespace Graphics
{
	SharedPtr<Device> device;
	SharedPtr<Presentation> presentation;
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
	}

	void MainRender()
	{

	}

	void CleanUp()
	{
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
