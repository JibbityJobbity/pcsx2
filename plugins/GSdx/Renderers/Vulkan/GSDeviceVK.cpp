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

GSDeviceVK::GSDeviceVK()
{
	Vulkan::LoadVulkan();
}

bool GSDeviceVK::Create(const std::shared_ptr<GSWnd> &wnd)
{
	m_wnd = wnd;

	m_vk.instance = InitInstance();
	if (m_vk.instance == VK_NULL_HANDLE)
		return false;

	return true;
}

VkInstance GSDeviceVK::InitInstance()
{
	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "GSdx";
	app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
	app_info.pEngineName = "GSdx";
	app_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
	app_info.apiVersion = VK_API_VERSION_1_1;

	VkInstanceCreateInfo instance_create_info = {};
	instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_create_info.pApplicationInfo = &app_info;
	instance_create_info.enabledExtensionCount = static_cast<uint32_t>(m_vk.instance_extensions.size());
	instance_create_info.ppEnabledExtensionNames = m_vk.instance_extensions.data();
	instance_create_info.enabledLayerCount = 0;
	instance_create_info.ppEnabledLayerNames = nullptr;

	VkInstance out = VK_NULL_HANDLE;
	VkResult result = vkCreateInstance(&instance_create_info, nullptr, &out);

	if (result != VK_SUCCESS) {
		fprintf(stderr, "VULKAN: Couldn't create the Vulkan instance\n");
		return VK_NULL_HANDLE;
	}

	if (!Vulkan::LoadInstanceFunctions(out)) {
		fprintf(stderr, "VULKAN: Error loading instance functions\n");
		return VK_NULL_HANDLE;
	}

	return out;
}

GSDeviceVK::~GSDeviceVK()
{
	CleanInstance(m_vk.instance);
	Vulkan::ReleaseVulkan();
}

void GSDeviceVK::CleanInstance(VkInstance instance)
{
	if (instance != VK_NULL_HANDLE)
		vkDestroyInstance(instance, nullptr);
}

GSTexture* GSDeviceVK::CreateSurface(int type, int w, int h, int format)
{
	return new GSTextureVK(type, w, h, format);
}

bool GSDeviceVK::Reset(int w, int h)
{
	return GSDevice::Reset(w, h);
}