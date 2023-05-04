#pragma once

#include "window.hpp"
#include "ve_device.hpp"
#include "ve_swap_chain.hpp"


#include <memory>
#include <vector>
#include <cassert>

namespace ve {
	class veRenderer {
	public:
		veRenderer(Window& window, veDevice& device);
		~veRenderer();

		veRenderer(const veRenderer&) = delete;
		veRenderer& operator=(const veRenderer&) = delete;

		VkCommandBuffer beginFrame();
		void endFrame();
		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

		VkRenderPass getSwapChainRenderPass() const { return veSwapChain->getRenderPass(); }

		float getAspectRatio() const { return veSwapChain->extentAspectRatio(); }

		bool isFrameInProgress() const { return isFrameStarted; }

		VkCommandBuffer getCurrentCommandBuffer() const
		{
			assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
			return commandBuffers[currentFrameIndex];
		}

		int getFrameIndex() const {
			assert(isFrameStarted && "Cannot get frame index when frame not in progress");
			return currentFrameIndex;
		}

	private:

		void createCommandBuffers();

		void freeCommandBuffers();

		void recreateSwapChain();


		Window& Window;

		veDevice& veDevice;

		std::unique_ptr<veSwapChain> veSwapChain;

		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex;
		bool isFrameStarted = false;

	};
}