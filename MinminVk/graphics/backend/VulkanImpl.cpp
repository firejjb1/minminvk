// Concrete implementation of all headers related to graphics that use a Vulkan backend

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <stb_image.h>

#include <graphics/Device.h>
#include <graphics/Presentation.h>
#include <graphics/Pipeline.h>
#include <graphics/Geometry.h>
#include <graphics/Import.h>
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
	VkQueue computeQueue;
	VkSurfaceKHR surface;
	VkQueue presentQueue;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
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
	Vector<VkCommandBuffer> computeCommandBuffers;
	Vector<VkBuffer> vertexBuffers;
	Vector<VkDeviceMemory> vertexBufferMemories;
	Vector<VkBuffer> indexBuffers;
	Vector<VkDeviceMemory > indexBufferMemories;
	Vector<VkBuffer> uniformBuffers;
	Vector<VkDeviceMemory> uniformBufferMemories;
	Vector<void*> uniformBuffersMapped;
	Vector<VkDescriptorPool> descriptorPools;
	Vector<Vector<VkDescriptorSet>> descriptorSetsPerPool;
	Vector<VkImage> textureImages;
	Vector<VkDeviceMemory> textureImageMemories;
	Vector<VkImageView> textureImageViews;
	Vector<VkSampler> textureSamplers;
	VkFormat depthFormatChosen;
	VkImageView depthImageView;
	Vector<VkBuffer> shaderStorageBuffers;
	Vector<VkDeviceMemory> shaderStorageBufferMemories;

	u32 MAX_FRAMES_IN_FLIGHT = 2;
	Vector<VkSemaphore> imageAvailableSemaphores;
	Vector<VkSemaphore> imageFinishedSemaphores;
	Map<int, Vector<VkSemaphore>> pipelineWaitSemaphore;
	Vector<Vector<VkFence>> pipelineInFlightFences;
	u32 fenceIndexGraphics = 0;
	u32 fenceIndexCompute = 1;

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

	void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, 
		VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, u32 mipLevels = 1, VkSampleCountFlagBits numSamples = VK_SAMPLE_COUNT_1_BIT) {
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.samples = numSamples;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image!");
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, image, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate image memory!");
		}

		vkBindImageMemory(device, image, imageMemory, 0);
	}

	void CreateSurface(GLFWwindow* window)
	{ 
		
		glfwCreateWindowSurface(instance, window, nullptr, &surface);
	}


	VkSampleCountFlagBits GetMaxUsableSampleCount() {
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

		VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
		if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
		if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
		if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
		if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
		if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
		if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

		return VK_SAMPLE_COUNT_1_BIT;
	}

	VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, u32 mipLevels = 1) {
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = mipLevels;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}

		return imageView;
	}

	VkSampleCountFlagBits MapToSampleCount(u32 count)
	{
		if (count >= 64)
			return VK_SAMPLE_COUNT_64_BIT;
		if (count >= 32)
			return VK_SAMPLE_COUNT_32_BIT;
		if (count >= 16)
			return VK_SAMPLE_COUNT_16_BIT;
		if (count >= 8)
			return VK_SAMPLE_COUNT_8_BIT;
		if (count >= 4)
			return VK_SAMPLE_COUNT_4_BIT;
		if (count >= 2)
			return VK_SAMPLE_COUNT_2_BIT;
		return VK_SAMPLE_COUNT_1_BIT;
	}

	void CreateDepthResources(Graphics::Presentation* presentation, Graphics::RenderPass* renderPass = nullptr)
	{		
		Vector<VkFormat> candidates;
		if (presentation->depthFormatType == Graphics::Presentation::DepthFormatType::D32)
			candidates.push_back(VK_FORMAT_D32_SFLOAT);
		if (presentation->depthFormatType == Graphics::Presentation::DepthFormatType::D32S8)
			candidates.push_back(VK_FORMAT_D32_SFLOAT_S8_UINT);
		if (presentation->depthFormatType == Graphics::Presentation::DepthFormatType::D24S8)
			candidates.push_back(VK_FORMAT_D24_UNORM_S8_UINT);
		VkImageTiling tiling;
		if (presentation->depthTilingType == Graphics::Texture::TilingType::LINEAR)
			tiling = VK_IMAGE_TILING_LINEAR;
		if (presentation->depthTilingType == Graphics::Texture::TilingType::OPTIMAL)
			tiling = VK_IMAGE_TILING_OPTIMAL;
		const auto& features = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
		for (VkFormat format : candidates) 
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
				depthFormatChosen = format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
				depthFormatChosen = format;
			}
		}
		if (presentation->depthTextureID.id != 0)
		{
			// depth resources already exist
			VkImage& depthImage = textureImages[presentation->depthTextureID.id];
			VkDeviceMemory& depthImageMemory = textureImageMemories[presentation->depthTextureID.memoryID];
			VkImageView& depthImageView = textureImageViews[presentation->depthTextureID.viewID];
			vkDestroyImage(VulkanImpl::device, depthImage, nullptr);
			vkFreeMemory(VulkanImpl::device, depthImageMemory, nullptr);
			vkDestroyImageView(device, depthImageView, nullptr);
			CreateImage(swapChainExtent.width, swapChainExtent.height, depthFormatChosen, tiling, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				depthImage, depthImageMemory, 1, Min(msaaSamples, MapToSampleCount(renderPass == nullptr ? 64 : renderPass->numSamplesDepth)));
			depthImageView = CreateImageView(depthImage, depthFormatChosen, VK_IMAGE_ASPECT_DEPTH_BIT);
		}
		else
		{
			VkImage& depthImage = textureImages.emplace_back();
			VkDeviceMemory& depthImageMemory = textureImageMemories.emplace_back();
			VkImageView& depthImageView = textureImageViews.emplace_back();
			presentation->depthTextureID.id = textureImages.size() - 1;
			presentation->depthTextureID.memoryID = textureImageMemories.size() - 1;
			presentation->depthTextureID.viewID = textureImageViews.size() - 1;

			CreateImage(swapChainExtent.width, swapChainExtent.height, depthFormatChosen, tiling, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
				depthImage, depthImageMemory, 1, Min(msaaSamples, MapToSampleCount(renderPass == nullptr ? 64 : renderPass->numSamplesDepth)));
			depthImageView = CreateImageView(depthImage, depthFormatChosen, VK_IMAGE_ASPECT_DEPTH_BIT);
		}
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
#ifdef APPLE
		extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif
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
#ifdef APPLE
		createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
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
		std::optional<u32> graphicsAndComputeFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsAndComputeFamily.has_value() && presentFamily.has_value();
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
			if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)) {
				indices.graphicsAndComputeFamily = i;
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
				msaaSamples = GetMaxUsableSampleCount();
				break;
			}
		}
		if (physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("failed to find a suitable GPU!");
		}
	}

	void CreateFenceObjects(int pipelineID, int numFences)
	{
		assert(pipelineInFlightFences.size() == pipelineID);

		Vector<VkFence> newFences;
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		for (size_t i = 0; i < numFences; ++i)
		{
			auto& newFence = newFences.emplace_back();
			if (vkCreateFence(device, &fenceInfo, nullptr, &newFence) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create fence objects for a pipeline!");
			}
		}
		pipelineInFlightFences.push_back(newFences);

	}

	void CreateLogicalDevice()
	{
		QueueFamilyIndices indices = FindQueueFamilies(physicalDevice);

		Vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		Set<u32> uniqueQueueFamilies = { indices.graphicsAndComputeFamily.value(), indices.presentFamily.value() };
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
		vkGetDeviceQueue(device, indices.graphicsAndComputeFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.graphicsAndComputeFamily.value(), 0, &computeQueue);
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

		// graphics fences
		CreateFenceObjects(fenceIndexGraphics, MAX_FRAMES_IN_FLIGHT);
		// compute fences
		CreateFenceObjects(fenceIndexCompute, MAX_FRAMES_IN_FLIGHT);

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
		uint32_t queueFamilyIndices[] = { indices.graphicsAndComputeFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsAndComputeFamily != indices.presentFamily) {
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

	void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, u32 mipLevels = 1) {
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
		barrier.subresourceRange.levelCount = mipLevels;
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

	void CreateUniformBuffer(Graphics::Buffer& buffer, int numBuffer)
	{
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			auto& uniformBuffer = uniformBuffers.emplace_back();
			auto& uniformBufferMemory = uniformBufferMemories.emplace_back();
			auto& uniformBufferMapped = uniformBuffersMapped.emplace_back();
			assert(uniformBuffers.size() == uniformBufferMemories.size() && uniformBuffers.size() == uniformBuffersMapped.size());
			buffer.extendedBufferIDs.push_back(uniformBuffers.size() - 1);
			CreateBuffer(buffer.GetBufferSize(), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffer, uniformBufferMemory);
			vkMapMemory(device, uniformBufferMemory, 0, buffer.GetBufferSize(), 0, &uniformBufferMapped);
		}
	}

	void UpdateUniformBuffer(void* bufferData, size_t bufferSize, Graphics::Buffer& buffer, int swapID)
	{
		memcpy(uniformBuffersMapped[buffer.extendedBufferIDs[swapID]], bufferData, bufferSize);
	}

	VkBufferUsageFlags MapToVulkanBUfferUsageFlags(Graphics::Buffer::BufferUsageType bufferUsageType)
	{
		VkBufferUsageFlags flags = 0;
		if (EnumBitwiseAnd(bufferUsageType, Graphics::Buffer::BufferUsageType::BUFFER_VERTEX))
			flags = flags == 0 ? VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : (flags | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		if (EnumBitwiseAnd(bufferUsageType, Graphics::Buffer::BufferUsageType::BUFFER_STORAGE))
			flags = flags == 0 ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT : (flags | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		if (EnumBitwiseAnd(bufferUsageType, Graphics::Buffer::BufferUsageType::BUFFER_TRANSFER_DST))
			flags = flags == 0 ? VK_BUFFER_USAGE_TRANSFER_DST_BIT : (flags | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		if (EnumBitwiseAnd(bufferUsageType, Graphics::Buffer::BufferUsageType::BUFFER_TRANSFER_SRC))
			flags = flags == 0 ? VK_BUFFER_USAGE_TRANSFER_SRC_BIT : (flags | VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		if (EnumBitwiseAnd(bufferUsageType, Graphics::Buffer::BufferUsageType::BUFFER_INDEX))
			flags = flags == 0 ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT : (flags | VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
		if (EnumBitwiseAnd(bufferUsageType, Graphics::Buffer::BufferUsageType::BUFFER_UNIFORM))
			flags = flags == 0 ? VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT : (flags | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		return flags;
		
	}

	void CreateStorageBuffer(u32 bufferSize, Graphics::Buffer::BufferUsageType bufferUsageType, Graphics::StructuredBuffer* buffer, bool forAllFramesInFlight = true)
	{
		// Create a staging buffer used to upload data to the gpu
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

		void* data;
		vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
		memcpy(data, buffer->bufferData.data(), (size_t)bufferSize);
		vkUnmapMemory(device, stagingBufferMemory);
		for (int i = 0; i < (forAllFramesInFlight ? MAX_FRAMES_IN_FLIGHT : 1); ++i)
		{
			auto & shaderStorageBuffer = shaderStorageBuffers.emplace_back();
			auto & shaderStorageBufferMemory = shaderStorageBufferMemories.emplace_back();
			
			buffer->extendedBufferIDs.push_back(shaderStorageBuffers.size() - 1);
			CreateBuffer(bufferSize, 
				MapToVulkanBUfferUsageFlags(bufferUsageType), 
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
				shaderStorageBuffer, 
				shaderStorageBufferMemory);
			CopyBuffer(stagingBuffer, shaderStorageBuffer, bufferSize);
		}
		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
	}

	Graphics::PipeLineID CreateComputePipeline(SharedPtr<Graphics::Shader> computeShader, Graphics::ComputePipeline* pipeline, int layoutID)
	{
		VkShaderModule computeShaderModule = createShaderModule(computeShader->shaderCode);

        VkPipelineShaderStageCreateInfo computeShaderStageInfo{};
        computeShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        computeShaderStageInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        computeShaderStageInfo.module = computeShaderModule;
        computeShaderStageInfo.pName = computeShader->entryPoint.c_str();

		auto& computePipelineLayout = pipelineLayouts.emplace_back();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
		auto computeDescriptorSetLayout = descriptorSetLayouts[layoutID];
        pipelineLayoutInfo.pSetLayouts = &computeDescriptorSetLayout;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &computePipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create compute pipeline layout!");
        }

        VkComputePipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineInfo.layout = computePipelineLayout;
        pipelineInfo.stage = computeShaderStageInfo;
		VkPipeline &computePipeline = pipelines.emplace_back();

        if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &computePipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create compute pipeline!");
        }

        vkDestroyShaderModule(device, computeShaderModule, nullptr);
		return Graphics::PipeLineID{ (u32)pipelineLayouts.size() - 1, pipeline };
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
		{
			rasterizer.lineWidth = pipeline->lineWidth;
			rasterizer.polygonMode = VK_POLYGON_MODE_POINT;
		}
		

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
		multisampling.rasterizationSamples = Min(MapToSampleCount(renderPassID.pointer->numSamples), msaaSamples);
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
		pipelineLayoutInfo.pSetLayouts =  &descriptorSetLayouts[pipeline->uniformDesc->layoutID];

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = pipeline->blendEnabled ? VK_TRUE : VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; // TODO
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_DST_ALPHA; // Optional
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

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = pipeline->depthTestEnable ? VK_TRUE : VK_FALSE;
		depthStencil.depthWriteEnable = pipeline->depthWriteEnable ? VK_TRUE : VK_FALSE;
		depthStencil.depthCompareOp = pipeline->depthCompareOp == Graphics::GraphicsPipeline::DepthCompareOpType::LESS ? VK_COMPARE_OP_LESS : VK_COMPARE_OP_GREATER;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.0f; // Optional
		depthStencil.maxDepthBounds = 1.0f; // Optional
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {}; // Optional
		depthStencil.back = {}; // Optional

		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
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
		if (EnumBitwiseAnd(usage, Graphics::Texture::UsageType::TRANSFER_SRC))
			result = result == 0 ? VK_IMAGE_USAGE_SAMPLED_BIT : (result | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

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
		Vector<VkAttachmentDescription> attachments;

		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = MapToVulkanFormat(renderPass->formatType);
		colorAttachment.samples = MapToSampleCount(renderPass->numSamples);
		colorAttachment.loadOp = MapToVulkanLoadOp(renderPass->loadOp);
		colorAttachment.storeOp = MapToVulkanStoreOp(renderPass->storeOp);
		colorAttachment.stencilLoadOp = MapToVulkanLoadOp(renderPass->stencilLoadOp);
		colorAttachment.stencilStoreOp = MapToVulkanStoreOp(renderPass->stencilStoreOp);
		colorAttachment.initialLayout = MapToVulkanImageLayout(renderPass->initialLayout);
		colorAttachment.finalLayout = MapToVulkanImageLayout(renderPass->finalLayout);
		attachments.push_back(colorAttachment);

		VkAttachmentReference colorAttachmentRef{};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = depthFormatChosen;
		depthAttachment.samples = MapToSampleCount(renderPass->numSamplesDepth);
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments.push_back(depthAttachment);

		VkAttachmentDescription colorAttachmentResolve{};
		colorAttachmentResolve.format = swapChainImageFormat;
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		attachments.push_back(colorAttachmentResolve);

		VkAttachmentReference colorAttachmentResolveRef{};
		colorAttachmentResolveRef.attachment = 2;
		colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		subpass.pResolveAttachments = &colorAttachmentResolveRef;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.attachmentCount = attachments.size();
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

	void CreateColorResources(Graphics::RenderPass* renderPass) {
		VkFormat colorFormat = swapChainImageFormat;
		if (renderPass->colorAttachment.textureID.id != 0)
		{
			auto& colorImage = textureImages[renderPass->colorAttachment.textureID.id];
			auto& colorImageMemory = textureImageMemories[renderPass->colorAttachment.textureID.memoryID];
			auto& colorImageView = textureImageViews[renderPass->colorAttachment.textureID.viewID];
			vkDestroyImage(VulkanImpl::device, colorImage, nullptr);
			vkFreeMemory(VulkanImpl::device, colorImageMemory, nullptr);
			vkDestroyImageView(device, colorImageView, nullptr);
			CreateImage(swapChainExtent.width, swapChainExtent.height, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				colorImage, colorImageMemory, 1, Min(msaaSamples, MapToSampleCount(renderPass->numSamples)));
			colorImageView = CreateImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		}
		else
		{
			auto& colorImage = textureImages.emplace_back();
			auto& colorImageMemory = textureImageMemories.emplace_back();
			auto& colorImageView = textureImageViews.emplace_back();
			renderPass->colorAttachment.textureID.id = textureImages.size() - 1;
			renderPass->colorAttachment.textureID.memoryID = textureImageMemories.size() - 1;
			renderPass->colorAttachment.textureID.viewID = textureImageViews.size() - 1;
			CreateImage(swapChainExtent.width, swapChainExtent.height, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				colorImage, colorImageMemory, 1, Min(msaaSamples, MapToSampleCount(renderPass->numSamples)));
			colorImageView = CreateImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		}
	
	}
	
	void CreateFramebuffers(SharedPtr<Graphics::RenderPass> renderPass, SharedPtr<Graphics::Presentation> presentation)
	{
		if (renderPass->shouldFramebuffersMatchSwapchain)
		{
			// TODO able to create non-swapchain frame buffers
			swapChainFramebuffers.resize(swapChainImageViews.size());
			for (size_t i = 0; i < swapChainImageViews.size(); i++) {

				Vector<VkImageView> attachments;
				attachments.push_back(textureImageViews[renderPass->colorAttachment.textureID.viewID]);
				if (renderPass->writeToDepth)
					attachments.push_back(textureImageViews[presentation->depthTextureID.viewID]);
				attachments.push_back(swapChainImageViews[i]);
				VkFramebufferCreateInfo framebufferInfo{};
				framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				auto& renderPassVK = renderPasses[renderPass->renderPassID.id];
				framebufferInfo.renderPass = renderPassVK;
				framebufferInfo.attachmentCount = attachments.size();
				framebufferInfo.pAttachments = attachments.data();
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
		poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsAndComputeFamily.value();
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
		allocInfo.commandBufferCount = (u32)commandBuffers.size();

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

	Vector<Graphics::CommandList> CreateComputeCommandBuffers()
	{
		computeCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (u32)computeCommandBuffers.size();

		if (vkAllocateCommandBuffers(device, &allocInfo, computeCommandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}

		Vector<Graphics::CommandList> commandLists;
		for (u32 i = 0; i < computeCommandBuffers.size(); ++i)
		{
			auto& cmdList = commandLists.emplace_back();
			cmdList.commandListID = i;
			cmdList.isSecondary = false;
		}
		return commandLists;
	}

	void Draw(Graphics::CommandList commandList, Graphics::Geometry& geometry, Graphics::PipeLineID pipelineID, Graphics::DescriptorPoolID descriptorPoolID, int swapID)
	{
		auto& graphicsPipeline = pipelines[pipelineID.id];
		VkCommandBuffer commandBuffer = commandBuffers[commandList.commandListID];

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
		auto& vertexBuffer = vertexBuffers[geometry.geometryID.vertexBufferID];
		VkBuffer vertexBuffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };

		geometry.basicUniform->transformUniform.model = geometry.modelMatrix;
		UpdateUniformBuffer(geometry.basicUniform->GetData(), geometry.basicUniform->GetBufferSize(), *geometry.basicUniform, swapID);

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, indexBuffers[geometry.geometryID.indexBufferID], 0, VK_INDEX_TYPE_UINT16);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts[geometry.basicUniform->layoutID], 0, 1, &(descriptorSetsPerPool[descriptorPoolID.id][swapID]), 0, nullptr);

		vkCmdDrawIndexed(commandBuffer, static_cast<u32>(geometry.GetIndicesData().size()), 1, 0, 0, 0);
	}

	void Dispatch(Graphics::CommandList commandList, int pipelineID, int pipelineLayoudID, int descriptorPoolID, int swapID, vec3 threadSz, vec3 invocationSz)
	{
		auto& computePipeline = VulkanImpl::pipelines[pipelineID];
		VkCommandBuffer commandBuffer = VulkanImpl::computeCommandBuffers[commandList.commandListID];

		VkCommandBufferBeginInfo beginInfo{};

		auto& computePipelineLayout = pipelineLayouts[pipelineLayoudID];
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		vkCmdPipelineBarrier(commandBuffer, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 0, nullptr);
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout, 0, 1, &descriptorSetsPerPool[descriptorPoolID][swapID], 0, nullptr);

        vkCmdDispatch(commandBuffer, Max(threadSz[0] / invocationSz[0], 1.f), Max(threadSz[1] / invocationSz[1], 1.f), Max(threadSz[2] / invocationSz[2], 1.f));

		
	}

	void RecordCommandBuffer(Graphics::CommandList commandList) 
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
	}

	void BeginRenderPass(Graphics::CommandList commandList, Graphics::RenderPassID renderPassID, Graphics::PipeLineID pipelineID)
	{
		VkCommandBuffer commandBuffer = commandBuffers[commandList.commandListID];
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		auto& renderPass = renderPasses[renderPassID.id];
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[commandList.imageIndex];

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent;

		Vector<VkClearValue> clearValues(2);
		clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

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

	void EndRenderPass(Graphics::CommandList commandList)
	{
		VkCommandBuffer commandBuffer = commandBuffers[commandList.commandListID];
		vkCmdEndRenderPass(commandBuffer);
	}

	void DrawBuffer(Graphics::CommandList commandList, Graphics::Buffer& buffer, u32 bufferSize, u32 swapID, Graphics::DescriptorPoolID &descriptorPoolID, SharedPtr<Graphics::BasicUniformBuffer> basicUniform)
	{
		VkCommandBuffer commandBuffer = commandBuffers[commandList.commandListID];
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &shaderStorageBuffers[buffer.extendedBufferIDs[swapID]], offsets);
		UpdateUniformBuffer(basicUniform->GetData(), basicUniform->GetBufferSize(), *basicUniform, swapID);

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayouts[basicUniform->layoutID], 0, 1, &(descriptorSetsPerPool[descriptorPoolID.id][swapID]), 0, nullptr);

		vkCmdDraw(commandBuffer, bufferSize, 1, 0, 0);
	}

	void EndRecordCommandbuffer(Graphics::CommandList commandList)
	{
		VkCommandBuffer commandBuffer = commandBuffers[commandList.commandListID];

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void CreateSyncObjects()
	{
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		imageFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageFinishedSemaphores[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
	}

	void CreatePipelineSemaphore(int pipelineID, int numSemaphores)
	{
		Vector<VkSemaphore> semaphores;
		semaphores.resize(numSemaphores);
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		for (size_t i = 0; i < numSemaphores; i++) {
			if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphores[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
		}
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		pipelineWaitSemaphore[pipelineID] = semaphores;
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
		CreateColorResources(context.renderPass.get());
		CreateDepthResources(context.presentation.get(), context.renderPass.get());
		CreateFramebuffers(context.renderPass, context.presentation);
	}

	int CreateDescriptorSetLayout(const Vector<SharedPtr<Graphics::Buffer>>& buffers, const Vector<Graphics::Texture>& textures)
	{
		Vector<VkDescriptorSetLayoutBinding> bindings;
		// buffers
		for (const auto &buffer: buffers)
		{
			const auto& bufferBinding = buffer->GetBinding();
			VkDescriptorSetLayoutBinding bufferLayoutBinding{};
			bufferLayoutBinding.binding = bufferBinding.binding;
			bufferLayoutBinding.descriptorType = buffer->GetBufferType() == Graphics::Buffer::BufferType::UNIFORM ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			bufferLayoutBinding.descriptorCount = 1;
			bufferLayoutBinding.stageFlags = 0;
			bufferLayoutBinding.stageFlags |= bufferBinding.shaderStageType == Graphics::ResourceBinding::ShaderStageType::VERTEX ? VK_SHADER_STAGE_VERTEX_BIT : 0;
			bufferLayoutBinding.stageFlags |= bufferBinding.shaderStageType == Graphics::ResourceBinding::ShaderStageType::FRAGMENT ? VK_SHADER_STAGE_FRAGMENT_BIT : 0;
			bufferLayoutBinding.stageFlags |= bufferBinding.shaderStageType == Graphics::ResourceBinding::ShaderStageType::ALL_GRAPHICS ? VK_SHADER_STAGE_ALL_GRAPHICS : 0;
			bufferLayoutBinding.stageFlags |= bufferBinding.shaderStageType == Graphics::ResourceBinding::ShaderStageType::COMPUTE ? VK_SHADER_STAGE_COMPUTE_BIT : 0;
			bindings.push_back(bufferLayoutBinding);
		}
		// textures
		for (u32 i = 0; i < textures.size(); ++i)
		for (const auto &texture : textures)
		{
			VkDescriptorSetLayoutBinding samplerLayoutBinding{};
			const auto &textureBinding = texture.binding;
			samplerLayoutBinding.binding = textureBinding.binding;
			samplerLayoutBinding.descriptorCount = 1;
			samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			samplerLayoutBinding.pImmutableSamplers = nullptr;
			samplerLayoutBinding.stageFlags = 0;
			samplerLayoutBinding.stageFlags |= textureBinding.shaderStageType == Graphics::ResourceBinding::ShaderStageType::VERTEX ? VK_SHADER_STAGE_VERTEX_BIT : 0;
			samplerLayoutBinding.stageFlags |= textureBinding.shaderStageType == Graphics::ResourceBinding::ShaderStageType::FRAGMENT ? VK_SHADER_STAGE_FRAGMENT_BIT : 0;
			samplerLayoutBinding.stageFlags |= textureBinding.shaderStageType == Graphics::ResourceBinding::ShaderStageType::ALL_GRAPHICS ? VK_SHADER_STAGE_ALL_GRAPHICS : 0;
			samplerLayoutBinding.stageFlags |= textureBinding.shaderStageType == Graphics::ResourceBinding::ShaderStageType::COMPUTE ? VK_SHADER_STAGE_COMPUTE_BIT : 0;
			bindings.push_back(samplerLayoutBinding);
		}

		auto& descriptorSetLayout = descriptorSetLayouts.emplace_back();
		int layoutID = descriptorSetLayouts.size() - 1;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = bindings.size();
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
			throw std::runtime_error("failed to create descriptor set layout!");
		return layoutID;
	}

	int CreateDescriptorPool(u32 numUniforms = 0, u32 numTextures = 0, u32 numStorageBuffers = 0)
	{
		Vector<VkDescriptorPoolSize> poolSizes{};

		if (numUniforms > 0)
		{
			auto& poolSize = poolSizes.emplace_back();
			poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSize.descriptorCount = numUniforms;
		}
		if (numStorageBuffers > 0)
		{
			auto& poolSize = poolSizes.emplace_back();
			poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			poolSize.descriptorCount = numStorageBuffers;
		}
		if (numTextures > 0)
		{
			auto& poolSize = poolSizes.emplace_back();
			poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSize.descriptorCount = numTextures;
		}

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = poolSizes.size();
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = poolSizes.size();

		auto & descriptorPool = descriptorPools.emplace_back();
		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
		return descriptorPools.size() - 1;
	}

	void UpdateDescriptorSets(int descriptorPoolID, Vector<SharedPtr<Graphics::Buffer>> buffers = {}, Vector<Graphics::TextureID> textureIDs = {})
	{
		auto& descriptorSets = descriptorSetsPerPool[descriptorPoolID];
		u32 setsSize = descriptorSets.size();
		for (size_t i = 0; i < setsSize; i++)
		{
			Vector<VkWriteDescriptorSet> descriptorWrites;

			Vector<VkDescriptorBufferInfo> bufferInfos;
			if (buffers.size() > 0)
			{
				// TODO
				for (size_t bufferIndex = 0; bufferIndex < buffers.size(); ++bufferIndex)
				{
					auto& bufferInfo = bufferInfos.emplace_back();
					const auto& buffer = buffers[bufferIndex];
					bufferInfo.buffer = buffer->GetBufferType() == Graphics::Buffer::BufferType::UNIFORM ? uniformBuffers[buffer->extendedBufferIDs[i]] : shaderStorageBuffers[buffer->extendedBufferIDs[i]];
					bufferInfo.offset = 0;
					bufferInfo.range = buffer->GetBufferSize();
				}
				u32 bufferIndex = 0;
				for (auto& bufferInfo : bufferInfos)
				{
					auto &descriptorWrite = descriptorWrites.emplace_back();
					const auto& buffer = buffers[bufferIndex];
					descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					descriptorWrite.dstSet = descriptorSets[i];
					descriptorWrite.dstBinding = bufferIndex;
					descriptorWrite.dstArrayElement = 0;
					descriptorWrite.descriptorType = buffer->GetBufferType() == Graphics::Buffer::BufferType::UNIFORM ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
					descriptorWrite.descriptorCount = 1;
					descriptorWrite.pBufferInfo = &bufferInfo;
					descriptorWrite.pImageInfo = nullptr; // Optional
					descriptorWrite.pTexelBufferView = nullptr; // Optional
					bufferIndex++;
				}
			}
			Vector<VkDescriptorImageInfo> imageInfos;
			if (textureIDs.size() > 0)
			{
				// TODO
				for (auto& textureID : textureIDs)
				{
					VkDescriptorImageInfo& imageInfo = imageInfos.emplace_back();
					imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					imageInfo.imageView = textureImageViews[textureID.viewID];
					imageInfo.sampler = textureSamplers[textureID.samplerID];
				}
				u32 imageIndex = 0;
				for (auto& imageInfo : imageInfos)
				{
					auto& texDescriptorWrite = descriptorWrites.emplace_back();
					texDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					texDescriptorWrite.dstSet = descriptorSets[i];
					texDescriptorWrite.dstBinding = buffers.size() + imageIndex;
					imageIndex++;
					texDescriptorWrite.dstArrayElement = 0;
					texDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					texDescriptorWrite.descriptorCount = 1;
					texDescriptorWrite.pImageInfo = &imageInfo;
				}
			}
			vkUpdateDescriptorSets(device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

		}

	}

	void CreateDescriptorSets(int layoutID, int descriptorsetCount, int descriptorPoolID, Vector<SharedPtr<Graphics::Buffer>> allBuffers = {}, Vector<Graphics::Texture> allTextures = {})
	{
		Vector<VkDescriptorSetLayout> layouts(descriptorsetCount, descriptorSetLayouts[layoutID]);

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

		allocInfo.descriptorPool = descriptorPools[descriptorPoolID];
		allocInfo.descriptorSetCount = descriptorsetCount;
		allocInfo.pSetLayouts = layouts.data();

		auto &descriptorSets = descriptorSetsPerPool.emplace_back();
		assert(descriptorSetsPerPool.size() == descriptorPools.size());
		descriptorSets.resize(descriptorsetCount);
		if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
		Vector<Graphics::TextureID> allTexturesIDs;
		for (const auto &texture : allTextures)
			allTexturesIDs.push_back(texture.textureID);

		UpdateDescriptorSets(descriptorPoolID, allBuffers, allTexturesIDs);
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

	void GenerateMipmaps(VkImage image, VkFormat imageFormat, i32 texWidth, i32 texHeight, u32 mipLevels)
	{
		// Check if image format supports linear blitting
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, imageFormat, &formatProperties);

		if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
			throw std::runtime_error("texture image format does not support linear blitting!");
		}

		VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = texWidth;
		int32_t mipHeight = texHeight;

		for (uint32_t i = 1; i < mipLevels; i++) {
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(commandBuffer,
				image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(commandBuffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		EndSingleTimeCommands(commandBuffer);
	}

	void CreateTextureImage(Graphics::Texture& texture, stbi_uc* pixels, i32 width, i32 height, u32 mipLevels)
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

		auto& textureImage = textureImages.emplace_back();
		auto& textureImageMemory = textureImageMemories.emplace_back();
		auto& textureImageView = textureImageViews.emplace_back();
		texture.textureID.id = textureImages.size() - 1;
		texture.textureID.memoryID = textureImageMemories.size() - 1;
		texture.textureID.viewID = textureImageViews.size() - 1;

		CreateImage(width, height, MapToVulkanFormat(texture.formatType), MapToVulkanImageTiling(texture.tilingType), MapToVulkanUsageFlags(texture.usageType), VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory, mipLevels);

		textureImageView = CreateImageView(textureImage, MapToVulkanFormat(texture.formatType), VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);

		TransitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
		CopyBufferToImage(stagingBuffer, textureImage, static_cast<u32>(width), static_cast<u32>(height));

		vkDestroyBuffer(device, stagingBuffer, nullptr);
		vkFreeMemory(device, stagingBufferMemory, nullptr);
		
		GenerateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, width, height, mipLevels);
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

	void CreateTextureSampler(Graphics::Sampler & sampler, u32 mipLevels = 1)
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
		samplerInfo.minLod = 0;
		samplerInfo.maxLod = mipLevels;
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
		computeCommandLists = VulkanImpl::CreateComputeCommandBuffers();

		// TODO should be here?
		VulkanImpl::CreateSyncObjects();
	}

	bool Device::BeginRecording(Graphics::RenderContext& context)
	{
		SharedPtr<Graphics::RenderPass> renderPass = context.renderPass;
		const u32 frameID = context.frameID;

		if (!renderPass->isFrameBufferCreated)
		{
			VulkanImpl::CreateFramebuffers(renderPass, context.presentation);
			renderPass->isFrameBufferCreated = true;
		}
		u32 swapID = frameID % VulkanImpl::MAX_FRAMES_IN_FLIGHT;
		// wait for previous frame to finish
		auto pipelineID = context.renderPass->pso->pipelineID.id;

		vkWaitForFences(VulkanImpl::device, 1, &VulkanImpl::pipelineInFlightFences[VulkanImpl::fenceIndexGraphics][swapID], VK_TRUE, UINT64_MAX);
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
		vkResetFences(VulkanImpl::device, 1, &VulkanImpl::pipelineInFlightFences[VulkanImpl::fenceIndexGraphics][swapID]);
		// record a command buffer which draws the scene onto the image
		auto &commandList = GetCommandList(swapID);
		commandList.imageIndex = imageIndex;
		VulkanImpl::RecordCommandBuffer(commandList);
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

		Vector<VkSemaphore> waitSemaphores;
		Vector<VkPipelineStageFlags> waitStages;

		for (auto& pipelineToWait : context.renderPass->pso->pipelinesToWait)
		{
			waitSemaphores.push_back(VulkanImpl::pipelineWaitSemaphore[pipelineToWait.id][swapID]);
			waitStages.push_back(VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
		}
		waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		waitSemaphores.push_back(VulkanImpl::imageAvailableSemaphores[swapID]);
		submitInfo.waitSemaphoreCount = waitSemaphores.size();
		submitInfo.pWaitSemaphores = waitSemaphores.data();
		submitInfo.pWaitDstStageMask = waitStages.data();

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		VkSemaphore signalSemaphores[] = { VulkanImpl::imageFinishedSemaphores[swapID] };

		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(VulkanImpl::graphicsQueue, 1, &submitInfo, VulkanImpl::pipelineInFlightFences[VulkanImpl::fenceIndexGraphics][swapID]) != VK_SUCCESS) {
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

	bool Graphics::Device::BeginRecording(Graphics::ComputeContext& context)
	{
		u32 swapID = context.frameID % VulkanImpl::MAX_FRAMES_IN_FLIGHT;
		auto pipelineID = context.computePipeline->pipelineID.id;

		vkWaitForFences(VulkanImpl::device, 1, &VulkanImpl::pipelineInFlightFences[VulkanImpl::fenceIndexCompute][swapID], VK_TRUE, UINT64_MAX);

		vkResetFences(VulkanImpl::device, 1, &VulkanImpl::pipelineInFlightFences[VulkanImpl::fenceIndexCompute][swapID]);
		auto& commandList = context.device->GetComputeCommandList(swapID);
		VkCommandBuffer commandBuffer = VulkanImpl::computeCommandBuffers[commandList.commandListID];

		VkCommandBufferBeginInfo beginInfo{};

		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording compute command buffer!");
		}
		return true;
	}

	void Graphics::Device::EndRecording(Graphics::ComputeContext& context)
	{
		u32 swapID = context.frameID % VulkanImpl::MAX_FRAMES_IN_FLIGHT;
		auto& commandList = context.device->GetComputeCommandList(swapID);

		VkCommandBuffer commandBuffer = VulkanImpl::computeCommandBuffers[commandList.commandListID];

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record compute command buffer!");
		}

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		auto pipelineID = context.computePipeline->pipelineID.id;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		VkSemaphore toSignal{ VulkanImpl::pipelineWaitSemaphore[pipelineID].empty() ? nullptr : VulkanImpl::pipelineWaitSemaphore[pipelineID][swapID] };
		submitInfo.signalSemaphoreCount = VulkanImpl::pipelineWaitSemaphore[pipelineID].empty() ? 0 : 1;
		submitInfo.pSignalSemaphores = &toSignal;
		if (vkQueueSubmit(VulkanImpl::computeQueue, 1, &submitInfo, VulkanImpl::pipelineInFlightFences[VulkanImpl::fenceIndexCompute][swapID]) != VK_SUCCESS) {
			throw std::runtime_error("failed to submit compute command buffer!");
		};


	}

	void Device::BeginRenderPass(Graphics::RenderContext& context)
	{
		SharedPtr<Graphics::RenderPass> renderPass = context.renderPass;
		const u32 frameID = context.frameID;
		u32 swapID = frameID % VulkanImpl::MAX_FRAMES_IN_FLIGHT;
		auto pipelineID = context.renderPass->pso->pipelineID.id;
		auto& commandList = GetCommandList(swapID);

		VulkanImpl::BeginRenderPass(commandList, renderPass->renderPassID, renderPass->pso->pipelineID);
	}

	void Device::EndRenderPass(Graphics::RenderContext& context)
	{
		const u32 frameID = context.frameID;
		u32 swapID = frameID % VulkanImpl::MAX_FRAMES_IN_FLIGHT;
		auto& commandList = GetCommandList(swapID);
		VulkanImpl::EndRenderPass(commandList);
	}

	void Device::CleanUp()
	{
		for (size_t i = 0; i < VulkanImpl::MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(VulkanImpl::device, VulkanImpl::imageFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(VulkanImpl::device, VulkanImpl::imageAvailableSemaphores[i], nullptr);		
		}

		for (auto& entry : VulkanImpl::pipelineWaitSemaphore)
		{
			for (size_t i = 0; i < entry.second.size(); i++) {
				vkDestroySemaphore(VulkanImpl::device, entry.second[i], nullptr);
			}
		}

		for (auto& inFlightFences : VulkanImpl::pipelineInFlightFences)
		{
			for (auto & inFlightFence : inFlightFences)
				vkDestroyFence(VulkanImpl::device, inFlightFence, nullptr);
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
		for (auto& buffer : VulkanImpl::shaderStorageBuffers)
			vkDestroyBuffer(VulkanImpl::device, buffer, nullptr);
		for (auto& memory : VulkanImpl::shaderStorageBufferMemories)
			vkFreeMemory(VulkanImpl::device, memory, nullptr);


		for (size_t i = 0; i < VulkanImpl::uniformBuffers.size(); i++) {
			vkDestroyBuffer(VulkanImpl::device, VulkanImpl::uniformBuffers[i], nullptr);
			vkFreeMemory(VulkanImpl::device, VulkanImpl::uniformBufferMemories[i], nullptr);
		}
		for (auto& pool : VulkanImpl::descriptorPools)
			vkDestroyDescriptorPool(VulkanImpl::device, pool, nullptr);

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

	void UniformBuffer::Init()
	{
		VulkanImpl::CreateUniformBuffer(*this, VulkanImpl::MAX_FRAMES_IN_FLIGHT);
	}

	// Pipeline
	void Pipeline::Wait(PipeLineID pipelineID)
	{
		pipelinesToWait.push_back(pipelineID);
		VulkanImpl::CreatePipelineSemaphore(pipelineID.id, VulkanImpl::MAX_FRAMES_IN_FLIGHT);
	}


	void GraphicsPipeline::Init(Graphics::RenderPassID renderPassID)
	{
		Vector<SharedPtr<Graphics::Buffer>> allBuffers { this->uniformDesc };
		for (auto buffer : this->buffers)
			allBuffers.push_back(buffer);
		int layoutID = VulkanImpl::CreateDescriptorSetLayout(allBuffers, this->textures);
		this->uniformDesc->layoutID = layoutID;
		pipelineID = VulkanImpl::CreateGraphicsPipeline(vertexShader, fragmentShader, this, renderPassID);

		int poolID = VulkanImpl::CreateDescriptorPool(VulkanImpl::MAX_FRAMES_IN_FLIGHT, VulkanImpl::MAX_FRAMES_IN_FLIGHT, 0);
		this->descriptorPoolID.id = poolID;
		VulkanImpl::CreateDescriptorSets(layoutID, VulkanImpl::MAX_FRAMES_IN_FLIGHT, poolID, allBuffers);

	}

	void ComputePipeline::Init()
	{
		int layoutID = VulkanImpl::CreateDescriptorSetLayout(this->buffers, this->textures);
		this->layoutID = layoutID;
		pipelineID = VulkanImpl::CreateComputePipeline(computeShader, this, layoutID);

		u32 numUniform = 0;
		u32 numSSBO = 0;
		u32 numTex = 0;
		for (auto& buffer : buffers)
		{
			if (buffer->GetBufferType() == Buffer::BufferType::UNIFORM)
				numUniform += buffer->extendedBufferIDs.size();
			if (buffer->GetBufferType() == Buffer::BufferType::STRUCTURED)
				numSSBO += buffer->extendedBufferIDs.size();
		}
		for (auto& tex : this->textures)
			numTex++;
		// create descriptor pool
		int poolID = VulkanImpl::CreateDescriptorPool(VulkanImpl::MAX_FRAMES_IN_FLIGHT * numUniform, numTex, VulkanImpl::MAX_FRAMES_IN_FLIGHT * numSSBO);
		this->descriptorPoolID.id = poolID;
		// create descriptor sets
		VulkanImpl::CreateDescriptorSets(layoutID, VulkanImpl::MAX_FRAMES_IN_FLIGHT, poolID, this->buffers, this->textures);
	}

	void ComputePipeline::UpdateResources(Vector<SharedPtr<Buffer>>& buffers, Vector<Texture>& textures)
	{
		Vector<Graphics::TextureID> textureIDs;
		for (auto& texture : textures)
		{
			textureIDs.push_back(texture.textureID);
		}
		VulkanImpl::UpdateDescriptorSets(this->descriptorPoolID.id, buffers, textureIDs);
	}

	void ComputePipeline::Dispatch(ComputeContext& context)
	{
		u32 swapID = context.frameID % VulkanImpl::MAX_FRAMES_IN_FLIGHT;
		auto& commandList = context.device->GetComputeCommandList(swapID);
		VulkanImpl::Dispatch(commandList, this->pipelineID.id, this->layoutID, this->descriptorPoolID.id, swapID, this->threadSz, this->invocationSz);
	}

	void UniformBuffer::UpdateUniformBuffer(int frameID)
	{
		VulkanImpl::UpdateUniformBuffer(GetData(), GetBufferSize(), *this, frameID % VulkanImpl::MAX_FRAMES_IN_FLIGHT);
	}

	// RenderPass
	void RenderPass::Init(SharedPtr<Graphics::Presentation> presentation)
	{
		VulkanImpl::CreateColorResources(this);
		VulkanImpl::CreateDepthResources(presentation.get(), this);

		renderPassID = VulkanImpl::CreateRenderPass(this);
	}

	Geometry::Geometry(SharedPtr<BasicUniformBuffer> basicUniform, Texture mainTexture)
		: basicUniform{basicUniform}, mainTexture{mainTexture}
	{

	}

	Quad::Quad(int descriptorPoolID, SharedPtr<BasicUniformBuffer> basicUniform, Texture mainTexture) 
		: Geometry( basicUniform, mainTexture ) 
	{
		VulkanImpl::CreateVertexBuffer(*this);
		VulkanImpl::CreateIndexBuffer(*this);
		// TODO replace with new function
		VulkanImpl::UpdateDescriptorSets(
			descriptorPoolID,
			Vector<SharedPtr<Graphics::Buffer>>{basicUniform},
			Vector<Graphics::TextureID>{
				// add more textures in future
				this->mainTexture.textureID
			}
		);
	}

	OBJMesh::OBJMesh(int descriptorPoolID, SharedPtr<BasicUniformBuffer> basicUniform, Texture mainTexture, String filename)
		: Geometry(basicUniform, mainTexture)
	{
		Import::LoadOBJ(vertexDesc, indices, filename);
		VulkanImpl::CreateVertexBuffer(*this);
		VulkanImpl::CreateIndexBuffer(*this);
		// TODO replace with new function
		VulkanImpl::UpdateDescriptorSets(
			descriptorPoolID,
			Vector<SharedPtr<Graphics::Buffer>>{basicUniform},
			Vector<Graphics::TextureID>{
				// add more textures in future
				this->mainTexture.textureID
		}
		);
	}
	
	OBJMesh::OBJMesh(int descriptorPoolID, SharedPtr<BasicUniformBuffer> basicUniform, String filename)
		: Geometry(basicUniform, mainTexture)
	{
		Import::LoadOBJ(vertexDesc, indices, filename);
		VulkanImpl::CreateVertexBuffer(*this);
		VulkanImpl::CreateIndexBuffer(*this);
		// TODO replace with new function
		VulkanImpl::UpdateDescriptorSets(
			descriptorPoolID,
			Vector<SharedPtr<Graphics::Buffer>>{basicUniform},
			Vector<Graphics::TextureID>{}
		);
	}

	void Geometry::Draw(RenderContext& context)
	{
		u32 swapID = context.frameID % VulkanImpl::MAX_FRAMES_IN_FLIGHT;
		auto& commandList = context.device->GetCommandList(swapID);
		VulkanImpl::Draw(commandList, *this, context.renderPass->pso->pipelineID, context.renderPass->pso->descriptorPoolID, swapID);
	}

	Texture::Texture(String filename, bool autoMipchain)
		: autoMipChain{autoMipChain}
	{
		i32 width = -1;
		i32 height = -1;
		stbi_uc* data = Util::IO::ReadImage(width, height, filename);
		if (autoMipChain)
		{
			mipLevels = static_cast<u32>(std::floor(std::log2(Max(width, height))) + 1);
		}
		VulkanImpl::CreateTextureImage(*this, (stbi_uc*)data, width, height, mipLevels);
	}

	Sampler::Sampler()
	{
		VulkanImpl::CreateTextureSampler(*this, maxLOD);
	}

	void StructuredBuffer::Init()
	{
		if (extendedBufferIDs.size() == 0)
			VulkanImpl::CreateStorageBuffer(GetBufferSize(), GetUsageType(), this);

	}

	void StructuredBuffer::DrawBuffer(RenderContext& context, u32 numVertex)
	{
		u32 swapID = context.frameID % VulkanImpl::MAX_FRAMES_IN_FLIGHT;
		auto& commandList = context.device->GetCommandList(swapID);
		VulkanImpl::DrawBuffer(commandList, *this, numVertex, swapID, context.renderPass->pso->descriptorPoolID, context.renderPass->pso->uniformDesc);
	}

}