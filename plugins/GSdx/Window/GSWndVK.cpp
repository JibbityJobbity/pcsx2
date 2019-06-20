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

#include "GSWndVK.h"

GSWndVK::GSWndVK()
{
	
}

void GSWndVK::InitVulkan()
{
	/*
	 *	This function is just a bunch of Vulkan setup stuff.
	 *	I feel like it's fairly justified given the amount
	 *	of control devs are responsible for. I'll fix it if
	 *	someone bugs me enough.
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

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	std::vector<const char*> extensions = {
		"VK_KHR_surface",
		"VK_KHR_xlib_surface"
	};
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();
	createInfo.enabledLayerCount = 0;
	VkResult result = vkCreateInstance(&createInfo, nullptr, &m_vk_Instance);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "Vulkan: Couldn't create instance\n");
		throw GSDXError();
	}

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_vk_Instance, &deviceCount, nullptr);
	if (deviceCount == 0) {
		fprintf(stderr, "Vulkan: Couldn't get physical devices\n");
		throw GSDXError();
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_vk_Instance, &deviceCount, devices.data());
	m_vk_PhysicalDevice = devices.at(0);
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(m_vk_PhysicalDevice, &deviceProperties);
	fprintf(stdout, "Vulkan information:\n");
	fprintf(stdout, "\t%s\n\t%s\n", 
		deviceProperties.deviceName, 
		deviceProperties.driverVersion);

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_vk_PhysicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_vk_PhysicalDevice, &queueFamilyCount, queueFamilies.data());
	int i = 0;
	bool queueIndicesFound = false;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			m_vk_graphicsFamily = i;
		}
		if (m_vk_graphicsFamily != -1) {
			queueIndicesFound = true;
			break;
		}
		i++;
	}
	if (!queueIndicesFound) {
		fprintf(stderr, "Vulkan: Couldn't get the required queue family indices\n");
		throw GSDXError();
	}

	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = m_vk_graphicsFamily;
	queueCreateInfo.queueCount = 1;
	float queuePrirority = 1.0f;
	queueCreateInfo.pQueuePriorities = &queuePrirority;
	
}

bool GSWndVK::Create(const std::string& title, int w, int h) 
{
	if (m_NativeWindow)
		throw GSDXRecoverableError();
	
	if (w <= 0 || w <= 0) {
		w = theApp.GetConfigI("ModeWidth");
		h = theApp.GetConfigI("ModeHeight");
	}

	m_managed = true;
	// Something was mentioned in GSWndOGL.cpp about this line
	// "note this part must be only executed when replaying .gs debug file"
	m_NativeDisplay = XOpenDisplay(NULL);

	m_NativeWindow = XCreateSimpleWindow(m_NativeDisplay, DefaultRootWindow(m_NativeDisplay), 0, 0, w, h, 0, 0, 0);
	XMapWindow(m_NativeDisplay, m_NativeWindow);

	if (m_NativeWindow == 0)
		throw GSDXRecoverableError();
	
	InitVulkan();

	return true;
}