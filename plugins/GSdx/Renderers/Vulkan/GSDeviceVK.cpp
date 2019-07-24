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
#include "GSTextureVK.h"

GSDeviceVK::GSDeviceVK()
{
	Vulkan::LoadVulkan();
}

bool GSDeviceVK::Create(const std::shared_ptr<GSWnd> &wnd)
{
	m_wnd = wnd;

	if (!InitInstance()) {
		return false;
	}

	return true;
}

bool GSDeviceVK::InitInstance()
{
	VkResult result;

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "GSdx";
	appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	appInfo.pEngineName = "GSdx";
	appInfo.engineVersion = VK_MAKE_VERSION(0, 1, 0);
	appInfo.apiVersion = VK_API_VERSION_1_1;

	#ifdef VK_USE_PLATFORM_XLIB_KHR
		m_vk.instanceExtensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
	#endif
	#ifdef VK_USE_PLATFORM_WIN32_KHR
		m_vk.instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
	#endif

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(m_vk.instanceExtensions.size());
	instanceCreateInfo.ppEnabledExtensionNames = m_vk.instanceExtensions.data();
	instanceCreateInfo.enabledLayerCount = 0;
	instanceCreateInfo.ppEnabledLayerNames = nullptr;
	result = vkCreateInstance(&instanceCreateInfo, nullptr, &m_vk.instance);

	if (result != VK_SUCCESS) {
		fprintf(stderr, "VULKAN: Couldn't create the Vulkan instance\n");
		return false;
	}
	if (!Vulkan::LoadInstanceFunctions(m_vk.instance)) {
		fprintf(stderr, "VULKAN: Error loading instance functions\n");
		return false;
	}

	return true;
}

GSDeviceVK::~GSDeviceVK()
{
	vkDestroyInstance(m_vk.instance, nullptr);
	Vulkan::ReleaseVulkan();
}

GSTexture* GSDeviceVK::CreateSurface(int type, int w, int h, int format)
{
	return new GSTextureVK(type, w, h, format);
}

bool GSDeviceVK::Reset(int w, int h)
{
	return GSDevice::Reset(w, h);
}