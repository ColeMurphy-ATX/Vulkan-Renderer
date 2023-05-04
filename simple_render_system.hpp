#pragma once

#include "ve_pipeline.hpp"
#include "ve_device.hpp"
#include "ve_game_object.hpp"
#include "ve_camera.hpp"

#include <memory>
#include <vector>

namespace ve {
	class SimpleRenderSystem {
	public:

		SimpleRenderSystem(veDevice& device, VkRenderPass renderPass);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem& operator=(const SimpleRenderSystem&) = delete;

		void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<veGameObject>& gameObjects, const veCamera& camera);

	private:

		void loadGameObjects();

		void createPipelineLayout();

		void createPipeline(VkRenderPass renderPass);

		veDevice& veDevice;

		std::unique_ptr<vePipeline> vePipeline; //smart pointer, simulates pointer with automatic memory management

		VkPipelineLayout pipelineLayout;
	};
}