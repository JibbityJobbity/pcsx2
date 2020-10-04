/*
 *	Copyright (C) 2020 PCSX2 Dev Team
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
#include "stdafx.h"

#define VULKAN_HPP_DISPATCH_LOADER_DYNAMIC 1
#include "vulkan/vulkan.hpp"

#include <vector>

struct VKModuleInfo {
	vk::UniqueShaderModule module;
	std::string name;
	vk::ShaderStageFlagBits stageType;
};

struct GSInputAttributeVK {
	vk::Format format;
	uint32_t offset;
};

class GSPipelineVK {
protected:
	std::vector<VKModuleInfo> m_modules;
	std::vector<vk::VertexInputAttributeDescription> m_vertex_attributes;
	uint32_t m_vertex_size;
	vk::UniquePipelineLayout m_layout;
	vk::UniquePipeline m_pipeline;
	vk::Viewport m_viewport;
	vk::Rect2D m_scissor;
public:
	void AddShader(vk::UniqueDevice& dev, int id, vk::ShaderStageFlagBits usage);
	void SetDims(vk::Extent2D& extent);
	void SetVertexAttributes(std::vector<GSInputAttributeVK>& attributes, uint32_t vertexSize);
	void Initialize(vk::UniqueDevice& dev, vk::RenderPass renderPass);
	void Bind(vk::UniqueCommandBuffer& commandBuffer, vk::PipelineBindPoint& bindPoint);
};