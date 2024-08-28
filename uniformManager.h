#pragma once

#include "vulkanWrapper/buffer.h"
#include "vulkanWrapper/descriptorSetLayout.h"
#include "vulkanWrapper/descriptorPool.h"
#include "vulkanWrapper/descriptorSet.h"
#include "vulkanWrapper/description.h"
#include "base.h"

//Usage of descriptors consists of three parts:
//1.Specify a descriptor layout during pipeline creation
//2.Allocate a descriptor set from a descriptor pool
//3.Bind the descriptor set during rendering

namespace Tea {

	class UniformManager {
	public:
		using Ptr = std::shared_ptr<UniformManager>;
		static Ptr create() { return std::make_shared<UniformManager>(); }

		UniformManager();

		~UniformManager();

		void init(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr& commandPool, int frameCount);

		void update(const VPMatrices& vpMatrices, const ObjectUniform& objectUniform, const int& frameCount);

		[[nodiscard]] auto getDescriptorLayout() const { return mDescriptorSetLayout->getLayout(); }

		[[nodiscard]] auto getDescriptorSet(int frameCount) const { return mDescriptorSet->getDescriptorSet(frameCount); }

	private:
		Wrapper::Device::Ptr mDevice{ nullptr };

		std::vector<Wrapper::UniformParameter::Ptr> mUniformParams{};

		//descriptor layout describes the type of descriptors that can be bound. 
		// for each VkBuffer, bind it to the uniform buffer descriptor.
		Wrapper::DescriptorSetLayout::Ptr mDescriptorSetLayout{ nullptr };
		Wrapper::DescriptorPool::Ptr	mDescriptorPool{ nullptr };
		Wrapper::DescriptorSet::Ptr		mDescriptorSet{ nullptr };

	};
}