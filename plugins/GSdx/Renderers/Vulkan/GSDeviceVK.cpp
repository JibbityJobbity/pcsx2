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

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

GSDeviceVK::GSDeviceVK()
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
			0,
			nullptr,
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

	return true;
}

GSDeviceVK::~GSDeviceVK()
{
}

GSTexture* GSDeviceVK::CreateSurface(int type, int w, int h, int format)
{
	return new GSTextureVK(type, w, h, format);
} 

bool GSDeviceVK::Reset(int w, int h)
{
	return GSDevice::Reset(w, h);
}