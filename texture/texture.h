#pragma once

#include "../base.h"
#include "../vulkanWrapper/image.h"
#include "../vulkanWrapper/sampler.h"
#include "../vulkanWrapper/device.h"
#include "../vulkanWrapper/commandPool.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"

namespace Tea {
	//start by creating a staging resource 
	//fill it with pixel data and copy stagomg to the final image object that we'll use for rendering.
	//staging image or buffer,  Vulkan also allows you to copy pixels from a VkBuffer to an image and the API for this is actually faster on some hardware
	//Although we could set up the shader to access the pixel values in the buffer
	//it's better to use image objects in Vulkan for this purpose. Image objects will make it easier and faster to retrieve colors by allowing us to use 2D coordinates, for one. 
	class Texture {
	public:
		using Ptr = std::shared_ptr<Texture>;
		static Ptr create(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr& commandPool, const std::string& imageFilePath) {
			return std::make_shared<Texture>(device, commandPool, imageFilePath);
		}

		Texture(const Wrapper::Device::Ptr& device, const Wrapper::CommandPool::Ptr& commandPool, const std::string& imageFilePath);

		~Texture();

		[[nodiscard]] auto getImageInfo() { return mImageInfo; }
	private:
		Wrapper::Device::Ptr mDevice{ nullptr };
		Wrapper::Image::Ptr mImage{ nullptr };
		Wrapper::Sampler::Ptr mSampler{ nullptr };

		VkDescriptorImageInfo mImageInfo{};
	};
}