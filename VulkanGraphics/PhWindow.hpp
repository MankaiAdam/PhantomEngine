#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

// std lib headers
#include <string>

namespace phantom {

	class PhWindow {
	public:
		PhWindow(int w, int h, std::string name);
		~PhWindow();

		PhWindow(const PhWindow&) = delete;
		PhWindow &operator=(const PhWindow&) = delete;

		bool shouldClose() { return glfwWindowShouldClose(window); }
		VkExtent2D getExtent() { return{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }
		bool wasWindowResized() { return framebufferResized; }
		void resetWindowResizedFlag() { framebufferResized = false; }

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	private:
		int width;
		int height;
		bool framebufferResized = false;

		std::string windowName;
		GLFWwindow* window;

		void initWindow();
		static void framebufferResizeCallback(GLFWwindow *window, int width, int height);
	};

}