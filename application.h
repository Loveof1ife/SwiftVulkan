#pragma once

#include "base.h"
#include "vulkanWrapper/instance.h"
#include "vulkanWrapper/device.h"
#include "vulkanWrapper/window.h"
#include "vulkanWrapper/windowSurface.h"
#include "vulkanWrapper/swapChain.h"
#include "vulkanWrapper/shader.h"
#include "vulkanWrapper/pipeline.h"
#include "vulkanWrapper/renderPass.h"
#include "vulkanWrapper/commandPool.h"
#include "vulkanWrapper/commandBuffer.h"
#include "vulkanWrapper/semaphore.h"
#include "vulkanWrapper/fence.h"
#include "vulkanWrapper/buffer.h"
#include "vulkanWrapper/image.h"
#include "vulkanWrapper/sampler.h"
#include "texture/texture.h"

#include "model.h"
#include "uniformManager.h"
namespace Tea {

	class Application {
	public:
		Application() = default;

		~Application() = default;

		void run();

	private:
		void initWindow();

		void initVulkan();

		void mainLoop();

		void render();

		void cleanUp();

	private:
		void createPipeline();
		void createRenderPass();
		void createCommandBuffers();
		void createSyncObjects();

		//重建交换链:  当窗口大小发生变化的时候，交换链也要发生变化，Frame View Pipeline RenderPass Sync
		void recreateSwapChain();

		void cleanupSwapChain();

	private:
		unsigned int mWidth{ 800 };
		unsigned int mHeight{ 600 };

	private:
		int mCurrentFrame{ 0 };
		Wrapper::Window::Ptr mWindow{ nullptr };
		Wrapper::Instance::Ptr mInstance{ nullptr };
		Wrapper::Device::Ptr mDevice{ nullptr };
		Wrapper::WindowSurface::Ptr mSurface{ nullptr };
		Wrapper::SwapChain::Ptr mSwapChain{ nullptr };
		Wrapper::Pipeline::Ptr mPipeline{ nullptr };
		Wrapper::RenderPass::Ptr mRenderPass{ nullptr };
		Wrapper::CommandPool::Ptr mCommandPool{ nullptr };

		std::vector<Wrapper::CommandBuffer::Ptr> mCommandBuffers{};

		std::vector<Wrapper::Semaphore::Ptr> mImageAvailableSemaphores{};
		std::vector<Wrapper::Semaphore::Ptr> mRenderFinishedSemaphores{};
		std::vector<Wrapper::Fence::Ptr> mFences{};

		UniformManager::Ptr mUniformManager{ nullptr };

		Model::Ptr mModel{ nullptr };
		VPMatrices	mVPMatrices;
	};
}