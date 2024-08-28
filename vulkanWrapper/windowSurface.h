#pragma once

#include "../base.h"
#include "instance.h"
#include "window.h"

namespace Tea::Wrapper {
	class WindowSurface {
	public:
		using Ptr = std::shared_ptr<WindowSurface>;
		static Ptr create(Instance::Ptr instance, Window::Ptr window) {
			return std::make_shared<WindowSurface>(instance, window);
		}

		WindowSurface(Instance::Ptr instance, Window::Ptr window);

		~WindowSurface();

		[[nodiscard]] auto getSurface() const { return mSurface; }

	private:
		/*	VkSurfaceKHR handle refers to Vulkan¡¯s view of a window
	represents the connection between Vulkan(your application) and the windowing system*/
		VkSurfaceKHR mSurface;
		Instance::Ptr mInstance{ nullptr };
	};
}