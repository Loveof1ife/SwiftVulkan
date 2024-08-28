#include "commandBuffer.h"

namespace Tea::Wrapper {

	CommandBuffer::CommandBuffer(const Device::Ptr& device, const CommandPool::Ptr& commandPool, bool asSecondary) {
		mDevice = device;
		mCommandPool = commandPool;
		
		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandBufferCount = 1;
		allocInfo.commandPool = mCommandPool->getCommandPool();
		allocInfo.level = asSecondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		//When the function vkAllocateCommandBuffers is called, 
		//Vulkan allocates the requested number of command buffers from the specified command pool. 
		//The function then stores the handles (references) to these allocated command buffers in the array pointed to by pCommandBuffers.
		//VkCommandBufferAllocateInfo: 
		//commandBufferCount specifies the number of command buffers that we want to allocate from the pool.
		//pCommandBuffer specifies the pointer &mCommandBuffer
		if (vkAllocateCommandBuffers(mDevice->getDevice(), &allocInfo, &mCommandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Error: falied to create commandBuffer");
		}
	}

	CommandBuffer::~CommandBuffer() {}//会随着CommandPool析构，而析构释放

	void CommandBuffer::begin(VkCommandBufferUsageFlags flag, const VkCommandBufferInheritanceInfo& inheritance) {
		//VkCommandBufferUsageFlags
		//VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT：这个命令，只会被使用提交一次
		//VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT：这个命令缓冲，是一个二级缓冲，位于一个renderPass当中
		//VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT:命令已经被提交了，执行期间，可以再次提交
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = flag;
		beginInfo.pInheritanceInfo = &inheritance;

		if (vkBeginCommandBuffer(mCommandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("Error:failed to begin commandBuffer");
		}
	}

	void CommandBuffer::beginRenderPass(const VkRenderPassBeginInfo& renderPassBeginInfo, 
										const VkSubpassContents& subPassContents ) {
		vkCmdBeginRenderPass(mCommandBuffer, &renderPassBeginInfo, subPassContents);
	}

	void CommandBuffer::bindGraphicPipeline(const VkPipeline& pipeline) {
		vkCmdBindPipeline(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	}

	void CommandBuffer::bindDescriptorSet(const VkPipelineLayout layout, const VkDescriptorSet& descriptorSet) {
		vkCmdBindDescriptorSets(mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &descriptorSet, 0, nullptr);
	}

	void CommandBuffer::bindVertexBuffer(const std::vector<VkBuffer>& buffers){
		std::vector<VkDeviceSize> offsets(buffers.size(), 0);

		vkCmdBindVertexBuffers(mCommandBuffer, 0, static_cast<uint32_t>(buffers.size()), buffers.data(), offsets.data());
	}

	void CommandBuffer::bindIndexBuffer(const VkBuffer& buffer){
		vkCmdBindIndexBuffer(mCommandBuffer, buffer, 0, VK_INDEX_TYPE_UINT32);
	}

	void CommandBuffer::draw(size_t vertexCount) {
		vkCmdDraw(mCommandBuffer, vertexCount, 1, 0, 0);
	}

	void CommandBuffer::drawIndex(size_t indexCount){
		vkCmdDrawIndexed(mCommandBuffer, indexCount, 1, 0, 0, 0);
	}

	void CommandBuffer::endRenderPass() {
		vkCmdEndRenderPass(mCommandBuffer);
	}

	void CommandBuffer::end() {
		if (vkEndCommandBuffer(mCommandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("Error:failed to end Command Buffer");
		}
	}
	void CommandBuffer::copyBufferToBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t copyInfoCount, const std::vector<VkBufferCopy>& copyInfos){
		vkCmdCopyBuffer(mCommandBuffer, srcBuffer, dstBuffer, copyInfoCount, copyInfos.data() );
	}
	
	void CommandBuffer::copyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t width, uint32_t height) {
		VkBufferImageCopy region{};

		//为0代表不需要进行padding
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { width, height, 1 };

		vkCmdCopyBufferToImage(mCommandBuffer, srcBuffer, dstImage, dstImageLayout, 1, &region);
	}

	void CommandBuffer::submitSync(VkQueue queue, VkFence fence){
		//The submitSync method is called to submit the command buffer to the graphics queue and wait for the operation to complete
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &mCommandBuffer;

		vkQueueSubmit(queue, 1, &submitInfo, fence);

		vkQueueWaitIdle(queue);
	}

	void CommandBuffer::transferImageLayout(const VkImageMemoryBarrier& imageMemoryBarrier, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask) {
		//All types of pipeline barriers are submitted using the same function. 
		vkCmdPipelineBarrier(
			mCommandBuffer,
			srcStageMask,
			dstStageMask,
			0,
			0, nullptr,//MemoryBarrier
			0, nullptr, //BufferMemoryBarrier
			1, &imageMemoryBarrier
		);
	}
	}
	
