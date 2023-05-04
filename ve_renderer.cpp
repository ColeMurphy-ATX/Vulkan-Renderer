#include "ve_renderer.hpp"
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <array>
#include <cassert>

namespace ve {

	veRenderer::veRenderer(ve::Window& window, ve::veDevice& device) : Window{ window }, veDevice{ device } {
		recreateSwapChain();
		createCommandBuffers();
	}

	veRenderer::~veRenderer()
	{
		freeCommandBuffers();

	}

	void veRenderer::recreateSwapChain() {
		auto extent = Window.getExtent(); //gets current window size
		while (extent.width == 0 || extent.height == 0) {
			extent = Window.getExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(veDevice.device());

		veSwapChain = nullptr; //makes sure old swap chain is destroyed first

		if (veSwapChain == nullptr)
		{
			veSwapChain = std::make_unique<ve::veSwapChain>(veDevice, extent);
		}
		else {
			std::shared_ptr<ve::veSwapChain> oldSwapChain = std::move(veSwapChain);

			veSwapChain = std::make_unique<ve::veSwapChain>(veDevice, extent, oldSwapChain);


			if (!oldSwapChain->compareSwapFormats(*veSwapChain.get())) {
				throw std::runtime_error("Swap chain image(or depth) format has changed!");
			}
		}
	}

	void veRenderer::createCommandBuffers()
	{
		commandBuffers.resize(veSwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; //primary command buffers can be submited to the device graphics queue for execution but cant be called by other command buffers, the reverse is true for secondary command buffers
		allocInfo.commandPool = veDevice.getCommandPool(); //opaque objects that command buffer memory is allocated from
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(veDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failure to allocate command buffers");
		}
	}

	void veRenderer::freeCommandBuffers()
	{
		vkFreeCommandBuffers(veDevice.device(), veDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
		commandBuffers.clear();
	}



	VkCommandBuffer veRenderer::beginFrame() {
		assert(!isFrameStarted && "Can't call beginFrame while already in progress");

		auto result = veSwapChain->acquireNextImage(&currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swap chain image!");
		}

		isFrameStarted = true;

		auto commandBuffer = getCurrentCommandBuffer();
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}
		return commandBuffer;
	}

	void veRenderer::endFrame()
	{
		assert(isFrameStarted && "Can't call endFrame while frame is not in progress");

		auto commandBuffer = getCurrentCommandBuffer();

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}

		auto result = veSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
			Window.wasWindowResized()) {
			Window.resetWindowResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap chain image!");
		}

		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % veSwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	void veRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
		assert(
			commandBuffer == getCurrentCommandBuffer() &&
			"Can't begin render pass on command buffer from a different frame");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = veSwapChain->getRenderPass();
		renderPassInfo.framebuffer = veSwapChain->getFrameBuffer(currentImageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = veSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(veSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(veSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, veSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void veRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
		assert(
			commandBuffer == getCurrentCommandBuffer() &&
			"Can't end render pass on command buffer from a different frame");

		vkCmdEndRenderPass(commandBuffer);
	}
}