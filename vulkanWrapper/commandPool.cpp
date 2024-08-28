#include "commandPool.h"

namespace Tea::Wrapper {
	
	CommandPool::CommandPool(const Device::Ptr& device, VkCommandPoolCreateFlagBits flag) {
		mDevice = device;

		VkCommandPoolCreateInfo poolCreateInfo{};
		poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolCreateInfo.queueFamilyIndex = device->getGraphicQueueFamily().value();
		poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(mDevice->getDevice(), &poolCreateInfo, nullptr, &mCommandPool) != VK_SUCCESS) {
			throw std::runtime_error("Error:  failed to create command pool");
		}
	}

	CommandPool::~CommandPool() {
		if (mCommandPool != VK_NULL_HANDLE) {
			vkDestroyCommandPool(mDevice->getDevice(), mCommandPool, nullptr);
		}
	}

}