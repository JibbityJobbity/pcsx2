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

#pragma once

#include "Renderers/Common/GSDevice.h"
#include "GSTextureVK.h"
#include "Window/GSWndVK.h"

#ifdef __linux__
#define VK_USE_PLATFORM_XLIB_KHR
#include <X11/Xlib.h>
#include <GL/glx.h>
#elif defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include "vulkanloader.hpp"
#include <vector>
#include <set>
#include <limits>
#include <array>


class GSDeviceVK : public GSDevice
{
private:
	const bool enableValidationLayers = true;
	GSTexture* CreateSurface(int type, int w, int h, int format);

	void DoMerge(GSTexture* sTex[3], GSVector4* sRect, GSTexture* dTex, GSVector4* dRect, const GSRegPMODE& PMODE, const GSRegEXTBUF& EXTBUF, const GSVector4& c) {}
	void DoInterlace(GSTexture* sTex, GSTexture* dTex, int shader, bool linear, float yoffset = 0) {}
	uint16 ConvertBlendEnum(uint16 generic) { return 0xFFFF; }

protected:
	std::vector<const char*> m_vk_deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	std::vector<const char*> m_vk_instanceExtensions = {
		VK_KHR_SURFACE_EXTENSION_NAME
	};
	std::vector<const char*> m_vk_layers;
	VkDebugUtilsMessengerEXT	m_vk_debugMessenger;
	VkInstance	m_vk_Instance;
	uint32_t	m_vk_graphicsFamily = std::numeric_limits<uint32_t>::max();
	uint32_t	m_vk_presentFamily = std::numeric_limits<uint32_t>::max();
	VkPhysicalDevice	m_vk_PhysicalDevice = VK_NULL_HANDLE;
	VkDevice	m_vk_LogicalDevice = VK_NULL_HANDLE;
	VkQueue		m_vk_GraphicsQueue;
	VkQueue		m_vk_PresentQueue;
	VkSurfaceKHR	m_vk_Surface;
	VkSwapchainKHR	m_vk_SwapChain;
	VkExtent2D	m_vk_swapExtent;
	VkFormat	m_vk_swapChainFormat;
	VkPipelineLayout m_vk_PipelineLayout;
	VkPipeline	m_vk_currentPipeline;
	VkRenderPass	m_vk_renderPass;
	std::vector<VkImage>	m_vk_SwapChainImages;
	std::vector<VkImageView>	m_vk_SwapChainImageViews;

	void createPipeline();

public:
	GSDeviceVK();
	~GSDeviceVK();

	bool Create(const std::shared_ptr<GSWnd> &wnd);
	bool Reset(int w, int h);
	//void Present(const GSVector4i& r, int shader);
	//void Present(GSTexture* sTex, GSTexture* dTex, const GSVector4& dRect, int shader = 0);

	//void SetVsync(int vsync);

	//void BeginScene();
	//void DrawPrimitive();
	//void DrawIndexedPrimitive();
	//void DrawIndexedPrimitive(int offset, int count);

	void IASetVertexBuffer(const void* vertices, size_t count);
	void IASetIndexBuffer(const void* index, size_t count);
	void IASetPrimitiveTopology(GLenum topology)
};
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);
