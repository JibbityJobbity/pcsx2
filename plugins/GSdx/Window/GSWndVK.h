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

#include "GSWnd.h"

#if defined(__unix__)
#define VK_USE_PLATFORM_XLIB_KHR
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <vulkan/vulkan.h>
#include <vector>
#include <set>
#include <limits>

class GSWndVK : public GSWnd
{
protected:
	int m_w, m_h;
	const std::vector<const char*> vk_deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	Window		m_NativeWindow;
	Display* 	m_NativeDisplay;
	bool		m_has_late_vsync;

	uint32_t	m_vk_graphicsFamily = std::numeric_limits<uint32_t>::max();
	uint32_t	m_vk_presentFamily = std::numeric_limits<uint32_t>::max();
	VkInstance	m_vk_Instance;
	VkPhysicalDevice	m_vk_PhysicalDevice = VK_NULL_HANDLE;
	VkDevice	m_vk_LogicalDevice = VK_NULL_HANDLE;
	VkQueue		m_vk_GraphicsQueue;
	VkQueue		m_vk_PresentQueue;
	VkSurfaceKHR	m_vk_Surface;
	VkSwapchainKHR	m_vk_SwapChain;
	VkExtent2D	m_vk_swapExtent;
	VkFormat	m_vk_swapChainFormat;
	std::vector<VkImage>	m_vk_SwapChainImages;
	std::vector<VkImageView>	m_vk_SwapChainImageViews;

	void InitVulkan();

public:
	GSWndVK();
	~GSWndVK();

	bool Create(const std::string& title, int w, int h);
	bool Attach(void* handle, bool managed = true);
	void Detach(){};
	void* GetDisplay();
	void* GetHandle(){return nullptr;}
	GSVector4i GetClientRect(){return GSVector4i();}
	bool SetWindowText(const char* title) {return true;}
	void Show(){};
	void Hide(){};
	void HideFrame(){};
};

#endif