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
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <vulkan/vulkan.h>

class GSWndVK final : public GSWnd
{
protected:
	Window		m_NativeWindow;
	Display* 	m_NativeDisplay;
	bool		m_has_late_vsync;

	VkInstance	m_vk_Instance;
	VkPhysicalDevice	m_vk_PhysicalDevice = VK_NULL_HANDLE;
	VkDevice	m_vk_LogicalDevice = VK_NULL_HANDLE;

	void InitVulkan();

public:
	GSWndVK();
	virtual ~GSWndVK();

	bool Create(const std::string& title, int w, int h);
};

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;

	bool isComplete() {
		return graphicsFamily.has_value();
	}	
};

#endif