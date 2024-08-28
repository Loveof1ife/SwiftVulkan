#pragma once

#include "../base.h"
#include "device.h"

namespace Tea::Wrapper {
	//def::Commands:in Vulkan are not executed directly using function calls, instead, you have to record all of the operations in command buffer objs
    //advantages: The advantage of this is that when we are ready to tell the Vulkan what we want to do, all of the commands are submitted together and Vulkan can more efficiently process the commands since all of them are available together.
    //further, this allows command recording to happen in multiple threads if so desired.
	class CommandPool {
	public:
		using Ptr = std::shared_ptr<CommandPool>;
		static Ptr create(const Device::Ptr& device, VkCommandPoolCreateFlagBits flag = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) {
			return std::make_shared<CommandPool>(device, flag);
		}

		CommandPool(const Device::Ptr& device, VkCommandPoolCreateFlagBits flag = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

		~CommandPool();

		[[nodiscard]] auto getCommandPool() const { return mCommandPool; }

	private:
		VkCommandPool mCommandPool{VK_NULL_HANDLE};
		Device::Ptr mDevice;
	};
}