#pragma once
#include "PhWindow.hpp"
#include "PhPipeline.hpp"
#include "PhDevice.hpp"
#include "PhSwapChain.hpp"
#include "Model.hpp"
#include "PhGameObject.hpp"

#include <memory>
#include <vector>

namespace phantom {

	class app {

	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		app();
		~app();
		app(const app&) = delete;
		app &operator=(const app&) = delete;
		void run();

	private:
		void loadGameObjects();
		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();
		void freeCommandBuffers();
		void drawFrame();
		void recreateSwapChain();
		void recordCommandBuffer(int imageIndex);
		void renderGameObjects(VkCommandBuffer commandBuffer);

		PhWindow phWindow{WIDTH, HEIGHT, "Hello Vulkan!!"};
		PhDevice phDevice{ phWindow };
		std::unique_ptr<PhSwapChain> phSwapChain;	
		std::unique_ptr<PhPipeline> phPipeline;
		VkPipelineLayout pipelineLayout;
		std::vector<VkCommandBuffer> commandBuffers;
		std::vector<PhGameObject> gameObjects;
	};

}	