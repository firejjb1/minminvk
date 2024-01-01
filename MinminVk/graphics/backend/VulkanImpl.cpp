// Concrete implementation of all headers related to graphics that use a Vulkan backend

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <graphics/Device.h>
#include <graphics/Presentation.h>
#include <graphics/Pipeline.h>

namespace VulkanImpl
{
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		Vector<VkSurfaceFormatKHR> formats;
		Vector<VkPresentModeKHR> presentModes;
	};
	Vector<VkImageView> swapChainImageViews;

#ifdef NODEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif
	const Vector<const char*> validationLayers =
	{
		"VK_LAYER_KHRONOS_validation"
	};
	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	VkQueue graphicsQueue;
	VkSurfaceKHR surface;
	VkQueue presentQueue;
	VkSwapchainKHR swapChain;
	Vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	Vector<VkPipelineLayout> pipelineLayouts;

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		DebugPrint("%s\n", pCallbackData->pMessage);

		return VK_FALSE;
	}

	bool CheckValidationLayerSupport()
	{
		u32 layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		Vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	void CreateSurface(GLFWwindow* window)
	{ 
		
		glfwCreateWindowSurface(instance, window, nullptr, &surface);
	}

	Vector<const char*> GetRequiredExtensions() {
		u32 glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		Vector<const char*> extensions;
		// glfw extension is likely just this
		// extensions.push_back("VK_KHR_surface");
		for (uint32_t i = 0; i < glfwExtensionCount; i++) {
			extensions.emplace_back(glfwExtensions[i]);
		}

		extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

		if (enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) {
		SwapChainSupportDetails details;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
		u32 formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}
		return details;
	}

	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
	}

	void SetupDebugMessenger() {
		if (!enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		PopulateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	void CreateInstance()
	{
		if (enableValidationLayers && !CheckValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<u32>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			PopulateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else {
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		auto extensions = GetRequiredExtensions();


		createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

		createInfo.enabledExtensionCount = (u32)extensions.size();
		createInfo.ppEnabledExtensionNames = extensions.data();


		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}
		else
			DebugPrint("VK Instance created\n");

		u32 extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		Vector<VkExtensionProperties> extensionProps(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProps.data());
		DebugPrint("available extensions:\n");

		for (const auto& extension : extensionProps) {
			DebugPrint("%s\n", extension.extensionName);
		}
	}

	struct QueueFamilyIndices {
		std::optional<u32> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) {
		QueueFamilyIndices indices;
		// Assign index to queue families that could be found
		u32 queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		u32 i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport) {
				indices.presentFamily = i;
			}
			if (indices.isComplete()) {
				break;
			}
			i++;
		}
		return indices;
	}

	bool CheckDeviceExtensionSupport(VkPhysicalDevice device) {
		u32 extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		Vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		Set<String> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	bool isDeviceSuitable(VkPhysicalDevice device) {
		// TODO choose based on score? Device fields?
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		//return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
		QueueFamilyIndices indices = FindQueueFamilies(device);
		bool extensionsSupported = CheckDeviceExtensionSupport(device);
		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}
		bool isSuitable = indices.isComplete() && extensionsSupported && swapChainAdequate;
		if (isSuitable)
		{
			DebugPrint("Physical device chosen: %s\n", deviceProperties.deviceName);
		}

		return isSuitable;
	}

	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats, Graphics::Presentation::SwapChainDetails &swapChainDetails) 
	{

		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.colorSpace == VK_COLOR_SPACE_HDR10_HLG_EXT || availableFormat.colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT)
			{
				DebugPrint("This swapchain got HDR support ! HDR output TODO\n");
			}

			// manual matching, make a table if gets too many
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && swapChainDetails.format == Graphics::Presentation::SwapChainDetails::FormatType::RGBA8_SRGB
				&& availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && swapChainDetails.colorSpace == Graphics::Presentation::SwapChainDetails::ColorSpaceType::SRGB_NOLINEAR) {
				return availableFormat;
			}
			if (availableFormat.format == VK_FORMAT_A2R10G10B10_UNORM_PACK32 && swapChainDetails.format == Graphics::Presentation::SwapChainDetails::FormatType::R10G10B10A2_UNORM_PACK32
				&& availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && swapChainDetails.colorSpace == Graphics::Presentation::SwapChainDetails::ColorSpaceType::SRGB_NOLINEAR) {
				return availableFormat;
			}
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && swapChainDetails.format == Graphics::Presentation::SwapChainDetails::FormatType::RGBA8_UNORM
				&& availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && swapChainDetails.colorSpace == Graphics::Presentation::SwapChainDetails::ColorSpaceType::SRGB_NOLINEAR) {
				return availableFormat;
			}
			
		}
		return availableFormats[0];
	}

	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes, Graphics::Presentation::SwapChainDetails& swapChainDetails) {
		for (const auto& availablePresentMode : availablePresentModes) {
			// manual matching, make a table if gets too many
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR && swapChainDetails.mode == Graphics::Presentation::SwapChainDetails::ModeType::MAILBOX) {
				return availablePresentMode;
			}
			if (availablePresentMode == VK_PRESENT_MODE_FIFO_KHR && swapChainDetails.mode == Graphics::Presentation::SwapChainDetails::ModeType::FIFO) {
				return availablePresentMode;
			}
			if (availablePresentMode == VK_PRESENT_MODE_FIFO_RELAXED_KHR && swapChainDetails.mode == Graphics::Presentation::SwapChainDetails::ModeType::FIFO_RELAXED) {
				return availablePresentMode;
			}
			if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR && swapChainDetails.mode == Graphics::Presentation::SwapChainDetails::ModeType::IMMEDIATE) {
				return availablePresentMode;
			}
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow * window) 
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}
		else {
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent = {
				static_cast<u32>(width),
				static_cast<u32>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	void PickPhysicalDevice()
	{
		u32 deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		Vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		for (const auto& device : devices) {
			if (isDeviceSuitable(device)) {
				physicalDevice = device;
				break;
			}
		}
		if (physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}

	void CreateLogicalDevice()
	{
		QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

		Vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		Set<u32> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
		float queuePriority = 1.f;
		for (u32 queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size());

		createInfo.pEnabledFeatures = &deviceFeatures;
		// deprecated, only for older impls
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}
		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();
		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}
		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

	}

	void CreateSwapChain(Graphics::Presentation::SwapChainDetails& swapChainDetails, void* window)
	{
		VulkanImpl::SwapChainSupportDetails swapChainSupport = VulkanImpl::QuerySwapChainSupport(VulkanImpl::physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = VulkanImpl::ChooseSwapSurfaceFormat(swapChainSupport.formats, swapChainDetails);
		VkPresentModeKHR presentMode = VulkanImpl::ChooseSwapPresentMode(swapChainSupport.presentModes, swapChainDetails);
		VkExtent2D extent = VulkanImpl::ChooseSwapExtent(swapChainSupport.capabilities, (GLFWwindow*)window);
		uint32_t imageCount = Max<u32>(swapChainDetails.targetImageCount, swapChainSupport.capabilities.minImageCount + 1);

		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = VulkanImpl::surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0; // Optional
			createInfo.pQueueFamilyIndices = nullptr; // Optional
		}
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		// TODO: swapchain recreation
		createInfo.oldSwapchain = VK_NULL_HANDLE;
		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain!");
		}
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}

	void CreateImageViews()
	{
		swapChainImageViews.resize(swapChainImages.size());
		for (size_t i = 0; i < swapChainImages.size(); i++) {
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = swapChainImageFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;
			if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create image views!");
			}
		}
	}

	VkShaderModule createShaderModule(const Vector<char>& code) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const u32*>(code.data());
		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}
		return shaderModule;

	}

	u32 CreateGraphicsPipeline(SharedPtr<Graphics::Shader> vertexShader, SharedPtr<Graphics::Shader> fragShader, Graphics::GraphicsPipeline* pipeline)
	{
		VkShaderModule vertShaderModule = createShaderModule(vertexShader->shaderCode);
		VkShaderModule fragShaderModule = createShaderModule(fragShader->shaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = vertexShader->entryPoint.c_str();

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = fragShader->entryPoint.c_str();

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		Vector<VkDynamicState>	dynamicStatesVK;
		for (auto state : pipeline->dynamicStates)
		{
			switch (state)
			{
			case Graphics::GraphicsPipeline::StateType::STATE_SCISSOR:
				dynamicStatesVK.push_back(VK_DYNAMIC_STATE_SCISSOR);
				break;
			case Graphics::GraphicsPipeline::StateType::STATE_VIEWPORT:
				dynamicStatesVK.push_back(VK_DYNAMIC_STATE_VIEWPORT);
				break;
			}

		}
		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStatesVK.size());
		dynamicState.pDynamicStates = dynamicStatesVK.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		switch (pipeline->topologyType)
		{
		case Graphics::GraphicsPipeline::TopologyType::TOPO_TRIANGLE_LIST:
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			break;
		case Graphics::GraphicsPipeline::TopologyType::TOPO_POINT_LIST:
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			break;
		case Graphics::GraphicsPipeline::TopologyType::TOPO_LINE_LIST:
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			break;
		case Graphics::GraphicsPipeline::TopologyType::TOPO_LINE_STRIP:
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
			break;
		case Graphics::GraphicsPipeline::TopologyType::TOPO_TRIANGLE_STRIP:
			inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
			break;
		}
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = pipeline->viewport[0];
		viewport.y = pipeline->viewport[1];
		viewport.width = pipeline->viewport[2] > 0 ? pipeline->viewport[2] : (float)swapChainExtent.width;
		viewport.height = pipeline->viewport[3] > 0 ? pipeline->viewport[3] : (float)swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { (int)pipeline->scissor[0], (int)pipeline->scissor[1] };
		scissor.extent = { pipeline->scissor[2] > 0 ? pipeline->scissor[2] : swapChainExtent.width, pipeline->scissor[3] > 0 ? pipeline->scissor[3] : swapChainExtent.height};

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		auto& rasterStates = pipeline->rasterStates;
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		if (rasterStates.find(Graphics::GraphicsPipeline::RasterState::RASTER_DEPTH_CLAMP) != rasterStates.cend())
			rasterizer.depthClampEnable = VK_TRUE;

		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		if (rasterStates.find(Graphics::GraphicsPipeline::RasterState::RASTER_DISCARD) != rasterStates.cend())
			rasterizer.rasterizerDiscardEnable = VK_TRUE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		if (rasterStates.find(Graphics::GraphicsPipeline::RasterState::RASTER_POLYGON_MODE_LINE) != rasterStates.cend())
		{
			rasterizer.lineWidth = pipeline->lineWidth;
			rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
		}
		if (rasterStates.find(Graphics::GraphicsPipeline::RasterState::RASTER_POLYGON_MODE_POINT) != rasterStates.cend())
			rasterizer.polygonMode = VK_POLYGON_MODE_POINT;

		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		if (rasterStates.find(Graphics::GraphicsPipeline::RasterState::RASTER_CULL_MODE_BOTH) != rasterStates.cend())
			rasterizer.cullMode = VK_CULL_MODE_FRONT_AND_BACK;
		if (rasterStates.find(Graphics::GraphicsPipeline::RasterState::RASTER_CULL_MODE_DISABLE) != rasterStates.cend())
			rasterizer.cullMode = VK_CULL_MODE_NONE;
		if (rasterStates.find(Graphics::GraphicsPipeline::RasterState::RASTER_CULL_MODE_FRONT) != rasterStates.cend())
			rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;

		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		if (rasterStates.find(Graphics::GraphicsPipeline::RasterState::RASTER_FRONT_FACE_CCW) != rasterStates.end())
			rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		if (rasterStates.find(Graphics::GraphicsPipeline::RasterState::RASTER_DEPTH_BIAS_ENABLED) != rasterStates.end())
		{
			rasterizer.depthBiasEnable = VK_TRUE;
			rasterizer.depthBiasConstantFactor = 0.0f; // Optional
			rasterizer.depthBiasClamp = 0.0f; // Optional
			rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
		}
		// TODO
		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		auto& pipelineLayout = pipelineLayouts.emplace_back();
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		vkDestroyShaderModule(device, fragShaderModule, nullptr);
		vkDestroyShaderModule(device, vertShaderModule, nullptr);
		return pipelineLayouts.size();
	}


}


namespace Graphics
{
	// Device
	void Device::Init()
	{
		VulkanImpl::PickPhysicalDevice();
		VulkanImpl::CreateLogicalDevice();
	}

	void Device::CleanUp()
	{
		for (auto& layout : VulkanImpl::pipelineLayouts)
		{
			vkDestroyPipelineLayout(VulkanImpl::device, layout, nullptr);
		}
		vkDestroyDevice(VulkanImpl::device, nullptr);

		if (VulkanImpl::enableValidationLayers) {
			VulkanImpl::DestroyDebugUtilsMessengerEXT(VulkanImpl::instance, VulkanImpl::debugMessenger, nullptr);
		}
		vkDestroyInstance(VulkanImpl::instance, nullptr);
	}

	// Presentation
	void Presentation::Init(void* window)
	{
		// TODO Instance and Validation Layers should be created first but not here
		VulkanImpl::CreateInstance();
		VulkanImpl::SetupDebugMessenger();

		this->window = window;
		VulkanImpl::CreateSurface((GLFWwindow*)window);
	}

	void Presentation::InitSwapChain()
	{
		VulkanImpl::CreateSwapChain(this->swapChainDetails, this->window);
		VulkanImpl::CreateImageViews();
	}

	void Presentation::CleanUp()
	{
		for (auto imageView : VulkanImpl::swapChainImageViews) {
			vkDestroyImageView(VulkanImpl::device, imageView, nullptr);
		}
		vkDestroySwapchainKHR(VulkanImpl::device, VulkanImpl::swapChain, nullptr);
		vkDestroySurfaceKHR(VulkanImpl::instance, VulkanImpl::surface, nullptr);
	}

	// Pipeline
	void GraphicsPipeline::Init()
	{
		pipelineId = VulkanImpl::CreateGraphicsPipeline(vertexShader, fragmentShader, this);

	}
}

// namespace Test
// {
// 	void TestVK()
// 	{
// 		u32 extensionCount = 0;
// 		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
// 	}
// }