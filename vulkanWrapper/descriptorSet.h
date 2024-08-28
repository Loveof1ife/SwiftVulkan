#pragma once

#include "../base.h"
#include "device.h"
#include "description.h"
#include "descriptorSetLayout.h"
#include "descriptorPool.h"

namespace Tea::Wrapper{
	class DescriptorSet {
	public:
		using Ptr = std::shared_ptr<DescriptorSet>;
		static Ptr create(
			const Device::Ptr& device,
			const std::vector<UniformParameter::Ptr>& params,
			const DescriptorPool::Ptr& pool,
			const DescriptorSetLayout::Ptr& layout,
			int frameCount
		) {
			return std::make_shared<DescriptorSet>(
				device,
				params,
				pool,
				layout,
				frameCount
			);
		}

		DescriptorSet(
			const Device::Ptr& device,
			const std::vector<UniformParameter::Ptr>& params,
			const DescriptorPool::Ptr& pool,
			const DescriptorSetLayout::Ptr& layout,
			int frameCount
		);

		~DescriptorSet();

		[[nodiscard]] auto getDescriptorSet(int frameCount) const { return mDescriptorSets[frameCount]; }

	private:
		std::vector<VkDescriptorSet> mDescriptorSets{};
		Device::Ptr mDevice{ nullptr };
	};
}

