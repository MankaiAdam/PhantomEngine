#include <stdexcept>
#include "PhWindow.hpp"

namespace phantom {

	PhWindow::PhWindow(int w, int h, std::string name):width(w), height(h), windowName(name) {
		initWindow();
	}

	PhWindow::~PhWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void PhWindow::initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	void PhWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface) {
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}

	void PhWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto phWindow = reinterpret_cast<PhWindow*>(glfwGetWindowUserPointer(window));
		phWindow->framebufferResized = true;
		phWindow->width = width;
		phWindow->height = height;
	}

}