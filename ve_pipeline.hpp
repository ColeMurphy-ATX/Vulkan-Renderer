#pragma once

#include "ve_device.hpp"

//std
#include <string>
#include <vector>

namespace ve {

	struct PipelineConfigInfo {
		PipelineConfigInfo() = default;
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class vePipeline {
		public:
			vePipeline(
				veDevice& device,
				const std::string& vertFilepath,
				const std::string& fragFilepath,
				const PipelineConfigInfo& configInfo);
			~vePipeline();

			vePipeline(const vePipeline&) = delete; //deletes copy constructors
			vePipeline& operator=(const vePipeline&) = delete; //to prevent accidentally duplicating pointers to vulkan objects

			static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

			void bind(VkCommandBuffer commandBuffer);

		private:
			static std::vector<char> readFile(const std::string& filepath);

			void createGraphicsPipeline(
				const std::string& vertFilepath,
				const std::string& fragFilepath,
				const PipelineConfigInfo& configInfo);

			void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

			veDevice& veDevice; //potentially memory unsafe bc its a ref and a destructor could try to dereference this dangling pointer, so only used when member variable will implicitly outlive contain class that depends on it
			
			VkPipeline graphicsPipeline; //handle to vulkan pipeline object
			VkShaderModule vertShaderModule;
			VkShaderModule fragShaderModule;
	};
}