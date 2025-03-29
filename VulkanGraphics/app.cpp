
#include "app.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <stdexcept>

namespace phantom {

	struct SimplePushConstantData {
		glm::mat2 transform{1.0f};
		glm::vec2 offset;
		alignas(16) glm::vec3 color;
	};

	app::app(){
		loadGameObjects();
		createPipelineLayout();
		recreateSwapChain();
		createCommandBuffers();
	}

	app::~app() {
		vkDestroyPipelineLayout(phDevice.device(), pipelineLayout, nullptr);
	}

	void app::run() {
		while (!phWindow.shouldClose()) {
			glfwPollEvents();
			drawFrame();
		}

		vkDeviceWaitIdle(phDevice.device());
	}

	void app::loadGameObjects() {
		std::vector<Model::Vertex> vertices{
			{{+0.0f,-0.5f},{0.0f,1.0f,0.0f}},
			{{+0.5f,+0.5f},{1.0f,0.0f,0.0f}},
			{{-0.5f,+0.5f},{0.0f,0.0f,1.0f}}
		};

		auto model = std::make_shared<Model>(phDevice, vertices);

		auto triangle = PhGameObject::createGameObject();
		triangle.model = model;
		triangle.color = { 0.1f, 0.8f, 0.1f };
		triangle.transform2d.translation.x = 0.2f;
		triangle.transform2d.scale = { 2.0f, 0.5f };
		triangle.transform2d.rotation = 0.25f * glm::two_pi<float>();
		gameObjects.push_back(std::move(triangle));

	}

	void app::createPipelineLayout() {

		VkPushConstantRange pushConstantRange;
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(phDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout");
		}
	}

	void app::createPipeline() {
		assert(phSwapChain != nullptr && "Cannot create pipeline before swap chain");
		assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig = {};
		PhPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = phSwapChain->getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		phPipeline = std::make_unique<PhPipeline>(
			phDevice,
			"shader.vert.spv",
			"shader.frag.spv",
			pipelineConfig
		);
	}

	void app::recreateSwapChain() {
		auto extent = phWindow.getExtent();
		while (extent.width == 0 || extent.height == 0) {
			extent = phWindow.getExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(phDevice.device());
		if (phSwapChain == nullptr) {
			phSwapChain = std::make_unique<PhSwapChain>(phDevice, extent);
		}
		else {
			phSwapChain = std::make_unique<PhSwapChain>(phDevice, extent, std::move(phSwapChain));
			if (phSwapChain->imageCount() != commandBuffers.size()) {
				freeCommandBuffers();
				createCommandBuffers();
			}
		}
		createPipeline();
	}

	void app::createCommandBuffers() {
		commandBuffers.resize(phSwapChain->imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = phDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(phDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers");
		}
	}

	void app::freeCommandBuffers() {
		vkFreeCommandBuffers(
			phDevice.device(),
			phDevice.getCommandPool(),
			static_cast<uint32_t>(commandBuffers.size()),
			commandBuffers.data());
		commandBuffers.clear();
	}


	void app::recordCommandBuffer(int imageIndex) {

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = phSwapChain->getRenderPass();
		renderPassInfo.framebuffer = phSwapChain->getFrameBuffer(imageIndex);

		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = phSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f,0.01f,0.01f,0.1f };
		clearValues[1].depthStencil = { 1.0f,0 };

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(phSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(phSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, phSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
		vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

		renderGameObjects(commandBuffers[imageIndex]);

		vkCmdEndRenderPass(commandBuffers[imageIndex]);
		if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}

	void app::renderGameObjects(VkCommandBuffer commandBuffer) {
		phPipeline->bind(commandBuffer);

		for (auto& obj : gameObjects) {
			SimplePushConstantData push{};
			push.offset = obj.transform2d.translation;
			push.color = obj.color;
			push.transform = obj.transform2d.mat2();
			vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
			obj.model->bind(commandBuffer);
			obj.model->draw(commandBuffer);
		}
	}

	void app::drawFrame() {
		uint32_t imageIndex;
		auto result = phSwapChain->acquireNextImage(&imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire next swapchain image");
		}

		recordCommandBuffer(imageIndex);
		result = phSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || phWindow.wasWindowResized()) {
			phWindow.resetWindowResizedFlag();
			recreateSwapChain();
			return;
		}
		if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swap cahin image!");
		}
	}
}