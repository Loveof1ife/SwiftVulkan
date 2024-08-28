#include "application.h"

namespace Tea {

	void Application::run(){
		initWindow();
		initVulkan();
		mainLoop();
		cleanUp();
	}

	void Application::initWindow(){
		mWindow = Wrapper::Window::create(WIDTH, HEIGHT);
	}

	void Application::initVulkan(){
		mInstance = Wrapper::Instance::create(true);

		mSurface = Wrapper::WindowSurface::create(mInstance, mWindow);

		mDevice = Wrapper::Device::create(mInstance, mSurface);

		mCommandPool = Wrapper::CommandPool::create(mDevice);

		mSwapChain = Wrapper::SwapChain::create(mDevice, mWindow, mSurface);
		mWidth = mSwapChain->getExtent().width;
		mHeight = mSwapChain->getExtent().height;

		mRenderPass = Wrapper::RenderPass::create(mDevice);
		createRenderPass();

		mSwapChain->createFrameBuffers(mRenderPass);

		//descriptor ===========================
		mUniformManager = UniformManager::create();
		mUniformManager->init(mDevice, mCommandPool, mSwapChain->getImageCount());

		mModel = Model::create(mDevice);

		mPipeline = Wrapper::Pipeline::create(mDevice, mRenderPass);
		createPipeline();

		mCommandBuffers.resize(mSwapChain->getImageCount());

		createCommandBuffers();

		createSyncObjects();
	}

	void Application::mainLoop() {
		while (!mWindow->shouldClose()) {
			mWindow->pollEvents();

			mModel->update();

			mUniformManager->update(mVPMatrices, mModel->getUniform(), mCurrentFrame);
			render();
		}

		vkDeviceWaitIdle(mDevice->getDevice());
	}
	void Application::createPipeline() {
		//设置视口
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)mWidth;
		viewport.height = (float)mHeight;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = { mWidth, mHeight };

		mPipeline->setViewports({ viewport });
		mPipeline->setScissors({ scissor });

		//step1.shader
		//设置shader
		std::vector<Wrapper::Shader::Ptr> shaderGroup{};

		auto shaderVertex = Wrapper::Shader::create(mDevice, "shaders/vs.spv", VK_SHADER_STAGE_VERTEX_BIT, "main");
		shaderGroup.push_back(shaderVertex);

		auto shaderFragment = Wrapper::Shader::create(mDevice, "shaders/fs.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "main");
		shaderGroup.push_back(shaderFragment);

		mPipeline->setShaderGroup(shaderGroup);

		//step2.Fixed-function
		//顶点的排布模式
		auto vertexBindingDes = mModel->getVertexInputBindingDescriptions();
		auto vertexAttributeDes = mModel->getAttributeDescriptions();

		mPipeline->mVertexInputState.vertexBindingDescriptionCount = vertexBindingDes.size();
		mPipeline->mVertexInputState.pVertexBindingDescriptions = vertexBindingDes.data();
		mPipeline->mVertexInputState.vertexAttributeDescriptionCount = vertexAttributeDes.size();
		mPipeline->mVertexInputState.pVertexAttributeDescriptions = vertexAttributeDes.data();

		//图元装配
		mPipeline->mAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		mPipeline->mAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		mPipeline->mAssemblyState.primitiveRestartEnable = VK_FALSE;

		//光栅化设置
		mPipeline->mRasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		mPipeline->mRasterState.polygonMode = VK_POLYGON_MODE_FILL;//其他模式需要开启gpu特性
		mPipeline->mRasterState.lineWidth = 1.0f;//大于1需要开启gpu特性
		mPipeline->mRasterState.cullMode = VK_CULL_MODE_BACK_BIT;
		mPipeline->mRasterState.frontFace = VK_FRONT_FACE_CLOCKWISE;

		mPipeline->mRasterState.depthBiasEnable = VK_FALSE;
		mPipeline->mRasterState.depthBiasConstantFactor = 0.0f;
		mPipeline->mRasterState.depthBiasClamp = 0.0f;
		mPipeline->mRasterState.depthBiasSlopeFactor = 0.0f;

		//TODO:多重采样
		mPipeline->mSampleState.sampleShadingEnable = VK_FALSE;
		mPipeline->mSampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		mPipeline->mSampleState.minSampleShading = 1.0f;
		mPipeline->mSampleState.pSampleMask = nullptr;
		mPipeline->mSampleState.alphaToCoverageEnable = VK_FALSE;
		mPipeline->mSampleState.alphaToOneEnable = VK_FALSE;
		//深度与模板测试
		mPipeline->mDepthStencilState.depthTestEnable = VK_TRUE;
		mPipeline->mDepthStencilState.depthWriteEnable = VK_TRUE;
		mPipeline->mDepthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

		VkPipelineColorBlendAttachmentState blendAttachment{};
		blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
			VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;

		blendAttachment.blendEnable = VK_FALSE;
		blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;

		blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
		mPipeline->pushBlendAttachment(blendAttachment);

		//1 blend有两种计算方式，第一种如上所述，进行alpha为基础的计算，第二种进行位运算
		//2 如果开启了logicOp，那么上方设置的alpha为基础的运算，失灵
		//3 ColorWrite掩码，仍然有效，即便开启了logicOP
		//4 因为，我们可能会有多个FrameBuffer输出，所以可能需要多个BlendAttachment

		//配合blendAttachment的factor与operation
		mPipeline->mBlendState.blendConstants[0] = 0.0f;
		mPipeline->mBlendState.blendConstants[1] = 0.0f;
		mPipeline->mBlendState.blendConstants[2] = 0.0f;
		mPipeline->mBlendState.blendConstants[3] = 0.0f;

		//We need to specify the descriptor set layout during pipeline creation to tell Vulkan which descriptors the shaders will be using.
		//Descriptor set layouts are specified in the pipeline layout object.		
		//uniform的传递
		auto layout = mUniformManager->getDescriptorLayout();

		mPipeline->mLayoutState.setLayoutCount = 1;
		mPipeline->mLayoutState.pSetLayouts = &layout;
		mPipeline->mLayoutState.pushConstantRangeCount = 0;
		mPipeline->mLayoutState.pPushConstantRanges = nullptr;

		mPipeline->build();

        for(int i = 0; i< mSwapChain->getImageCount(); i++){
            auto imageSemaphore = Wrapper::Semaphore::create(mDevice);
            mImageAvailableSemaphores.push_back(imageSemaphore);
            auto renderSemaphore = Wrapper::Semaphore::create(mDevice);
            mRenderFinishedSemaphores.push_back(renderSemaphore);
            auto fence = Wrapper::Fence::create(mDevice);
            mFences.push_back(fence);
        }
	}

	void Application::createRenderPass() {
		//输入画布的描述
		VkAttachmentDescription attachmentDes{};
		attachmentDes.format = mSwapChain->getFormat();
		attachmentDes.samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDes.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDes.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDes.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDes.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDes.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachmentDes.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		mRenderPass->addAttachment(attachmentDes);

		VkAttachmentDescription depthAttachmentDes{};
		depthAttachmentDes.format = Wrapper::Image::findDepthFormat(mDevice);
		depthAttachmentDes.samples = VK_SAMPLE_COUNT_1_BIT;
		depthAttachmentDes.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachmentDes.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachmentDes.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachmentDes.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachmentDes.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachmentDes.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		mRenderPass->addAttachment(depthAttachmentDes);

		//对于画布的索引设置以及格式要求
		VkAttachmentReference attachmentRef{};
		attachmentRef.attachment = 0;
		attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		//创建子流程
		Wrapper::SubPass subPass{};
		subPass.addColorAttachmentReference(attachmentRef);
		subPass.setDepthStencilAttachmentReference(depthAttachmentRef);
		subPass.buildSubPassDescription();

		mRenderPass->addSubPass(subPass);

		//子流程之间的依赖关系
		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		mRenderPass->addDependency(dependency);

		mRenderPass->buildRenderPass();
	}

	void Application::createCommandBuffers() {
	for (int i = 0; i < mSwapChain->getImageCount(); ++i) {
			mCommandBuffers[i] = Wrapper::CommandBuffer::create(mDevice, mCommandPool);

			mCommandBuffers[i]->begin();

			VkRenderPassBeginInfo renderBeginInfo{};
			renderBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderBeginInfo.renderPass = mRenderPass->getRenderPass();
			renderBeginInfo.framebuffer = mSwapChain->getFrameBuffer(i);
			renderBeginInfo.renderArea.offset = { 0, 0 };
			renderBeginInfo.renderArea.extent = mSwapChain->getExtent();

			VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
			renderBeginInfo.clearValueCount = 1;
			renderBeginInfo.pClearValues = &clearColor;


			mCommandBuffers[i]->beginRenderPass(renderBeginInfo);

			mCommandBuffers[i]->bindGraphicPipeline(mPipeline->getPipeline());

			mCommandBuffers[i]->bindDescriptorSet(mPipeline->getLayout(), mUniformManager->getDescriptorSet(mCurrentFrame));

			//mCommandBuffers[i]->bindVertexBuffer({ mModel->getVertexBuffer()->getBuffer() });

			mCommandBuffers[i]->bindVertexBuffer({ mModel->getVertexBuffers() });

			mCommandBuffers[i]->bindIndexBuffer(mModel->getIndexBuffer()->getBuffer());

			mCommandBuffers[i]->drawIndex(mModel->getIndexCount());

			mCommandBuffers[i]->endRenderPass();

			mCommandBuffers[i]->end();
		}
	}

	void Application::createSyncObjects(){
		for (int i = 0; i < mSwapChain->getImageCount(); ++i) {
			auto imageSemaphore = Wrapper::Semaphore::create(mDevice);
			mImageAvailableSemaphores.push_back(imageSemaphore);

			auto renderSemaphore = Wrapper::Semaphore::create(mDevice);
			mRenderFinishedSemaphores.push_back(renderSemaphore);

			auto fence = Wrapper::Fence::create(mDevice);
			mFences.push_back(fence);
		}
	}

	void Application::recreateSwapChain(){
		int width = 0, height = 0;
		glfwGetFramebufferSize(mWindow->getWindow(), &width, &height);
		while (width == 0 || height == 0) {
			glfwWaitEvents();
			glfwGetFramebufferSize(mWindow->getWindow(), &width, &height);
		}

		vkDeviceWaitIdle(mDevice->getDevice());

		cleanupSwapChain();

		mSwapChain = Wrapper::SwapChain::create(mDevice, mWindow, mSurface);
		mWidth = mSwapChain->getExtent().width;
		mHeight = mSwapChain->getExtent().height;

		mRenderPass = Wrapper::RenderPass::create(mDevice);
		createRenderPass();

		mSwapChain->createFrameBuffers(mRenderPass);

		mPipeline = Wrapper::Pipeline::create(mDevice, mRenderPass);
		createPipeline();

		mCommandBuffers.resize(mSwapChain->getImageCount());

		createCommandBuffers();

		createSyncObjects();
	}

	void Application::cleanupSwapChain(){
		//每个对象里存储了依赖对象的智能指针
		mSwapChain.reset();
		mCommandBuffers.clear();
		mPipeline.reset();
		mRenderPass.reset();
		mImageAvailableSemaphores.clear();
		mRenderFinishedSemaphores.clear();
		mFences.clear();
	}
	

	void Application::render() {
		//1.Wait for the previous frame to finish
		//2.Acquire an image from the swap chain
		//3.Record a command buffer which draws the scene onto the image
		//4.Submit the recorded command buffer
		//5.Present the swap chain image

		//there are a number of events that we need to order explicitly because they happen on the GPU :
		//Acquire an image from the swap chain
		//Execute commands that draw onto the acquired image
		//Present that image to the screen for presentation, returning it to the swapchain
		//Each of these events is set in motion using a single function call, but are all executed asynchronously.The function calls will return before the operations are actually finished and the order of execution is also undefined.
		//That is unfortunate, because each of the operations depends on the previous one finishing.Thus we need to explore which primitives we can use to achieve the desired ordering.

		//等待当前要提交的CommandBuffer执行完毕
		mFences[mCurrentFrame]->block();;//to make the CPU wait for one or more fences to be signaled by the GPU.

		//获取交换链当中的下一帧
		uint32_t imageIndex{ 0 };

		vkAcquireNextImageKHR(
			mDevice->getDevice(),
			mSwapChain->getSwapChain(),
			UINT32_MAX,
			mImageAvailableSemaphores[mCurrentFrame]->getSemaphore(),
			VK_NULL_HANDLE,
			&imageIndex);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		//wait with writing colors to the image until waitSemaphore available, 
		VkSemaphore waitSemaphores[] = { mImageAvailableSemaphores[mCurrentFrame]->getSemaphore() }; 
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;//which semaphores to wait on before execution begin
		submitInfo.pWaitDstStageMask = waitStages;// in which stage(s) of the pipeline to wait

		//work is represented as a sequence of commands
		//that are created and recorded into "command buffers" by cpu
		//they are submitted to gpu to execution

		auto commandBuffer = mCommandBuffers[imageIndex]->getCommandBuffer() ;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		//执行命令后light which Semaphore
		VkSemaphore lightSemaphores[] = { mRenderFinishedSemaphores[mCurrentFrame]->getSemaphore() };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = lightSemaphores;

		mFences[mCurrentFrame]->resetFence();

		if (vkQueueSubmit(mDevice->getGraphicQueue(), 1, &submitInfo, mFences[mCurrentFrame]->getFence() ) != VK_SUCCESS) {
			throw std::runtime_error("Error:failed to submit renderCommand");
		}

		//render command above, present command below
		//drawing a frame is submitting the result back to the swap chain to have it eventually show up on the screen
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = lightSemaphores; //specify which semaphores to wait on before presentation can happen,

		VkSwapchainKHR swapChains[] = { mSwapChain->getSwapChain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains; // specify the swap chains to present images to
		presentInfo.pImageIndices = &imageIndex;// the index of the image for each swap chain
		presentInfo.pResults = nullptr;

		vkQueuePresentKHR(mDevice->getPresentQueue(), &presentInfo);//submits the request to present an image to the swap chain

		mCurrentFrame = (mCurrentFrame + 1) % mSwapChain->getImageCount();
	}


	void Application::cleanUp() {
		mPipeline.reset();

		mRenderPass.reset();

		mSwapChain.reset();

		mDevice.reset();
		mSurface.reset();
		mInstance.reset();
		mWindow.reset();
	
	}
}