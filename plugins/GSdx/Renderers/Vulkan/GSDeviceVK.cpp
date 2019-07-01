/*
 *	Copyright (C) 2019 Hamish Elliott
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "stdafx.h"
#include "GSDeviceVK.h"

GSDeviceVK::GSDeviceVK()
{
	Vulkan::load_vulkan();
}

bool GSDeviceVK::Create(const std::shared_ptr<GSWnd> &wnd)
{
	m_wnd = wnd;
	/*
	 *	This function is just a bunch of Vulkan setup stuff.
	 *	I feel like it's fairly justified given the amount
	 *	of control devs are responsible for. You'll have to 
	 *	excuse how sloppy it is though, I'll fix it if someone
	 *	bugs me enough.
	 *	tl;dr this thing is entirely vulkan duct tape. heckle me, idc
	 *	- Hamish
	 */
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "PCSX2 GSdx (VK Renderer v0.1)";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	appInfo.pEngineName = "PCSX2 GSdx";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
	appInfo.apiVersion = VK_API_VERSION_1_1;

#ifdef VK_USE_PLATFORM_XLIB_KHR
	m_vk_instanceExtensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
	m_vk_instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
	if (enableValidationLayers) {
		//m_vk_layers.resize(m_vk_layers.size() + 1);
		m_vk_layers.push_back("VK_LAYER_KHRONOS_validation");
		m_vk_instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(m_vk_instanceExtensions.size());
	instanceCreateInfo.ppEnabledExtensionNames = m_vk_instanceExtensions.data();
	instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(m_vk_layers.size());
	instanceCreateInfo.ppEnabledLayerNames = m_vk_layers.data();
	VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &m_vk_Instance);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "Vulkan: Couldn't create instance\n");
		return false;
	}
	if (!Vulkan::load_instance_functions(m_vk_Instance)) {
		fprintf(stderr, "Vulkan: Couldn't load instance functions\n");
		return false;
	}
	if (enableValidationLayers) {
		VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo = {};
		debugUtilsCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugUtilsCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugUtilsCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugUtilsCreateInfo.pfnUserCallback = debugCallback;
		result = vkCreateDebugUtilsMessengerEXT(m_vk_Instance, &debugUtilsCreateInfo, nullptr, &m_vk_debugMessenger);
		if (result != VK_SUCCESS) {
			fprintf(stderr, "Vulkan: Couldn't create debug messenger\n");
			return false;
		}
	}

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_vk_Instance, &deviceCount, nullptr);
	if (deviceCount == 0) {
		fprintf(stderr, "Vulkan: Couldn't detect any physical devices\n");
		return false;
	}
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_vk_Instance, &deviceCount, devices.data());
	for (VkPhysicalDevice dev : devices) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(dev, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(dev, nullptr, &extensionCount, availableExtensions.data());
		std::set<std::string> requiredExtensions(m_vk_deviceExtensions.begin(), m_vk_deviceExtensions.end());
		for (const auto& extension: availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		if (requiredExtensions.empty()) {
			m_vk_PhysicalDevice = dev;
			break;
		}
	}
	if (m_vk_PhysicalDevice == VK_NULL_HANDLE) {
		fprintf(stderr, "Vulkan: Detected physical devices but none were suitable\n");
		return false;
	}
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(m_vk_PhysicalDevice, &deviceProperties);
	fprintf(stdout, "Vulkan device information:\n");
	fprintf(stdout, "\t%s\n\t%i\n", 
		deviceProperties.deviceName, 
		deviceProperties.driverVersion);
	fprintf(stdout, "Enabled instance extensions:\n");
	for (const char* ext : m_vk_instanceExtensions) {
		fprintf(stdout, "\t%s\n", ext);
	}
	fprintf(stdout, "Enabled device extensions:\n");
	for (const char* ext : m_vk_deviceExtensions) {
		fprintf(stdout, "\t%s\n", ext);
	}

#ifdef VK_USE_PLATFORM_XLIB_KHR
	VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.dpy = (Display*)((GSWndVK*)m_wnd.get())->GetDisplay();
	surfaceCreateInfo.window = ((GSWndVK*)m_wnd.get())->GetNativeWindow();
	result = vkCreateXlibSurfaceKHR(m_vk_Instance, &surfaceCreateInfo, nullptr, &m_vk_Surface);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "Vulkan: Couldn't create the surface\n");
		return false;
	}
#endif
#ifdef VK_USE_PLATFORM_WIN32_KHR
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hwnd = (HWND)((GSWndVK*)m_wnd.get())->GetDisplay();
	surfaceCreateInfo.hinstance = GetModuleHandle(nullptr);
	result = vkCreateWin32SurfaceKHR(m_vk_Instance, &surfaceCreateInfo, nullptr, &m_vk_Surface);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "Vulkan: Couldn't create the surface\n");
		return false;
	}
#endif

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_vk_PhysicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_vk_PhysicalDevice, &queueFamilyCount, queueFamilies.data());
	int i = 0;
	bool queueIndicesFound = false;
	for (const auto& queueFamily : queueFamilies) {
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(m_vk_PhysicalDevice, i, m_vk_Surface, &presentSupport);
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			m_vk_graphicsFamily = i;
		}
		if (queueFamily.queueCount > 0 && presentSupport) {
			m_vk_presentFamily = i;
		}
		if (m_vk_graphicsFamily != std::numeric_limits<uint32_t>::max() && m_vk_presentFamily != std::numeric_limits<uint32_t>::max()) {
			queueIndicesFound = true;
			break;
		}
		i++;
	}
	if (!queueIndicesFound) {
		fprintf(stderr, "Vulkan: Couldn't get the required queue family indices\n");
		return false;
	}
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {
		m_vk_graphicsFamily, m_vk_presentFamily
	};
	float queuePrirority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = m_vk_graphicsFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePrirority;
		queueCreateInfos.push_back(queueCreateInfo);
	}
	VkPhysicalDeviceFeatures deviceFeatures = {};
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(m_vk_deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = m_vk_deviceExtensions.data();
	deviceCreateInfo.enabledLayerCount = 0;
	result = vkCreateDevice(m_vk_PhysicalDevice, &deviceCreateInfo, nullptr, &m_vk_LogicalDevice);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "Vulkan: Couldn't create the logical device\n");
		return false;
	}
	if (!Vulkan::load_device_functions(m_vk_LogicalDevice)) {
		fprintf(stderr, "Vulkan: Couldn't load device functions");
		return false;
	}
	vkGetDeviceQueue(m_vk_LogicalDevice, m_vk_graphicsFamily, 0, &m_vk_GraphicsQueue);
	vkGetDeviceQueue(m_vk_LogicalDevice, m_vk_presentFamily, 0, &m_vk_PresentQueue);

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_vk_PhysicalDevice, m_vk_Surface, &surfaceCapabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_vk_PhysicalDevice, m_vk_Surface, &formatCount, nullptr);
	if (formatCount == 0) {
		fprintf(stderr, "Vulkan: This physical device doesn't support any surfaces");
		return false;
	}
	std::vector<VkSurfaceFormatKHR> formats;
	formats.resize(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_vk_PhysicalDevice, m_vk_Surface, &formatCount, formats.data());
	VkSurfaceFormatKHR selectedSwapSurfaceFormat;
	for (const auto& availableFormat : formats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
			selectedSwapSurfaceFormat = availableFormat;
			break;
		}
	}
	m_vk_swapChainFormat = selectedSwapSurfaceFormat.format;

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_vk_PhysicalDevice, m_vk_Surface, &presentModeCount, nullptr);
	if (presentModeCount == 0) {
		fprintf(stderr, "Vulkan: This physical device doesn't support any present modes");
		return false;
	}
	std::vector<VkPresentModeKHR> presentModes;
	presentModes.resize(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_vk_PhysicalDevice, m_vk_Surface, &presentModeCount, presentModes.data());
	VkPresentModeKHR selectedPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	for (const auto& presentMode : presentModes) {
		if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			selectedPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			break;
		}
	}

	if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		m_vk_swapExtent = surfaceCapabilities.currentExtent;
	} else {
		m_vk_swapExtent = {640, 480};	// 1x Native, TODO: Make it configurable

		m_vk_swapExtent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, m_vk_swapExtent.width));
		m_vk_swapExtent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, m_vk_swapExtent.height));
	}

	uint32_t swapImageCount = surfaceCapabilities.minImageCount + 1;
	if (surfaceCapabilities.maxImageCount > 0 && swapImageCount > surfaceCapabilities.maxImageCount) {
		swapImageCount = surfaceCapabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = m_vk_Surface;
	swapchainCreateInfo.imageFormat = selectedSwapSurfaceFormat.format;
	swapchainCreateInfo.imageColorSpace = selectedSwapSurfaceFormat.colorSpace;
	swapchainCreateInfo.imageExtent = m_vk_swapExtent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	uint32_t queueFamilyIndices[] = {m_vk_graphicsFamily, m_vk_presentFamily};
	if (m_vk_graphicsFamily == m_vk_presentFamily) {
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainCreateInfo.queueFamilyIndexCount = 2;
		swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfo.queueFamilyIndexCount = 0;
		swapchainCreateInfo.pQueueFamilyIndices = nullptr;
	}
	swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = selectedPresentMode;
	swapchainCreateInfo.clipped = VK_TRUE;
	swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

	result = vkCreateSwapchainKHR(m_vk_LogicalDevice, &swapchainCreateInfo, nullptr, &m_vk_SwapChain);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "Vulkan: Swapchain creation failed");
		return false;
	}
	uint32_t imageCount;
	vkGetSwapchainImagesKHR(m_vk_LogicalDevice, m_vk_SwapChain, &imageCount, nullptr);
	m_vk_SwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_vk_LogicalDevice, m_vk_SwapChain, &imageCount, m_vk_SwapChainImages.data());

	m_vk_SwapChainImageViews.resize(m_vk_SwapChainImages.size());
	for (size_t i = 0; i < m_vk_SwapChainImages.size(); i++) {
		VkImageViewCreateInfo imageViewCreateInfo = {};
		imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewCreateInfo.image = m_vk_SwapChainImages[i];
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = m_vk_swapChainFormat;
		imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;
		
		result = vkCreateImageView(m_vk_LogicalDevice, &imageViewCreateInfo, nullptr, &m_vk_SwapChainImageViews[i]);
		if (result != VK_SUCCESS) {
			fprintf(stderr, "Vulkan: Couldn't create image %i", i);
			return false;
		}
	}

	return true;
}

void GSDeviceVK::createPipeline()
{
	VkResult result;
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)m_vk_swapExtent.width;
	viewport.height = (float)m_vk_swapExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor = {};
	scissor.offset = {0, 0};
	scissor.extent = m_vk_swapExtent;
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	VkPipelineDepthStencilStateCreateInfo stencilInfo = {};
	stencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	stencilInfo.stencilTestEnable = VK_FALSE;	// TODO: Impliment depth stencil pipelineInfo.pDepthStencilState
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;	// NO BLENDING. TODO: revise VK colour blending
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;	// this also states no blending, check Fixed Functions chapter of vulkan tutorial
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	// TODO: decide whether we want a dynamic pipeline state here
	// be sure to set pipelineInfo.pDynamicState

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;	// We'll need this for uniforms
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;
	result = vkCreatePipelineLayout(m_vk_LogicalDevice, &pipelineLayoutInfo, nullptr, &m_vk_PipelineLayout);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "Vulkan: Couldn't create pipeline layout");
		throw GSDXError();
	}

	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = m_vk_swapChainFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	result = vkCreateRenderPass(m_vk_LogicalDevice, &renderPassInfo, nullptr, &m_vk_renderPass);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "Vulkan: Couldn't create the render pass");
		throw GSDXError();
	}


	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	//pipelineInfo.pStages = shaderStages; // TODO: SHADERS!!!
	//pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr; // Optional
	result = vkCreateGraphicsPipelines(m_vk_LogicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_vk_currentPipeline);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "Vulkan: Couldn't create the graphics pipeline");
		throw GSDXError();
	}
}

bool GSDeviceVK::Reset(int w, int h)
{
	return GSDevice::Reset(w, h);
}

GSTexture* GSDeviceVK::CreateSurface(int type, int w, int h, int format)
{
	return new GSTextureVK(type, w, h, format);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    fprintf(stderr, "Vulkan message: %s\n", pCallbackData->pMessage);

    return VK_FALSE;
}