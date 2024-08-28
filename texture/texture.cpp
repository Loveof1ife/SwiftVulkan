#include "texture.h"
namespace Tea {

	Texture::Texture(
		const Wrapper::Device::Ptr& device, 
		const Wrapper::CommandPool::Ptr& commandPool, 
		const std::string& imageFilePath) 
	{
		mDevice = device;

		int texWidth, texHeight, texSize, texChannles;
		stbi_uc* pixels = stbi_load(imageFilePath.c_str(), &texWidth, &texHeight, &texChannles, STBI_rgb_alpha);
		

		if (!pixels) {
			throw std::runtime_error("Error: failed to read image data");
		}

		//The pixels are laid out row by row with 4 bytes per pixel in the case of STBI_rgb_alpha for a total of texWidth * texHeight * 4 values
		texSize = texWidth * texHeight * 4;

		mImage = Wrapper::Image::create(
			mDevice, texWidth, texHeight,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_TYPE_2D,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_SAMPLE_COUNT_1_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT
		);

		VkImageSubresourceRange region{};

		region.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		region.baseArrayLayer = 0;
		region.layerCount = 1;

		region.baseMipLevel = 0;
		region.levelCount = 1;
		//record and execute "vkCmdCopyBufferToImage" to finish the job,
		// but this command requires the image to be in the right layout first. 
		//Create a new function to handle layout transitions:
		mImage->setImageLayout(
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			region,
			commandPool
		);

		// We'll start by creating a staging resource 
		// and filling it with pixel data 
		// and then we copy this to the final image object that we'll use for rendering. 
		mImage->fillImageData(texSize, (void*)pixels, commandPool);

		stbi_image_free(pixels);

		mSampler = Wrapper::Sampler::create(mDevice);

		mImageInfo.imageLayout = mImage->getLayout();
		mImageInfo.imageView = mImage->getImageView();
		mImageInfo.sampler = mSampler->getSampler();
	}

	Texture::~Texture() {}
}