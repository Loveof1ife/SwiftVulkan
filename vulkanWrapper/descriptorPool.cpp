#include"descriptorPool.h"

namespace Tea::Wrapper {
	DescriptorPool::DescriptorPool(const Device::Ptr& device) {
		mDevice = device;
	}
	
	DescriptorPool::~DescriptorPool() {
		if (mPool != VK_NULL_HANDLE) {
			vkDestroyDescriptorPool(mDevice->getDevice(), mPool, nullptr);
		}
	}

	void DescriptorPool::build(std::vector<UniformParameter::Ptr>& params, const int& frameCount) {
		int uniformBufferCount{ 0 };
		int textureCount{ 0 };

		for (const auto& param : params) {
			if(param->mDescriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
				uniformBufferCount++;

			if (param->mDescriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) 
				textureCount++; 
		}

		//描述每一种uniform都有多少个
		std::vector<VkDescriptorPoolSize> poolSizes{};

		VkDescriptorPoolSize uniformBufferSize{};
		uniformBufferSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformBufferSize.descriptorCount = uniformBufferCount * frameCount;
		poolSizes.push_back(uniformBufferSize);

		VkDescriptorPoolSize textureSize{};
		textureSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureSize.descriptorCount = textureCount * frameCount;//这边的size是指，有多少个descriptor
		poolSizes.push_back(textureSize);

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size() );
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = static_cast<uint32_t>(frameCount);

		if (vkCreateDescriptorPool(mDevice->getDevice(), &poolInfo, nullptr, &mPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}
}

