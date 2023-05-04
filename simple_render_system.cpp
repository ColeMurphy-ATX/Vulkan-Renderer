#include "simple_render_system.hpp"

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <array>
#include <cassert>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace ve {

	struct SimplePushConstantData {
		glm::mat4 tansform{ 1.0f };
		glm::mat4 modelMatrix{ 1.0f };
	};


	SimpleRenderSystem::SimpleRenderSystem(ve::veDevice& device, VkRenderPass renderPass) : veDevice{ device } {
		createPipelineLayout();
		createPipeline(renderPass);
	}

	SimpleRenderSystem::~SimpleRenderSystem()
	{
		vkDestroyPipelineLayout(veDevice.device(), pipelineLayout, nullptr);

	}

	void SimpleRenderSystem::createPipelineLayout() {

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr; //pipeline setlayout is used to pass data other than vertex buffer to our shaders (like textures and other stuff)
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange; //push constants are a way to efficiently send a very amount of data to shader programs

		if (vkCreatePipelineLayout(veDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout");
		}
	}

	void SimpleRenderSystem::createPipeline(VkRenderPass renderPass) {
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		vePipeline::defaultPipelineConfigInfo(pipelineConfig);
		// a render pass describes the structure and format of our frame buffer objects and their attatchments
		//think of it as a blueprint for telling a graphics pipeline what layout to expect for the output frame buffer
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		vePipeline = std::make_unique<ve::vePipeline>(
			veDevice,
			"D:/Vulkan Renderer/Vulkan Renderer/shader.vert.spv",
			"D:/Vulkan Renderer/Vulkan Renderer/shader.frag.spv",
			pipelineConfig);

	}

	void SimpleRenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<veGameObject>& gameObjects, const veCamera& camera)
	{
		vePipeline->bind(commandBuffer);

		auto projectionView = camera.getProjection() * camera.getView();

		for (auto& obj : gameObjects)
		{

			SimplePushConstantData push{};
			auto modelMatrix = obj.transform.mat4();
			push.tansform = projectionView * modelMatrix;
			push.modelMatrix = modelMatrix;

			vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);

			obj.model->bind(commandBuffer);
			obj.model->draw(commandBuffer);
		}
	}

}