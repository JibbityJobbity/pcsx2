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

#include "GSPipelineVK.h"

#include <fstream>

void GSPipelineVK::AddShader(vk::UniqueDevice& dev, std::string& name, vk::ShaderStageFlagBits& usage)
{
	std::ifstream shaderFile;
	shaderFile.open(name, std::ios::ate | std::fstream::binary);
	size_t fileSize = (size_t)shaderFile.tellg();
	std::vector<char> contents(fileSize);
	shaderFile.seekg(0);
	shaderFile.read(contents.data(), fileSize);

	vk::ShaderModuleCreateInfo createInfo(
		{},
		fileSize,
		reinterpret_cast<const uint32_t*>(contents.data())
	);

	VKModuleInfo moduleInfo{};
	moduleInfo.module = dev->createShaderModuleUnique(createInfo);
	moduleInfo.name = name;
	moduleInfo.stageType = usage;
	m_modules.push_back(std::move(moduleInfo));
}

void GSPipelineVK::SetVertexAttributes(std::vector<GSInputAttributeVK>& attributes, uint32_t vertexSize)
{
	m_vertex_attributes.clear();
	int i = 0;
	for (const auto& gsattr : attributes)
	{
		vk::VertexInputAttributeDescription attribute(
			0,
			i,
			gsattr.format,
			gsattr.offset
		);
		m_vertex_attributes.push_back(attribute);
	}
	m_vertex_size = vertexSize;
}

void GSPipelineVK::SetDims(vk::Extent2D& extent)
{
	m_viewport.setWidth(extent.width);
	m_viewport.setHeight(extent.height);
	m_scissor.setExtent(extent);
	m_scissor.setOffset({0, 0});
}

void GSPipelineVK::Initialize(vk::UniqueDevice& dev, vk::RenderPass renderPass)
{
	std::vector<vk::PipelineShaderStageCreateInfo> shaderStageInfos;
	for (auto& m : m_modules)
	{
		vk::PipelineShaderStageCreateInfo shaderStageInfo(
			{},
			m.stageType,
			*m.module,
			m.name.c_str()
		);
		shaderStageInfos.push_back(shaderStageInfo);
	}

	vk::PipelineRasterizationStateCreateInfo rsInfo(
		{},
		VK_FALSE,
		VK_FALSE,
		vk::PolygonMode::eFill,
		vk::CullModeFlagBits::eNone,
		vk::FrontFace::eCounterClockwise,
		VK_FALSE,
		0.0f,
		0.0f,
		0.0f,
		1.0f
	);

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly(
		{},
		vk::PrimitiveTopology::eTriangleList,
		VK_FALSE
	);

	vk::PipelineViewportStateCreateInfo viewportInfo(
		{},
		1,
		&m_viewport,
		1,
		&m_scissor
	);

	vk::PipelineMultisampleStateCreateInfo msInfo(
		{},
		vk::SampleCountFlagBits::e1,
		VK_FALSE
	);

	//vk::PipelineDepthStencilStateCreateInfo depthInfo; TODO set depth state

	vk::VertexInputBindingDescription vertexBindingDesc(
		0,
		m_vertex_size,
		vk::VertexInputRate::eVertex
	);
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo(
		{},
		1,
		&vertexBindingDesc,
		static_cast<uint32_t>(m_vertex_attributes.size()),
		m_vertex_attributes.data()
	);

	vk::PipelineColorBlendAttachmentState blendAttachmentState(
		VK_FALSE
	);
	vk::PipelineColorBlendStateCreateInfo blendInfo(
		{},
		VK_FALSE,
		vk::LogicOp::eCopy,
		1,
		&blendAttachmentState
	); // NOTE blending disabled

	vk::PipelineLayoutCreateInfo layoutInfo; // TODO needs to be set at some stage
	m_layout = dev->createPipelineLayoutUnique(layoutInfo);

	vk::GraphicsPipelineCreateInfo pipelineInfo(
		{},
		shaderStageInfos.size(),
		shaderStageInfos.data(),
		&vertexInputInfo,
		&inputAssembly,
		nullptr,
		&viewportInfo,
		&rsInfo,
		&msInfo,
		nullptr,
		&blendInfo,
		nullptr,
		*m_layout,
		renderPass,
		0,  // NOTE assume only one subpass
		nullptr,
		-1
	);
	m_pipeline = dev->createGraphicsPipelineUnique(nullptr, pipelineInfo);
}

void GSPipelineVK::Bind(vk::UniqueCommandBuffer& commandBuffer, vk::PipelineBindPoint& bindPoint)
{
	commandBuffer->bindPipeline(bindPoint, *m_pipeline);
}