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
#include "vulkanloader.hpp"
#include <vector>
#include <limits>
#include <set>

class GSDeviceVK : public GSDevice {
private:
	GSTexture* CreateSurface(int type, int w, int h, int format);

	void DoMerge(GSTexture* sTex[3], GSVector4* sRect, GSTexture* dTex, GSVector4* dRect, const GSRegPMODE& PMODE, const GSRegEXTBUF& EXTBUF, const GSVector4& c) {}
	void DoInterlace(GSTexture* sTex, GSTexture* dTex, int shader, bool linear, float yoffset = 0) {}
	uint16 ConvertBlendEnum(uint16 generic) { return 0xFFFF; }
protected:
	struct {
		std::vector<const char*> deviceExtensions = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
		std::vector<const char*> instanceExtensions = {
			VK_KHR_SURFACE_EXTENSION_NAME
		};
		VkInstance instance;
		VkPhysicalDevice physicalDevice;
		VkDevice logicalDevice;
		VkSurfaceKHR surface;
		uint32_t graphicsFamily = std::numeric_limits<uint32_t>::max(), 
			presentFamily = std::numeric_limits<uint32_t>::max();
	} m_vk;

	VkPhysicalDevice PickDevice(std::vector<VkPhysicalDevice>& devices);

	bool InitInstance();
	bool CreateDevice();

public:
	GSDeviceVK();
	~GSDeviceVK();

	bool Create(const std::shared_ptr<GSWnd> &wnd);
	bool Reset(int w, int h);
};