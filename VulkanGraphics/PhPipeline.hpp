#pragma once

#include "PhDevice.hpp"

// std lib headers
#include <string>
#include <vector>

namespace phantom {

	struct PipelineConfigInfo{
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

	class PhPipeline {
	public:
		PhPipeline(PhDevice& device, const std::string& vertFilePath, const std::string& fragFilePath, const PipelineConfigInfo& configInfo);
		~PhPipeline();

		PhPipeline(const PhPipeline&) = delete;
		PhPipeline &operator=(const PhPipeline&) = delete;

		void bind(VkCommandBuffer commandBuffer);

		static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

	private:
		static std::vector<char> readFile(const std::string& filepath);

		void createGraphicsPipeline(const std::string& vertFilePath, const std::string& fragFilePath, const PipelineConfigInfo& configInfo);

		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

		PhDevice& phDevice;
		VkPipeline graphicsPipeline;
		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;
	};
}