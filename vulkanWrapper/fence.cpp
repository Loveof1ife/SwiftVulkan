#include"fence.h"

namespace Tea::Wrapper {
	/*Fences in Vulkan are used for synchronization between the CPU and GPU.
	ensure that the CPU waits for the GPU to complete certain operations.*/

	/*Command Buffer Submission : Ensuring that a submitted command buffer has finished executing.
	Frame Rendering : Synchronizing frame rendering, ensuring that the CPU does not start processing the next frame until the current frame has finished rendering.*/

	Fence::Fence(const Device::Ptr& device, bool signaled) {
		mDevice = device;

		VkFenceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		createInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;

		if (vkCreateFence(mDevice->getDevice(), &createInfo, nullptr, &mFence) != VK_SUCCESS) {
			throw std::runtime_error("Error:failed to create fence");
		}
	}

	Fence::~Fence() {
		if (mFence != VK_NULL_HANDLE) {
			vkDestroyFence(mDevice->getDevice(), mFence, nullptr);
		}
	}

		//置为非激发态
	void Fence::resetFence() {
		vkResetFences(mDevice->getDevice(), 1, &mFence);
	}

		//调用此函数，如果fence没有被激发，那么阻塞在这里，等待激发
	void Fence::block(uint64_t timeout) {
		vkWaitForFences(mDevice->getDevice(), 1, &mFence, VK_TRUE, timeout);
	}


}