#include "swapChain.h"

namespace Tea::Wrapper {

	SwapChain::SwapChain(
		const Device::Ptr& device, 
		const Window::Ptr& window, 
		const WindowSurface::Ptr& surface,
		const CommandPool::Ptr& commandPool
	) {
		mDevice = device;
		mWindow = window;
		mSurface = surface;

		SwapChainSupportInfo swapChainSupportInfo = querySwapChainSupportInfo();

		//选择vkformat
		VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(swapChainSupportInfo.mFormats);

		//选择prsentMode
		VkPresentModeKHR presentMode = chooseSurfacePresentMode(swapChainSupportInfo.mPresentModes);

		//选择交换链范围
		VkExtent2D extent = chooseExtent(swapChainSupportInfo.mCapabilities);

		mImageCount = swapChainSupportInfo.mCapabilities.minImageCount + 1;

		if (swapChainSupportInfo.mCapabilities.maxImageCount > 0 && mImageCount > swapChainSupportInfo.mCapabilities.maxImageCount) {
			mImageCount = swapChainSupportInfo.mCapabilities.maxImageCount;
		}

		//填写创建信息, 此处初始化必须置空，因为会有忘记设置的变量，值为随机
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = mSurface->getSurface();
		createInfo.minImageCount = mImageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;

		//图像包含的层次，VR一般会有两个
		createInfo.imageArrayLayers = 1;	
		// 交换链生成的图像，到底用于何处
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		//因为交换链的图像，会被用来渲染或者显示，而渲染跟显示分别使用不同的队列，所以会出现两个队列使用同一个交换链的情况
		//那么我们就需要设置，让交换链的图像，被两个队列使用兼容
		std::vector<uint32_t> queueFamilies = { mDevice->getGraphicQueueFamily().value() , mDevice->getPresentQueueFamily().value() };
		if (mDevice->getGraphicQueueFamily().value() == mDevice->getPresentQueueFamily().value()) {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;//被某一个队列族独占，性能会更好
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}
		else {
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;//可以被共享的模式
			createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilies.size());
			createInfo.pQueueFamilyIndices = queueFamilies.data();
		}

		//交换链的图像初始变化，比如是否需要反转
		createInfo.preTransform = swapChainSupportInfo.mCapabilities.currentTransform;

		//不与原来窗体当中的内容混合
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		createInfo.presentMode = presentMode;

		//当前窗体被挡住的部分，不用绘制,但是会影响到回读！
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = VK_NULL_HANDLE;
		if (vkCreateSwapchainKHR(mDevice->getDevice(), &createInfo, nullptr, &mSwapChain) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain!");
		}

		mSwapChainFormat = surfaceFormat.format;
		mSwapChainExtent = extent;
		//The images were created by the implementation for the swap chain 
		//retrieving the handles of the VkImages in swap chain
		//Retrieving them is very similar to the other times where we retrieved an array of objects from Vulkan
		vkGetSwapchainImagesKHR(mDevice->getDevice(), mSwapChain, &mImageCount, nullptr);
		mSwapChainImages.resize(mImageCount);
		vkGetSwapchainImagesKHR(mDevice->getDevice(), mSwapChain, &mImageCount, mSwapChainImages.data());

		//创建imageView
		mSwapChainImageViews.resize(mImageCount);
		for (int i = 0; i < mImageCount; ++i) {
			mSwapChainImageViews[i] = createImageView(mSwapChainImages[i], mSwapChainFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
		}

		mDepthImages.resize(mImageCount);

		VkImageSubresourceRange region{};
		region.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		region.baseMipLevel = 0;
		region.levelCount = 1;
		region.baseArrayLayer = 0;
		region.layerCount = 1;

		for (int i = 0; i < mImageCount; i++) {
			mDepthImages[i] = Image::createDepthImage(mDevice, mSwapChainExtent.width, mSwapChainExtent.height);

			mDepthImages[i]->setImageLayout(
				VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
				region,
				commandPool
			);
		}
	}

	SwapChain::~SwapChain()	{
				for (auto& imageView : mSwapChainImageViews) {
			vkDestroyImageView(mDevice->getDevice(), imageView, nullptr);
		}

		for (auto& frameBuffer : mSwapChainFrameBuffers) {
			vkDestroyFramebuffer(mDevice->getDevice(), frameBuffer, nullptr);
		}

		if (mSwapChain != VK_NULL_HANDLE) {
			vkDestroySwapchainKHR(mDevice->getDevice(), mSwapChain, nullptr);
		}
		
		mWindow.reset();
		mSurface.reset();
		mDevice.reset();
	}


	SwapChainSupportInfo SwapChain::querySwapChainSupportInfo() {
		SwapChainSupportInfo info;
		//获取基础特性
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &info.mCapabilities);
		//获取表面支持格式
		/*A surface may support multiple formats because different applications or devices may have varying requirements for color accuracy, performance, and compatibility*/
		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &formatCount, nullptr);

		if (formatCount != 0) {
			info.mFormats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &formatCount, info.mFormats.data());
		}

		//获取呈现模式
		/*Vulkan supports multiple present modes to provide flexibility in terms of performance, latency, and smoothness.*/
		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			info.mPresentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(mDevice->getPhysicalDevice(), mSurface->getSurface(), &presentModeCount, info.mPresentModes.data());
		}

		return info;
	}

	VkImageView SwapChain::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = image;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = format;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		//createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.aspectMask = aspectFlags;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = mipLevels;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VkImageView imageView{ VK_NULL_HANDLE };
		if (vkCreateImageView(mDevice->getDevice(), &createInfo, nullptr, &imageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create image views!");
		}
		return imageView;
	}

	VkSurfaceFormatKHR SwapChain::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		//如果只返回了一个未定义的格式，那么就没有首选格式，我们自己做一个
		if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) {
			return { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR SwapChain::chooseSurfacePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		//在设备上，只有FIFO是被绝对支持的, 如果在移动设备上，为了节省电源，优先选择FIFO
		VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}
		return bestMode;
	}

	VkExtent2D SwapChain::chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{	// The swap extent is the resolution of the swap chain image and it's almost always exactly equal to the resolution of the window 
		// The range of the possible resolutions is defined in the VkSurfaceCapabilitiesKHR structure.
		// Vulkan tells us to match the resolution of the window by setting the width and height in the" currentExtent "member.
		// However, some window managers do allow us to differ here and this is indicated by setting the width and height in currentExtent to a special value : the maximum value of uint32_t.*/
		// 
		//如果出现以下情况，说明系统不允许我们自己设定extent
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}
		else {
			//由于高清屏幕情况下 ，比如苹果， 窗体的坐标大小，并不等于像素的长宽
			int width = 0, height = 0;
			glfwGetFramebufferSize(mWindow->getWindow(), &width, &height);
			VkExtent2D actualExtent = { static_cast<uint32_t>(width),
									   static_cast<uint32_t>(height) };
			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
			//clamp到max与min之间
	/*		actualExtent.width = std::max(capabilities.minImageExtent.width,
				std::min(capabilities.maxImageExtent.width, actualExtent.width));

			actualExtent.height = std::max(capabilities.minImageExtent.height,
				std::min(capabilities.maxImageExtent.height, actualExtent.height));*/
			return actualExtent;

		}
	}

	void SwapChain::createFrameBuffers(const RenderPass::Ptr& renderPass) {
		//创建FrameBuffer
		mSwapChainFrameBuffers.resize(mImageCount);
		for (int i = 0; i < mImageCount; ++i) {
			//FrameBuffer 里面为一帧的数据，比如有n个ColorAttachment 1个DepthStencilAttachment，
			//这些东西的集合为一个FrameBuffer，送入管线，就会形成一个GPU的集合，由上方的Attachments构成
			std::array<VkImageView, 2> attachments { mSwapChainImageViews[i], mDepthImages[i]->getImageView() };

			VkFramebufferCreateInfo frameBufferCreateInfo{};
			frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frameBufferCreateInfo.renderPass = renderPass->getRenderPass();
			frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			frameBufferCreateInfo.pAttachments = attachments.data();
			frameBufferCreateInfo.width = mSwapChainExtent.width;
			frameBufferCreateInfo.height = mSwapChainExtent.height;
			frameBufferCreateInfo.layers = 1;

			if (vkCreateFramebuffer(mDevice->getDevice(), &frameBufferCreateInfo, nullptr, &mSwapChainFrameBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("Error:Failed to create frameBuffer");
			}
		}
	}
	

}
