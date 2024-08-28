#include "window.h"

namespace Tea::Wrapper {
	Window::Window(const int& width, const int& height) {
		mWidth = width;
		mHeight = height;

		glfwInit();
		//���û������ص�opengl API ���ҽ�ֹ���ڸı��С
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		mWindow = glfwCreateWindow(mWidth, mHeight, "Vulkan", nullptr, nullptr);
		if (!mWindow) {
			std::cerr << "Error: failed to create window" << std::endl;
		}
	}
	Window::~Window() {
		glfwDestroyWindow(mWindow);
		glfwTerminate();
	}

	bool Window::shouldClose() {
		return glfwWindowShouldClose(mWindow);
	}

	void Window::pollEvents() {
		glfwPollEvents();
	}
}