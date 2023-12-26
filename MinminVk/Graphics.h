#pragma once

#include "Type.h"
#include "Device.h"

#ifdef VULKAN_IMPL
#define GLFW_INCLUDE_VULKAN
#include "VulkanImpl.cpp"
#endif 


namespace Graphics
{
	SharedPtr<Device> device;
	void InitGraphics()
	{
		device = MakeShared<Device>();
		device->Init();
	}

	void MainRender()
	{

	}

	void CleanUp()
	{
		device->CleanUp();
	}
};
