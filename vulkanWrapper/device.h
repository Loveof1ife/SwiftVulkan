#pragma once

#include "../base.h"
#include "instance.h"
#include "windowSurface.h"

namespace Tea::Wrapper {
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	class Device {
	public:
		using Ptr = std::shared_ptr<Device>;
		static Ptr create(Instance::Ptr instance, WindowSurface::Ptr surface) {
			return std::make_shared<Device>(instance, surface);
		}

		Device(Instance::Ptr instance, WindowSurface::Ptr surface);

		~Device();

		void pickPhysicalDevice();

		int rateDevice(VkPhysicalDevice device);

		bool isDeviceSuitable(VkPhysicalDevice device);

		void initQueueFamilies(VkPhysicalDevice device);

		void createLogicalDevice();


		[[nodiscard]] auto getDevice() const { return mDevice; }
		[[nodiscard]] auto getPhysicalDevice() const { return mPhysicalDevice; }

		[[nodiscard]] auto getGraphicQueueFamily() const { return mQueueFamilyIndices.graphicsFamily; }
		[[nodiscard]] auto getPresentQueueFamily() const { return mQueueFamilyIndices.presentFamily; }

		[[nodiscard]] auto getGraphicQueue() const { return mGraphicQueue; }
		[[nodiscard]] auto getPresentQueue() const { return mPresentQueue; }

	private:
		VkPhysicalDevice mPhysicalDevice{ VK_NULL_HANDLE };

		VkDevice mDevice{ VK_NULL_HANDLE };

		//存储当前渲染任务队列族的id
		//std::optional<uint32_t> mGraphicQueueFamily;
		//std::optional<uint32_t> mPresentQueueFamily;
		QueueFamilyIndices mQueueFamilyIndices;

		VkQueue	mGraphicQueue{ VK_NULL_HANDLE };
		VkQueue mPresentQueue{ VK_NULL_HANDLE };

		Instance::Ptr mInstance{ nullptr };
		WindowSurface::Ptr mSurface{ nullptr };
	};
}
