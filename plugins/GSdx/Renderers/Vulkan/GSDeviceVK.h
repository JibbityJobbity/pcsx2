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
#pragma once

#ifdef __linux
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include "Renderers/Common/GSDevice.h"
#include "GSPipelineVK.h"
#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include "vk_mem_alloc.hpp"
#include <vector>

class GSDeviceVK : public GSDevice {
private:
	vk::DynamicLoader dl;
	GSTexture* CreateSurface(int type, int w, int h, int format);

	void DoMerge(GSTexture* sTex[3], GSVector4* sRect, GSTexture* dTex, GSVector4* dRect, const GSRegPMODE& PMODE, const GSRegEXTBUF& EXTBUF, const GSVector4& c) {}
	void DoInterlace(GSTexture* sTex, GSTexture* dTex, int shader, bool linear, float yoffset = 0) {}
	uint16 ConvertBlendEnum(uint16 generic) { return 0xFFFF; }
protected:
	struct {
		const std::vector<const char*> instance_extensions = {
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_XLIB_KHR
			VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
#ifndef NDEBUG
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
		};
		const std::vector<const char*> instance_layers = {
#ifndef NDEBUG
			"VK_LAYER_KHRONOS_validation",
#endif
		};
		const std::vector<const char*> device_extensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			VK_EXT_MEMORY_BUDGET_EXTENSION_NAME,
		};
		uint32_t graphics_queue_index = UINT32_MAX;
		uint32_t compute_queue_index = UINT32_MAX;
		uint32_t transfer_queue_index = UINT32_MAX;
		uint32_t present_queue_index = UINT32_MAX;
		vk::UniqueInstance instance;
		vk::UniqueSurfaceKHR surface;
		vk::SurfaceFormatKHR surface_format;
		vk::PhysicalDevice physical_dev;
#ifndef NDEBUG
		vk::UniqueDebugUtilsMessengerEXT debug_messenger;
#endif
		vk::UniqueDevice device;
		vk::Queue graphics_queue;
		vk::Queue compute_queue;
		vk::Queue transfer_queue;
		vk::Queue present_queue;
		vk::UniqueRenderPass render_pass;
		vk::UniqueSwapchainKHR swapchain;
		std::vector<vk::Image> swapchain_images;
		vk::Extent2D swap_extent;
		vk::Format swap_image_format;
		std::vector<vk::UniqueImageView> swapchain_image_views;
		std::vector<vk::UniqueFramebuffer> framebuffers;
		vma::Allocator allocator;
		vk::UniqueCommandPool command_pool;
		std::vector<vk::UniqueCommandBuffer> draw_command_buffers;
	} m_vk;
	GSPipelineVK m_convert;

	void createSwapChain();

public:
	GSDeviceVK();
	~GSDeviceVK();

	void SetVSync(int vsync) override;
	bool Create(const std::shared_ptr<GSWnd> &wnd);
	bool Reset(int w, int h);
}; 