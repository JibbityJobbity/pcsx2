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