// Concrete implementation of all headers related to graphics that use a Vulkan backend

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <graphics/Device.h>
#include <graphics/Presentation.h>
#include <graphics/Pipeline.h>
#include <graphics/Geometry.h>
#include <util/IO.h>

namespace VulkanImpl
{
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		Vector<VkSurfaceFormatKHR> formats;
		Vector<VkPresentModeKHR> presentModes;
	};

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

	void* windowVK;

	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	VkQueue graphicsQueue;
	VkSurfaceKHR surface;
	VkQueue presentQueue;
	VkSwapchainKHR swapChain;
	Vector<VkImage> swapChainImages;
	Vector<VkImageView> swapChainImageViews;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	Vector<VkRenderPass> renderPasses;
	Vector<VkDescriptorSetLayout> descriptorSetLayouts;
	Vector<VkPipelineLayout> pipelineLayouts;
	Vector<VkPipeline> pipelines;
	Vector<VkFramebuffer> swapChainFramebuffers;
	VkCommandPool commandPool;
	Vector<VkCommandBuffer> commandBuffers;
	Vector<VkBuffer> vertexBuffers;
	Vector<VkDeviceMemory> vertexBufferMemories;
	Vector<VkBuffer> indexBuffers;
	Vector<VkDeviceMemory > indexBufferMemories;
	Vector<VkBuffer> uniformBuffers;
	Vector<VkDeviceMemory> uniformBufferMemories;
	Vector<void*> uniformBuffersMapped;
	VkDescriptorPool descriptorPool;
	Vector<VkDescriptorSet> uniformDescriptorSets;
	Vector<VkImage> textureImages;
	Vector<VkDeviceMemory> textureImageMemories;
	Vector<VkImageView> textureImageViews;
	Vector<VkSampler> textureSamplers;
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	u32 MAX_FRAMES_IN_FLIGHT = 2;
	Vector<VkSemaphore> imageAvailableSemaphores;
	Vector<VkSemaphore> imageFinishedSemaphores;
	Vector<VkFence> inFlightFences;


	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
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

	void CreateDepthResources()
	{

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

		// renderdoc doesn't support
		// extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

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

		// renderdoc doesn't support
		// createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

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

	VkFormat MapToVulkanFormat(Graphics::Texture::FormatType format)
	{
		if (format == Graphics::Texture::FormatType::RGBA8_SRGB)
			return VK_FORMAT_R8G8B8A8_SRGB;
		if (format == Graphics::Texture::FormatType::R10G10B10A2_UNORM_PACK32)
			return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
		if (format == Graphics::Texture::FormatType::RGBA8_UNORM)
			return VK_FORMAT_R8G8B8A8_UNORM;
		if (format == Graphics::Texture::FormatType::BGRA_SRGB)
			return VK_FORMAT_B8G8R8A8_SRGB;
		return VK_FORMAT_B8G8R8A8_SRGB;
	}

	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats, Graphics::Presentation::SwapChainDetails &swapChainDetails) 
	{

		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.colorSpace == VK_COLOR_SPACE_HDR10_HLG_EXT || availableFormat.colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT)
			{
				DebugPrint("This swapchain got HDR support ! HDR output TODO\n");
			}

			// manual matching, make a table if gets too many
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && swapChainDetails.format == Graphics::Texture::FormatType::RGBA8_SRGB
				&& availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && swapChainDetails.colorSpace == Graphics::Presentation::SwapChainDetails::ColorSpaceType::SRGB_NOLINEAR) {
				return availableFormat;
			}
			if (availableFormat.format == VK_FORMAT_A2R10G10B10_UNORM_PACK32 && swapChainDetails.format == Graphics::Texture::FormatType::R10G10B10A2_UNORM_PACK32
				&& availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && swapChainDetails.colorSpace == Graphics::Presentation::SwapChainDetails::ColorSpaceType::SRGB_NOLINEAR) {
				return availableFormat;
			}
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && swapChainDetails.format == Graphics::Texture::FormatType::RGBA8_UNORM
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
		deviceFeatures.samplerAnisotropy = VK_TRUE;
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

		swapChainDetails.width = extent.width;
		swapChainDetails.height = extent.height;
		uint32_t imageCount = Max<u32>(swapChainDetails.targetImageCount, swapChainSupport.capabilities.minImageCount + 1);
		windowVK = window;
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

	void CreateSwapchainImageViews()
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

	u32 FindMemoryType(u32 typeFilter, VkMemoryPropertyFlags properties) 
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create vertex buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);
		if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate vertex buffer memory!");
		}
		vkBindBufferMemory(device, buffer, bufferMemory, 0);
	}

	VkCommandBuffer BeginSingleTimeCommands()
	{
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = commandPool;
		allocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);
		return commandBuffer;
	}

	void EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphicsQueue);

		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	}

	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) 
	{
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

		EndSingleTimeCommands(commandBuffer);
	}

	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else {
			throw std::invalid_argument("unsupported layout transition!");
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		EndSingleTimeCommands(commandBuffer);
	}

	void CreateVertexBuffer(Graphics::Geometry &geometry)
	{
		const auto& vertices = geometry.GetVertexData().vertices;
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, vertices.data(), (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		auto& vertexBuffer = vertexBuffers.emplace_back();
		auto& vertexBufferMemory = vertexBufferMemories.emplace_back();

		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
		geometry.geometryID.vertexBufferID = vertexBuffers.size() - 1;

		CopyBuffer(stagingBuffer, vertexBuffer, bufferSize);
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	void CreateIndexBuffer(Graphics::Geometry& geometry)
	{
		const auto& indices = geometry.GetIndicesData();

		VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, indices.data(), (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);

		auto& indexBuffer = indexBuffers.emplace_back();
		auto& indexBufferMemory = indexBufferMemories.emplace_back();
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);
		geometry.geometryID.indexBufferID = indexBuffers.size() - 1;

		CopyBuffer(stagingBuffer, indexBuffer, bufferSize);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	void CreateBasicUniformBuffer(Graphics::BasicUniformBuffer& uniformDesc)
	{
		VkDeviceSize bufferSize = sizeof(Graphics::BasicUniformBuffer);
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			auto& uniformBuffer = uniformBuffers.emplace_back();
			auto& uniformBufferMemory = uniformBufferMemories.emplace_back();
			auto& uniformBufferMapped = uniformBuffersMapped.emplace_back();
			assert(uniformBuffers.size() == uniformBufferMemories.size() && uniformBuffers.size() == uniformBuffersMapped.size());
			uniformDesc.uniformBufferID = uniformBuffers.size() - 1;
			CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffer, uniformBufferMemory);
			vkMapMemory(device, uniformBufferMemory, 0, bufferSize, 0, &uniformBufferMapped);
		}
	}

	Graphics::PipeLineID CreateGraphicsPipeline(SharedPtr<Graphics::Shader> vertexShader, SharedPtr<Graphics::Shader> fragShader, Graphics::GraphicsPipeline* pipeline, Graphics::RenderPassID renderPassID)
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


		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		const Graphics::VertexBinding &vertexBinding = pipeline->vertexDesc->GetVertexBinding();
		const Vector<Graphics::VertexAttribute>& vertexAttributes = pipeline->vertexDesc->GetVertexAttributes();
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = vertexBinding.binding;
		bindingDescription.stride = vertexBinding.stride;
		bindingDescription.inputRate = vertexBinding.inputRateType == Graphics::VertexBinding::InputRateType::VERTEX ?
			VK_VERTEX_INPUT_RATE_VERTEX : VK_VERTEX_INPUT_RATE_INSTANCE;
		Vector<VkVertexInputAttributeDescription> attributeDescriptionsVK;
		for (const auto& attribute : vertexAttributes)
		{
			auto& attributeDescVK = attributeDescriptionsVK.emplace_back();
			attributeDescVK.binding = attribute.binding;
			attributeDescVK.location = attribute.location;
			attributeDescVK.offset = attribute.offset;
			if (attribute.vertexFormatType == Graphics::VertexAttribute::VertexFormatType::FLOAT)
				attributeDescVK.format = VK_FORMAT_R32_SFLOAT;
			else if (attribute.vertexFormatType == Graphics::VertexAttribute::VertexFormatType::VEC2)
				attributeDescVK.format = VK_FORMAT_R32G32_SFLOAT;
			else if (attribute.vertexFormatType == Graphics::VertexAttribute::VertexFormatType::VEC3)
				attributeDescVK.format = VK_FORMAT_R32G32B32_SFLOAT;
			else if (attribute.vertexFormatType == Graphics::VertexAttribute::VertexFormatType::VEC4)
				attributeDescVK.format = VK_FORMAT_R32G32B32A32_SFLOAT;
		}
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptionsVK.size());
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptionsVK.data();

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
		rasterizer.lineWidth = 1;
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

		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts =  &descriptorSetLayouts[pipeline->uniformDesc->uniformLayoutID];

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

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;

		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;

		pipelineInfo.layout = pipelineLayout;

		pipelineInfo.renderPass = renderPasses[renderPassID.id];
		pipelineInfo.subpass = 0;
		
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		VkPipeline &graphicsPipeline = pipelines.emplace_back();
		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}
		vkDestroyShaderModule(device, fragShaderModule, nullptr);
		vkDestroyShaderModule(device, vertShaderModule, nullptr);

		// assuming layout and pipeline vectors are same size
		return Graphics::PipeLineID{ (u32)pipelineLayouts.size() - 1, pipeline };
	}

	VkSampleCountFlagBits MapToVulkanCountFlag(u32 count)
	{
		if (count <= 1)
			return VK_SAMPLE_COUNT_1_BIT;
		if (count <= 2)
			return VK_SAMPLE_COUNT_2_BIT;
		if (count <= 4)
			return VK_SAMPLE_COUNT_4_BIT;
		if (count <= 8);
			return VK_SAMPLE_COUNT_8_BIT;
		if (count <= 16)
			return VK_SAMPLE_COUNT_16_BIT;
		if (count <= 32)
			return VK_SAMPLE_COUNT_32_BIT;
		return VK_SAMPLE_COUNT_64_BIT;
	}
	VkAttachmentLoadOp MapToVulkanLoadOp(Graphics::RenderPass::AttachmentOpType opType)
	{
		if (opType == Graphics::RenderPass::AttachmentOpType::CLEAR)
			return VK_ATTACHMENT_LOAD_OP_CLEAR;
		if (opType == Graphics::RenderPass::AttachmentOpType::DONTCARE)
			return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		return VK_ATTACHMENT_LOAD_OP_NONE_EXT;
	}
	VkAttachmentStoreOp MapToVulkanStoreOp(Graphics::RenderPass::AttachmentOpType opType)
	{
		if (opType == Graphics::RenderPass::AttachmentOpType::STORE)
			return VK_ATTACHMENT_STORE_OP_STORE;
		return VK_ATTACHMENT_STORE_OP_DONT_CARE;
	}
	VkImageLayout MapToVulkanImageLayout(Graphics::Texture::LayoutType layout)
	{
		if (layout == Graphics::Texture::LayoutType::PRESENT_SRC)
			return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		if (layout == Graphics::Texture::LayoutType::COLOR_ATTACHMENT)
			return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		if (layout == Graphics::Texture::LayoutType::DEPTH_ATTACHMENT)
			return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		if (layout == Graphics::Texture::LayoutType::READ_ONLY)
			return VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;
		if (layout == Graphics::Texture::LayoutType::TRANSFER_DST)
			return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		return VK_IMAGE_LAYOUT_UNDEFINED;

	}

	VkImageUsageFlags MapToVulkanUsageFlags(Graphics::Texture::UsageType usage)
	{
		VkImageUsageFlags result = 0;
		if (EnumBitwiseAnd(usage, Graphics::Texture::UsageType::SAMPLED))
			result = result == 0 ? VK_IMAGE_USAGE_SAMPLED_BIT : (result | VK_IMAGE_USAGE_SAMPLED_BIT);
		if (EnumBitwiseAnd(usage, Graphics::Texture::UsageType::TRANSFER_DST))
			result = result == 0 ? VK_IMAGE_USAGE_SAMPLED_BIT : (result | VK_IMAGE_USAGE_TRANSFER_DST_BIT);

		return result;
	}

	VkImageTiling MapToVulkanImageTiling(Graphics::Texture::TilingType tiling)
	{
		if (tiling == Graphics::Texture::TilingType::LINEAR)
			return VK_IMAGE_TILING_LINEAR;
		if (tiling == Graphics::Texture::TilingType::OPTIMAL)
			return VK_IMAGE_TILING_OPTIMAL;
	}

	Graphics::RenderPassID CreateRenderPass(Graphics::RenderPass* renderPass)
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = MapToVulkanFormat(renderPass->formatType);
		colorAttachment.samples = MapToVulkanCountFlag(renderPass->numSamples);
		colorAttachment.loadOp = MapToVulkanLoadOp(renderPass->loadOp);
		colorAttachment.storeOp = MapToVulkanStoreOp(renderPass->storeOp);
		colorAttachment.stencilLoadOp = MapToVulkanLoadOp(renderPass->stencilLoadOp);
		colorAttachment.stencilStoreOp = MapToVulkanStoreOp(renderPass->stencilStoreOp);
		colorAttachment.initialLayout = MapToVulkanImageLayout(renderPass->initialLayout);
		colorAttachment.finalLayout = MapToVulkanImageLayout(renderPass->finalLayout);

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;


		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;

		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;

		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		auto& renderPassVK = renderPasses.emplace_back();
		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPassVK) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}
		Graphics::RenderPassID id;
		id.id = renderPasses.size() - 1;
		id.pointer = renderPass;
		return id;
	}

	
	void CreateFramebuffers(SharedPtr<Graphics::RenderPass> renderPass)
	{
		if (renderPass->shouldFramebuffersMatchSwapchain)
		{
			swapChainFramebuffers.resize(swapChainImageViews.size());
			for (size_t i = 0; i < swapChainImageViews.size(); i++) {
				VkImageView attachments[] = {
					swapChainImageViews[i]
				};

				VkFramebufferCreateInfo framebufferInfo{};
				framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				auto& renderPassVK = renderPasses[renderPass->renderPassID.id];
				framebufferInfo.renderPass = renderPassVK;
				framebufferInfo.attachmentCount = 1;
				framebufferInfo.pAttachments = attachments;
				framebufferInfo.width = swapChainExtent.width;
				framebufferInfo.height = swapChainExtent.height;
				framebufferInfo.layers = 1;

				if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
					throw std::runtime_error("failed to create framebuffer!");
				}
			}
		}
		else
		{
			// TODO
			/*for (auto& frameBuffer : renderPass->frameBuffers)
			{
				auto& attachments = frameBuffer.textureIDs;
				VkImageView attachments[] = attachments.data();
			}*/
		}
	}

	void CreateCommandPool()
	{
		QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(physicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
		if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}

	Vector<Graphics::CommandList> CreateCommandBuffers()
	{
		commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}

		Vector<Graphics::CommandList> commandLists;
		for (u32 i = 0; i < commandBuffers.size(); ++i)
		{
			auto& cmdList = commandLists.emplace_back();
			cmdList.commandListID = i;
			cmdList.isSecondary = false;
		}
		return commandLists;
	}

	void Draw(Graphics::CommandList commandList, Graphics::Geometry& geometry, Graphics::PipeLineID pipelineID)
	{
		auto& graphicsPipeline = pipelines[pipelineID.id];
		VkCommandBuffer commandBuffer = commandBuffers[commandList.commandListID];

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
		auto& vertexBuffer = vertexBuffers[geometry.geometryID.vertexBufferID];
		VkBuffer vertexBuffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };

		geometry.basicUniform->transformUniform.model = geometry.modelMatrix;
		memcpy(uniformBuffersMapped[geometry.basicUniform->uniformBufferID], &geometry.basicUniform->transformUniform, sizeof(Graphics::BasicUniformBuffer::TransformUniform));

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, indexBuffers[geometry.geometryID.indexBufferID], 0, VK_INDEX_TYPE_UINT16);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts[geometry.basicUniform->uniformLayoutID], 0, 1, &uniformDescriptorSets[geometry.basicUniform->uniformBufferID], 0, nullptr);

		vkCmdDrawIndexed(commandBuffer, static_cast<u32>(geometry.GetIndicesData().size()), 1, 0, 0, 0);
	}

	void RecordCommandBuffer(Graphics::CommandList commandList, Graphics::RenderPassID renderPassID, Graphics::PipeLineID pipelineID) 
	{
		VkCommandBuffer commandBuffer = commandBuffers[commandList.commandListID];
		vkResetCommandBuffer(commandBuffer, 0);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		auto& renderPass = renderPasses[renderPassID.id];
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[commandList.imageIndex];

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent;

		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			auto& graphicsPipeline = pipelines[pipelineID.id];
			vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
			VkViewport viewport{};
			viewport.x = 0.0f;
			viewport.y = 0.0f;
			viewport.width = static_cast<float>(swapChainExtent.width);
			viewport.height = static_cast<float>(swapChainExtent.height);
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

			VkRect2D scissor{};
			scissor.offset = { 0, 0 };
			scissor.extent = swapChainExtent;
			vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void EndRecordCommandbuffer(Graphics::CommandList commandList)
	{
		VkCommandBuffer commandBuffer = commandBuffers[commandList.commandListID];
		vkCmdEndRenderPass(commandBuffer);

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void CreateSyncObjects()
	{
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		imageFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {

				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
	}

	void CleanupSwapChain() 
	{
		for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
			vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
		}

		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			vkDestroyImageView(device, swapChainImageViews[i], nullptr);
		}

		vkDestroySwapchainKHR(device, swapChain, nullptr);
	}

	void RecreateSwapchain(Graphics::RenderContext &context)
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize((GLFWwindow*) windowVK, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize((GLFWwindow*)windowVK, &width, &height);
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(device);
		CleanupSwapChain();

		CreateSwapChain(context.presentation->swapChainDetails, windowVK);
		CreateSwapchainImageViews();
		CreateFramebuffers(context.renderPass);
	}

	void CreateDescriptorSetLayout(Graphics::UniformDesc& uniform, u32 numTextures = 1)
	{
		const auto& uniformBinding = uniform.GetUniformBinding();
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = uniformBinding.binding;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		// TODO array of descriptor could be used for bones animation
		uboLayoutBinding.descriptorCount = 1;

		uboLayoutBinding.stageFlags = 0;
		uboLayoutBinding.stageFlags |= uniformBinding.shaderStageType == Graphics::UniformBinding::ShaderStageType::VERTEX ? VK_SHADER_STAGE_VERTEX_BIT : 0;
		uboLayoutBinding.stageFlags |= uniformBinding.shaderStageType == Graphics::UniformBinding::ShaderStageType::FRAGMENT ? VK_SHADER_STAGE_FRAGMENT_BIT : 0;
		uboLayoutBinding.stageFlags |= uniformBinding.shaderStageType == Graphics::UniformBinding::ShaderStageType::ALL_GRAPHICS ? VK_SHADER_STAGE_ALL_GRAPHICS : 0;
		uboLayoutBinding.stageFlags |= uniformBinding.shaderStageType == Graphics::UniformBinding::ShaderStageType::COMPUTE ? VK_SHADER_STAGE_COMPUTE_BIT : 0;

		Vector<VkDescriptorSetLayoutBinding> bindings{ uboLayoutBinding };

		for (u32 i = 0; i < numTextures; ++i)
		{
			VkDescriptorSetLayoutBinding samplerLayoutBinding{};
			samplerLayoutBinding.binding = bindings.size();
			samplerLayoutBinding.descriptorCount = 1;
			samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			samplerLayoutBinding.pImmutableSamplers = nullptr;
			samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			bindings.push_back(samplerLayoutBinding);
		}

		uboLayoutBinding.pImmutableSamplers = nullptr; // TODO

		auto& descriptorSetLayout = descriptorSetLayouts.emplace_back();
		uniform.uniformLayoutID = descriptorSetLayouts.size() - 1;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = bindings.size();
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
			throw std::runtime_error("failed to create descriptor set layout!");
	}
	
	void CreateUniformDescriptorPool()
	{
		Vector<VkDescriptorPoolSize> poolSizes{};
		poolSizes.resize(2);

		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		// assuming as many uniform descriptor sets as uniform buffers
		poolSizes[0].descriptorCount = uniformBuffers.size();
		// texture pool
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = uniformBuffers.size();

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = poolSizes.size();
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = uniformBuffers.size();

		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void UpdateDescriptorSets(Vector<Graphics::TextureID> textureIDs = {})
	{
		u32 setsSize = uniformBuffers.size();

		for (size_t i = 0; i < setsSize; i++)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(Graphics::BasicUniformBuffer);

			Vector< VkDescriptorImageInfo> imageInfos;
			if (textureIDs.size() > 0)
			{


				for (auto& textureID : textureIDs)
				{
					VkDescriptorImageInfo& imageInfo = imageInfos.emplace_back();
					imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageInfo.imageView = textureImageViews[textureID.id];
					imageInfo.sampler = textureSamplers[textureID.samplerID];
				}
			}
			Vector< VkWriteDescriptorSet>  descriptorWrites;
			auto& descriptorWrite = descriptorWrites.emplace_back();
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = uniformDescriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr; // Optional
			descriptorWrite.pTexelBufferView = nullptr; // Optional

			for (auto& imageInfo : imageInfos)
			{
				auto& texDescriptorWrite = descriptorWrites.emplace_back();
				texDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				texDescriptorWrite.dstSet = uniformDescriptorSets[i];
				texDescriptorWrite.dstBinding = 1;
				texDescriptorWrite.dstArrayElement = 0;
				texDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				texDescriptorWrite.descriptorCount = 1;
				texDescriptorWrite.pImageInfo = &imageInfo;
			}
			vkUpdateDescriptorSets(device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

		}
	}

	void CreateUniformDescriptorSets(Graphics::BasicUniformBuffer& uniform)
	{
		// assuming as many uniform descriptor sets as uniform buffers
		u32 setsSize = uniformBuffers.size();
		Vector<VkDescriptorSetLayout> layouts(setsSize, descriptorSetLayouts[uniform.uniformLayoutID]);
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = setsSize;
		allocInfo.pSetLayouts = layouts.data();

		uniformDescriptorSets.resize(setsSize);
		if (vkAllocateDescriptorSets(device, &allocInfo, uniformDescriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
		UpdateDescriptorSets();
		
	}

	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};
		vkCmdCopyBufferToImage(
			commandBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);
		EndSingleTimeCommands(commandBuffer);
	}

	void CreateTextureImage(Graphics::Texture& texture, stbi_uc* pixels, i32 width, i32 height)
	{
		VkDeviceSize imageSize = width * height * 4;
		assert(width > 0 && height > 0 && pixels != nullptr);
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(device, stagingBufferMemory);
		stbi_image_free(pixels);

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = static_cast<u32>(width);
		imageInfo.extent.height = static_cast<u32>(height);
		imageInfo.extent.depth = texture.depth;
		imageInfo.mipLevels = texture.mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = MapToVulkanFormat(texture.formatType);
		imageInfo.tiling = MapToVulkanImageTiling(texture.tilingType);
		imageInfo.initialLayout = imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = MapToVulkanUsageFlags(texture.usageType);
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0; // Optional

		auto& textureImage = textureImages.emplace_back();
		auto& textureImageMemory = textureImageMemories.emplace_back();
		auto& textureImageView = textureImageViews.emplace_back();

		if (vkCreateImage(device, &imageInfo, nullptr, &textureImage) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, textureImage, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &textureImageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}
		texture.textureID.id = textureImages.size() - 1;
		vkBindImageMemory(device, textureImage, textureImageMemory, 0);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = textureImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = MapToVulkanFormat(texture.formatType);
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		if (vkCreateImageView(device, &viewInfo, nullptr, &textureImageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}

		TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		CopyBufferToImage(stagingBuffer, textureImage, static_cast<u32>(width), static_cast<u32>(height));
		TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
		
	}

	VkSamplerAddressMode MapToVulkanAddressMode(Graphics::Sampler::AddressModeType addressMode)
	{
		if (addressMode == Graphics::Sampler::AddressModeType::CLAMP_TO_BORDER)
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		if (addressMode == Graphics::Sampler::AddressModeType::CLAMP_TO_EDGE)
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		if (addressMode == Graphics::Sampler::AddressModeType::REPEAT)
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		if (addressMode == Graphics::Sampler::AddressModeType::MIRRORED_REPEAT)
			return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
	}

	void CreateTextureSampler(Graphics::Sampler & sampler)
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = sampler.magFilter == Graphics::Sampler::FilterType::LINEAR ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
		samplerInfo.minFilter = sampler.minFilter == Graphics::Sampler::FilterType::LINEAR ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
		samplerInfo.addressModeU = MapToVulkanAddressMode(sampler.addressModeU);
		samplerInfo.addressModeV = MapToVulkanAddressMode(sampler.addressModeV);
		samplerInfo.addressModeW = MapToVulkanAddressMode(sampler.addressModeW);
		samplerInfo.anisotropyEnable = VK_TRUE;
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		samplerInfo.maxAnisotropy = Min(sampler.maxAnisotropy, (u32)properties.limits.maxSamplerAnisotropy);
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = sampler.mipFilter == Graphics::Sampler::FilterType::LINEAR ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;
		auto& textureSampler = textureSamplers.emplace_back();
		if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create texture sampler!");
		}
	}

}


namespace Graphics
{
	// Device
	void Device::Init()
	{
		VulkanImpl::PickPhysicalDevice();
		VulkanImpl::CreateLogicalDevice();
		VulkanImpl::CreateCommandPool();
		commandLists = VulkanImpl::CreateCommandBuffers();

		// TODO should be here?
		VulkanImpl::CreateSyncObjects();
	}

	bool Device::BeginRecording(Graphics::RenderContext& context)
	{
		SharedPtr<Graphics::RenderPass> renderPass = context.renderPass;
		const u32 frameID = context.frameID;

		if (!renderPass->isFrameBufferCreated)
		{
			VulkanImpl::CreateFramebuffers(renderPass);
			renderPass->isFrameBufferCreated = true;
		}
		u32 swapID = frameID % VulkanImpl::MAX_FRAMES_IN_FLIGHT;
		// wait for previous frame to finish
		vkWaitForFences(VulkanImpl::device, 1, &VulkanImpl::inFlightFences[swapID], VK_TRUE, UINT64_MAX);
		// acquire an image from the swap chain
		u32 imageIndex;
		VkResult result = vkAcquireNextImageKHR(VulkanImpl::device, VulkanImpl::swapChain, UINT64_MAX, VulkanImpl::imageAvailableSemaphores[swapID], VK_NULL_HANDLE, &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			VulkanImpl::RecreateSwapchain(context);
			return false;
		}
		else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}
		// Only reset the fence if we are submitting work
		vkResetFences(VulkanImpl::device, 1, &VulkanImpl::inFlightFences[swapID]);
		// record a command buffer which draws the scene onto the image
		auto &commandList = GetCommandList(swapID);
		commandList.imageIndex = imageIndex;
		VulkanImpl::RecordCommandBuffer(commandList, renderPass->renderPassID, renderPass->pso->pipelineID);
		return true;
	}

	void Graphics::Device::EndRecording(RenderContext& context)
	{
		u32 swapID = context.frameID % VulkanImpl::MAX_FRAMES_IN_FLIGHT;
		const auto& commandList = GetCommandList(swapID);
		VkCommandBuffer commandBuffer = VulkanImpl::commandBuffers[commandList.commandListID];

		VulkanImpl::EndRecordCommandbuffer(commandList);
		// submit the command buffer
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { VulkanImpl::imageAvailableSemaphores[swapID]};
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;

		submitInfo.pCommandBuffers = &commandBuffer;

		VkSemaphore signalSemaphores[] = { VulkanImpl::imageFinishedSemaphores[swapID]};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(VulkanImpl::graphicsQueue, 1, &submitInfo, VulkanImpl::inFlightFences[swapID]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		// submit to the swapchain
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;
		VkSwapchainKHR swapChains[] = { VulkanImpl::swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &commandList.imageIndex;
		presentInfo.pResults = nullptr; // Optional
		auto result = vkQueuePresentKHR(VulkanImpl::presentQueue, &presentInfo);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			VulkanImpl::RecreateSwapchain(context);
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

	}

	void Device::CleanUp()
	{

		for (size_t i = 0; i < VulkanImpl::MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(VulkanImpl::device, VulkanImpl::imageFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(VulkanImpl::device, VulkanImpl::imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(VulkanImpl::device, VulkanImpl::inFlightFences[i], nullptr);
		}

		for (auto& buffer : VulkanImpl::vertexBuffers)
			vkDestroyBuffer(VulkanImpl::device, buffer, nullptr);
		for (auto& memory : VulkanImpl::vertexBufferMemories)
			vkFreeMemory(VulkanImpl::device, memory, nullptr);
		for (auto buffer : VulkanImpl::indexBuffers)
			vkDestroyBuffer(VulkanImpl::device, buffer, nullptr);
		for (auto& memory : VulkanImpl::indexBufferMemories)
			vkFreeMemory(VulkanImpl::device, memory, nullptr);
		for (auto& texture : VulkanImpl::textureImages)
			vkDestroyImage(VulkanImpl::device, texture, nullptr);
		for (auto& memory : VulkanImpl::textureImageMemories)
			vkFreeMemory(VulkanImpl::device, memory, nullptr);
		for (auto& textureView : VulkanImpl::textureImageViews)
			vkDestroyImageView(VulkanImpl::device, textureView, nullptr);
		for (auto& sampler : VulkanImpl::textureSamplers)
			vkDestroySampler(VulkanImpl::device, sampler, nullptr);

		for (size_t i = 0; i < VulkanImpl::uniformBuffers.size(); i++) {
			vkDestroyBuffer(VulkanImpl::device, VulkanImpl::uniformBuffers[i], nullptr);
			vkFreeMemory(VulkanImpl::device, VulkanImpl::uniformBufferMemories[i], nullptr);
		}
		vkDestroyDescriptorPool(VulkanImpl::device, VulkanImpl::descriptorPool, nullptr);

		for (auto& layout : VulkanImpl::descriptorSetLayouts)
			vkDestroyDescriptorSetLayout(VulkanImpl::device, layout, nullptr);
		

		vkDestroyCommandPool(VulkanImpl::device, VulkanImpl::commandPool, nullptr);

		for (auto& layout : VulkanImpl::pipelineLayouts)
			vkDestroyPipelineLayout(VulkanImpl::device, layout, nullptr);
		for (auto& pipeline : VulkanImpl::pipelines)
			vkDestroyPipeline(VulkanImpl::device, pipeline, nullptr);
		for (auto renderpass : VulkanImpl::renderPasses)
			vkDestroyRenderPass(VulkanImpl::device, renderpass, nullptr);
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

		VulkanImpl::MAX_FRAMES_IN_FLIGHT = this->swapChainDetails.targetImageCount;
		this->window = window;
		VulkanImpl::CreateSurface((GLFWwindow*)window);
	}

	void Presentation::InitSwapChain()
	{
		VulkanImpl::CreateSwapChain(this->swapChainDetails, this->window);
		VulkanImpl::CreateSwapchainImageViews();

	}

	void Presentation::CleanUp()
	{
		vkDeviceWaitIdle(VulkanImpl::device);
		VulkanImpl::CleanupSwapChain();

		vkDestroySurfaceKHR(VulkanImpl::instance, VulkanImpl::surface, nullptr);

	}

	// Pipeline
	void GraphicsPipeline::Init(Graphics::RenderPassID renderPassID)
	{
		VulkanImpl::CreateDescriptorSetLayout(*this->uniformDesc, 1);
		VulkanImpl::CreateBasicUniformBuffer(*this->uniformDesc);

		pipelineID = VulkanImpl::CreateGraphicsPipeline(vertexShader, fragmentShader, this, renderPassID);

		VulkanImpl::CreateUniformDescriptorPool();
		VulkanImpl::CreateUniformDescriptorSets(*this->uniformDesc);

	}

	// RenderPass
	void RenderPass::Init()
	{
		renderPassID = VulkanImpl::CreateRenderPass(this);
	}

	Geometry::Geometry(SharedPtr<BasicUniformBuffer> basicUniform, Texture mainTexture)
		: basicUniform{basicUniform}, mainTexture{mainTexture}
	{

	}

	Quad::Quad(SharedPtr<BasicUniformBuffer> basicUniform, Texture mainTexture) 
		: Geometry( basicUniform, mainTexture ) 
	{
		VulkanImpl::CreateVertexBuffer(*this);
		VulkanImpl::CreateIndexBuffer(*this);
		VulkanImpl::UpdateDescriptorSets(
			Vector<Graphics::TextureID>{
				// add more textures in future
				this->mainTexture.textureID
			}
		);
	}

	void Quad::Draw(RenderContext& context)
	{
		u32 swapID = context.frameID % VulkanImpl::MAX_FRAMES_IN_FLIGHT;
		auto& commandList = context.device->GetCommandList(swapID);
		VulkanImpl::Draw(commandList, *this, context.renderPass->pso->pipelineID);
	}

	Texture::Texture(String filename)
	{
		i32 width = -1;
		i32 height = -1;
		stbi_uc* data = Util::IO::ReadImage(width, height, filename);

		VulkanImpl::CreateTextureImage(*this, (stbi_uc*)data, width, height);
	}

	Sampler::Sampler()
	{
		VulkanImpl::CreateTextureSampler(*this);
	}
}