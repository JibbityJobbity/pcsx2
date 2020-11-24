/*
 *	Copyright (C) 2019 PCSX2 Dev Team
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
#include "GSTextureVK.h"
#ifdef _WIN32
#include "Window/GSWndWVK.h"
#else
#include "Window/GSWndXVK.h"
#endif

#ifdef _WIN32
#include "resource.h"
#else
#include "GSdxResources.h"
#endif

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	FILE* outStream;
	const char* prefix;

	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
	{
		prefix = "VERBOSE: ";
		outStream = stdout;
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
	{
		prefix = "INFO: ";
		outStream = stdout;
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		prefix = "WARNING: ";
		outStream = stderr;
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		prefix = "ERROR: ";
		outStream = stderr;
	}

	fprintf(outStream, "VK%s[%d][%s] : %s\n", prefix, pCallbackData->messageIdNumber, pCallbackData->pMessageIdName, pCallbackData->pMessage);

	return VK_FALSE;
}

GSDeviceVK::GSDeviceVK() : m_convert(GSPipelineVK(nullptr))
{
}

bool GSDeviceVK::Create(const std::shared_ptr<GSWnd> &wnd)
{
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);
	m_wnd = wnd;

	// Create instance
	{
		vk::ApplicationInfo appInfo(
			"GSdx",
			VK_MAKE_VERSION(0, 1, 0),
			"GSdx",
			VK_MAKE_VERSION(0, 1, 0),
			VK_API_VERSION_1_1
		);

		vk::InstanceCreateInfo instanceCreateInfo(
			{},
			&appInfo,
			static_cast<uint32_t>(m_vk.instance_layers.size()),
			m_vk.instance_layers.data(),
			static_cast<uint32_t>(m_vk.instance_extensions.size()),
			m_vk.instance_extensions.data()
		);

		try
		{
			m_vk.instance = vk::createInstanceUnique(instanceCreateInfo);
			VULKAN_HPP_DEFAULT_DISPATCHER.init(*m_vk.instance);
		}
		catch (vk::SystemError ex)
		{
			fprintf(stderr, "VULKAN: Couldn't create an instance, reason is %s\n", ex.what());
			return false;
		}
	}

	// Create surface
	{
#ifdef VK_USE_PLATFORM_XLIB_KHR
		vk::XlibSurfaceCreateInfoKHR createInfo(
			{},
			(Display*)m_wnd->GetDisplay(),
			(Window)m_wnd->GetHandle()
		);
		try
		{
			m_vk.surface = m_vk.instance->createXlibSurfaceKHRUnique(createInfo);
		}
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
		vk::Win32SurfaceCreateInfoKHR createInfo(
			{},
			GetModuleHandle(nullptr),
			(HWND)m_wnd->GetDisplay()
		);
		try
		{
			m_vk.surface = m_vk.instance->createWin32SurfaceKHRUnique(createInfo);
		}
#else
#error Your platform is not supported
#endif

		catch (vk::SystemError ex)
		{
			fprintf(stderr, "VULKAN: Couldn't create a surface, reason is %s\n", ex.what());
			return false;
		}

		m_vk.surface_format = {
			vk::Format::eB8G8R8A8Unorm,
			vk::ColorSpaceKHR::eSrgbNonlinear
		};
	}

#ifndef NDEBUG
	// Create debug messenger
	{
		auto severityFlags = vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
			| vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
			| vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose
			| vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo;

		auto typeFlags = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
			| vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
			| vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

		try
		{
			m_vk.debug_messenger = m_vk.instance->createDebugUtilsMessengerEXTUnique(
				{{}, severityFlags, typeFlags, debugCallback},
				nullptr
			);
		}
		catch (vk::SystemError ex)
		{
			fprintf(stderr, "VULKAN: Couldn't create debug messenger, reason is %s\n", ex.what());
		}

	}
#endif

	// Create logical device
	{
		auto devs = m_vk.instance->enumeratePhysicalDevices();
		if (devs.size() <= 0)
		{
			fprintf(stderr, "VULKAN: No physical devices supported");
			return false;
		}
		m_vk.physical_dev = devs[0];

		auto queueProperties = m_vk.physical_dev.getQueueFamilyProperties();
		for (uint32_t i = 0; i < queueProperties.size(); i++)
		{
			if (queueProperties[i].queueFlags & vk::QueueFlagBits::eGraphics)
				m_vk.graphics_queue_index = i;
			if (queueProperties[i].queueFlags & vk::QueueFlagBits::eCompute)
				m_vk.compute_queue_index = i;
			if (queueProperties[i].queueFlags & vk::QueueFlagBits::eTransfer)
				m_vk.transfer_queue_index = i;
			if (m_vk.physical_dev.getSurfaceSupportKHR(i, *m_vk.surface) == VK_TRUE)
				m_vk.present_queue_index = i;
		}

		float priority = 1.0f;
		std::set<uint32_t> uniqueQueueIndices {
			m_vk.graphics_queue_index,
			m_vk.compute_queue_index,
			m_vk.transfer_queue_index,
			m_vk.present_queue_index
		};
		std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
		queueCreateInfos.reserve(uniqueQueueIndices.size());
		for (uint32_t i : uniqueQueueIndices) {
			vk::DeviceQueueCreateInfo c(
				{},
				i,
				1,
				&priority
			);
			queueCreateInfos.push_back(c);
		}

		vk::PhysicalDeviceFeatures features = m_vk.physical_dev.getFeatures();
		vk::DeviceCreateInfo createInfo(
			{},
			static_cast<uint32_t>(queueCreateInfos.size()),
			queueCreateInfos.data(),
			0,
			nullptr,
			static_cast<uint32_t>(m_vk.device_extensions.size()),
			m_vk.device_extensions.data(),
			&features
		);

		try 
		{
			m_vk.device = m_vk.physical_dev.createDeviceUnique(createInfo);
			m_vk.graphics_queue = m_vk.device->getQueue(m_vk.graphics_queue_index, 0);
			m_vk.compute_queue = m_vk.device->getQueue(m_vk.compute_queue_index, 0);
			m_vk.transfer_queue = m_vk.device->getQueue(m_vk.transfer_queue_index, 0);
			m_vk.present_queue = m_vk.device->getQueue(m_vk.present_queue_index, 0);
		}
		catch (vk::SystemError ex)
		{
			fprintf(stderr, "VULKAN: Couldn't create a device, reason is %s\n", ex.what());
			return false;
		}
	}

	// Create render pass
	{
		vk::AttachmentDescription colorAttachment(
			{},
			m_vk.surface_format.format,
			vk::SampleCountFlagBits::e1,
			vk::AttachmentLoadOp::eClear,
			vk::AttachmentStoreOp::eStore,
			vk::AttachmentLoadOp::eDontCare,
			vk::AttachmentStoreOp::eDontCare,
			vk::ImageLayout::eUndefined,
			vk::ImageLayout::ePresentSrcKHR
		);
		vk::AttachmentReference attachmentRef(
			0,
			vk::ImageLayout::eColorAttachmentOptimal
		);
		vk::SubpassDescription subpassDesc(
			{},
			vk::PipelineBindPoint::eGraphics,
			0,
			nullptr,
			1,
			&attachmentRef
		);
		vk::RenderPassCreateInfo renderPassInfo(
			{},
			1,
			&colorAttachment,
			1,
			&subpassDesc,
			0,
			nullptr
		);
		m_vk.render_pass = m_vk.device->createRenderPassUnique(renderPassInfo);
	}

	// Create swapchain
	try
	{
		createSwapChain();
	}
	catch (vk::SystemError ex)
	{
		fprintf(stderr, "VULKAN: Couldn't create the swapchain, reason is %s\n", ex.what());
		return false;
	}

	m_convert = GSPipelineVK(*m_vk.device);
	std::vector<GSInputAttributeVK> convertLayout = {
		{ vk::Format::eR32G32Sfloat,  0  },
		{ vk::Format::eR32G32Sfloat,  16 },
		{ vk::Format::eR8G8B8A8Uint,  8  },
		{ vk::Format::eR32Sfloat,     12 },
		{ vk::Format::eR16G16Uint,    16 },
		{ vk::Format::eR32Uint,       20 },
		{ vk::Format::eR16G16Uint,    24 },
		{ vk::Format::eR8G8B8A8Unorm, 28 }
	};
	m_convert.SetVertexAttributes(convertLayout, sizeof(GSVertex));
	std::vector<vk::DescriptorSetLayoutBinding> convertDescriptorBindings = {
		{ 0, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment },
		{ 15, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment },
		{ 20, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eGeometry },
		{ 21, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment }
	};
	m_convert.SetDescriptorSetLayoutBindings(convertDescriptorBindings);
	m_convert.SetDims(m_vk.swap_extent);
	m_convert.AddShader(IDR_CONVERT_PS0_SPV, vk::ShaderStageFlagBits::eFragment);
	m_convert.AddShader(IDR_CONVERT_VS_SPV, vk::ShaderStageFlagBits::eVertex);
	m_convert.Initialize(*m_vk.render_pass);

	return true;
}

void GSDeviceVK::createSwapChain()
{
	vk::SurfaceCapabilitiesKHR capabilities = m_vk.physical_dev.getSurfaceCapabilitiesKHR(*m_vk.surface);
	auto presentModes = m_vk.physical_dev.getSurfacePresentModesKHR(*m_vk.surface);

	vk::PresentModeKHR desiredPresentMode;
	switch(m_vsync)
	{
		default:
		case 0:
			desiredPresentMode = vk::PresentModeKHR::eImmediate;
			break;
		case 1:
			desiredPresentMode = vk::PresentModeKHR::eFifo;
			break;
		case -1:
			desiredPresentMode = vk::PresentModeKHR::eFifoRelaxed;
			break;
	}

	if (capabilities.currentExtent.width != UINT32_MAX)
		m_vk.swap_extent = capabilities.currentExtent;
	else
	{
		auto windowDims = m_wnd->GetClientRect();
		m_vk.swap_extent = vk::Extent2D{static_cast<uint32_t>(windowDims.z), static_cast<uint32_t>(windowDims.w)};
		m_vk.swap_extent.setWidth(std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, m_vk.swap_extent.width)));
		m_vk.swap_extent.setHeight(std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, m_vk.swap_extent.height)));
	}
	
	uint32_t imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0)
		imageCount = std::min(capabilities.maxImageCount, imageCount);

	uint32_t indices[] = {
		m_vk.graphics_queue_index,
		m_vk.present_queue_index
	};
	auto sharingMode = m_vk.graphics_queue_index == m_vk.present_queue_index ?
		vk::SharingMode::eExclusive : vk::SharingMode::eConcurrent;
	vk::SwapchainCreateInfoKHR swapCreateInfo(
		{},
		*m_vk.surface,
		imageCount,
		m_vk.surface_format.format,
		m_vk.surface_format.colorSpace,
		m_vk.swap_extent,
		1,
		vk::ImageUsageFlagBits::eColorAttachment,
		sharingMode,
		2,
		indices,
		capabilities.currentTransform,
		vk::CompositeAlphaFlagBitsKHR::eOpaque,
		desiredPresentMode,
		VK_TRUE,
		*m_vk.swapchain
	);

	m_vk.swapchain = m_vk.device->createSwapchainKHRUnique(swapCreateInfo);
	m_vk.swapchain_image_views.clear();
	m_vk.swapchain_images = m_vk.device->getSwapchainImagesKHR(*m_vk.swapchain);
	vk::ImageSubresourceRange isr(
		vk::ImageAspectFlagBits::eColor,
		0,
		1,
		0,
		1
	);

	m_vk.framebuffers.clear();
	for (const auto& i : m_vk.swapchain_images)
	{
		vk::ImageViewCreateInfo imageViewCreateInfo(
			{},
			i,
			vk::ImageViewType::e2D,
			m_vk.surface_format.format,
			{vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity},
			isr
		);
		m_vk.swapchain_image_views.push_back(m_vk.device->createImageViewUnique(imageViewCreateInfo));

		vk::FramebufferCreateInfo framebufferInfo(
			{},
			*m_vk.render_pass,
			1,
			&*m_vk.swapchain_image_views.back(),
			m_vk.swap_extent.width,
			m_vk.swap_extent.height,
			1
		);
		m_vk.framebuffers.push_back(m_vk.device->createFramebufferUnique(framebufferInfo));
	}
}

GSDeviceVK::~GSDeviceVK()
{
}

void GSDeviceVK::SetVSync(int vsync)
{
	if (m_vsync != vsync)
	{
		m_vsync = vsync;
		createSwapChain();
	}
}

GSTexture* GSDeviceVK::CreateSurface(int type, int w, int h, int format)
{
	return new GSTextureVK(type, w, h, format);
} 

bool GSDeviceVK::Reset(int w, int h)
{
	return GSDevice::Reset(w, h);
}