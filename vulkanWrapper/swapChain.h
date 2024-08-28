#pragma once
#include "../base.h"
#include "device.h"
#include "window.h"
#include "windowSurface.h"
#include "image.h"
#include "renderPass.h"

namespace Tea::Wrapper {

	struct SwapChainSupportInfo {
		VkSurfaceCapabilitiesKHR mCapabilities;
		std::vector<VkSurfaceFormatKHR> mFormats;
		std::vector<VkPresentModeKHR> mPresentModes;
	};

	class SwapChain {
	public:
		using Ptr = std::shared_ptr<SwapChain>;
		static Ptr create(const Device::Ptr& device, const Window::Ptr& window, const WindowSurface::Ptr& surface, const CommandPool::Ptr& commandPool) {
			return std::make_shared<SwapChain>(device, window, surface, commandPool);
		}

		SwapChain(
			const Device::Ptr& device, 
			const Window::Ptr& window, 
			const WindowSurface::Ptr& surface,
			const CommandPool::Ptr& commandPool
		);

		~SwapChain();

		SwapChainSupportInfo querySwapChainSupportInfo();

		VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

		static VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

		static VkPresentModeKHR chooseSurfacePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

		VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		void createFrameBuffers(const RenderPass::Ptr& renderPass);

		[[nodiscard]] auto getFormat() const { return mSwapChainFormat; }

		[[nodiscard]] auto getImageCount() const { return mImageCount; }

		[[nodiscard]] auto getSwapChain() const { return mSwapChain; }

		[[nodiscard]] auto getFrameBuffer(const int index) const { return mSwapChainFrameBuffers[index]; }

		[[nodiscard]] auto getExtent() const { return mSwapChainExtent; }

	private:
		//A swapchain is a series of images (or buffers) that are used for rendering and displaying to the screen
		//The images provided by a swapchain are often used as the color attachments in framebuffers.
		VkSwapchainKHR mSwapChain{ VK_NULL_HANDLE };

		VkFormat mSwapChainFormat;
		VkExtent2D mSwapChainExtent;
		uint32_t mImageCount{ 0 };

		//vkimage由swapchain创建，销毁也要交给swapchain
		std::vector<VkImage> mSwapChainImages;

		//对图像的管理器。管理框架
		std::vector<VkImageView> mSwapChainImageViews{};

		//A framebuffer is an abstraction representing a set of memory attachments that can be used as the destination for rendering commands. These attachments typically include color buffers, depth buffers, and stencil buffers. 
		//In Vulkan, a framebuffer is associated with a render pass and a set of attachments, which might be images from a swapchain or other render targets.
		std::vector<VkFramebuffer> mSwapChainFrameBuffers{};

		//depth
		std::vector<Image::Ptr> mDepthImages{};

		Device::Ptr mDevice{ nullptr };
		Window::Ptr mWindow{ nullptr };
		WindowSurface::Ptr mSurface{ nullptr };
	};

}