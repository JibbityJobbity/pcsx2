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

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	std::vector<const char*> extensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_XLIB_SURFACE_EXTENSION_NAME
	};
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	instanceCreateInfo.ppEnabledExtensionNames = extensions.data();
	instanceCreateInfo.enabledLayerCount = 0;
	VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &m_vk_Instance);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "Vulkan: Couldn't create instance\n");
		throw GSDXError();
	}

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(m_vk_Instance, &deviceCount, nullptr);
	if (deviceCount == 0) {
		fprintf(stderr, "Vulkan: Couldn't detect any physical devices\n");
		throw GSDXError();
	}
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(m_vk_Instance, &deviceCount, devices.data());
	for (VkPhysicalDevice dev : devices) {
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(m_vk_PhysicalDevice, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(m_vk_PhysicalDevice, nullptr, &extensionCount, availableExtensions.data());
		std::set<std::string> requiredExtensions(vk_deviceExtensions.begin(), vk_deviceExtensions.end());
		for (const auto& extension: availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		if (requiredExtensions.empty()) {
			m_vk_PhysicalDevice = dev;
			break;
		}
	}
	if (m_vk_PhysicalDevice == VK_NULL_HANDLE) {
		fprintf(stderr, "Vulkan: Couldn't find a suitable physical device\n");
		throw GSDXError();
	}
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(m_vk_PhysicalDevice, &deviceProperties);
	fprintf(stdout, "Vulkan device information:\n");
	fprintf(stdout, "\t%s\n\t%i\n", 
		deviceProperties.deviceName, 
		deviceProperties.driverVersion);

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
		throw GSDXError();
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
	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(vk_deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = vk_deviceExtensions.data();
	deviceCreateInfo.enabledLayerCount = 0;
	result = vkCreateDevice(m_vk_PhysicalDevice, &deviceCreateInfo, nullptr, &m_vk_LogicalDevice);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "Vulkan: Couldn't create the logical device\n");
		throw GSDXError();
	}
	vkGetDeviceQueue(m_vk_LogicalDevice, m_vk_graphicsFamily, 0, &m_vk_GraphicsQueue);
	vkGetDeviceQueue(m_vk_LogicalDevice, m_vk_presentFamily, 0, &m_vk_PresentQueue);


	VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.dpy = m_NativeDisplay;
	surfaceCreateInfo.window = m_NativeWindow;
	result = vkCreateXlibSurfaceKHR(m_vk_Instance, &surfaceCreateInfo, nullptr, &m_vk_Surface);
	if (result != VK_SUCCESS) {
		fprintf(stderr, "Vulkan: Couldn't create the surface\n");
		throw GSDXError();
	}
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