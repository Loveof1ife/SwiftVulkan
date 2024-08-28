#include"windowSurface.h"

namespace Tea::Wrapper {
	WindowSurface::WindowSurface(Instance::Ptr instance, Window::Ptr window){
		mInstance = instance;
		if (glfwCreateWindowSurface(mInstance->getInstance(), window->getWindow(), nullptr, &mSurface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface!");
		}
	}

	WindowSurface::~WindowSurface() {
		vkDestroySurfaceKHR(mInstance->getInstance(), mSurface, nullptr);
		mInstance.reset();
	}
}