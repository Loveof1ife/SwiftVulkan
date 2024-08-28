#pragma once

#include "../base.h"
#include "device.h"
#include "description.h"

namespace Tea::Wrapper {
	//Descriptor sets can't be created directly, they must be allocated from a pool like command buffers
	class DescriptorPool{
	public:
		using Ptr = std::shared_ptr<DescriptorPool>;
		static Ptr create(const Device::Ptr& device) { 
			return std::make_shared<DescriptorPool>(device); 
		}

		DescriptorPool(const Device::Ptr& device);

		void build(std::vector<UniformParameter::Ptr>& params, const int& frameCount);

		~DescriptorPool();

		[[nodiscard]] auto getPool() const { return mPool; }

	private:
		VkDescriptorPool mPool{ VK_NULL_HANDLE };

		Device::Ptr mDevice{ nullptr };
	};

}	