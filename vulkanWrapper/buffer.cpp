#include "buffer.h"
#include "commandBuffer.h"
#include "commandPool.h"

namespace Tea::Wrapper {
   Buffer::Ptr Buffer::createVertexBuffer(const Device::Ptr& device, VkDeviceSize size, void* pData){
      auto buffer = create(device, size,
                           static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT),
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

      return buffer;
   }

   Buffer::Ptr Buffer::createIndexBuffer(const Device::Ptr& device, VkDeviceSize size, void* pData){
      auto buffer = create(device, size,
                           static_cast<VkBufferUsageFlagBits>(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT),
                           VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

      return buffer;

   }

   Buffer::Ptr Buffer::createUniformBuffer(const Device::Ptr& device, VkDeviceSize size, void* pData) {
       auto buffer = create(device, size,
           VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
       );

       if (pData != nullptr)
           buffer->updateBufferByStage(pData, size);

       return buffer;
   }

   Buffer::Ptr Buffer::createStageBuffer(const Device::Ptr& device, VkDeviceSize size, void* pData)
   {
       auto buffer = create(device, size,
           VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

       if (pData != nullptr) {
           buffer->updateBufferByStage(pData, size);
       }

       return buffer;
   }

    uint32_t Buffer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties){
      VkPhysicalDeviceMemoryProperties memProperties;
      vkGetPhysicalDeviceMemoryProperties(mDevice->getPhysicalDevice(), &memProperties);

      for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
         if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) ) {
            return i;
         }
      }
      
      throw std::runtime_error("failed to find suitable memory type!");
    }

    Buffer::Buffer(const Device::Ptr& device, VkDeviceSize size, VkBufferUsageFlagBits usage,
                   VkMemoryPropertyFlags properties) {
       mDevice = device;
       VkBufferCreateInfo createInfo{};
       createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
       createInfo.size = size;
       createInfo.usage = usage;
       createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

       if(vkCreateBuffer(mDevice->getDevice(), &createInfo, nullptr, &mBuffer)!= VK_SUCCESS){
           throw std::runtime_error("Error: failed to allocate memory");
       }

       VkMemoryRequirements memReq{};
       vkGetBufferMemoryRequirements(mDevice->getDevice(), mBuffer, &memReq);

       VkMemoryAllocateInfo allocInfo{};
       allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
       allocInfo.allocationSize = memReq.size;
       //符合我上述buffer需求的内存类型的IDs:0x001 0x010
       allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, properties);

       if (vkAllocateMemory(mDevice->getDevice(), &allocInfo, nullptr, &mBufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("Error: failed to allocate memory");
       }

       vkBindBufferMemory(mDevice->getDevice(), mBuffer, mBufferMemory, 0);

       mBufferInfo.buffer = mBuffer;
       mBufferInfo.offset = 0;
       mBufferInfo.range = size;
   }

    Buffer::~Buffer() {
        if (mBuffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(mDevice->getDevice(), mBuffer, nullptr);
        }

        if (mBufferMemory != VK_NULL_HANDLE) {
            vkFreeMemory(mDevice->getDevice(), mBufferMemory, nullptr);
        }
    }

    void Buffer::updateBufferByMap(void* data, size_t size) {
        void* memPtr{nullptr};

        vkMapMemory(mDevice->getDevice(), mBufferMemory, 0, size, 0, &memPtr);
        memcpy(memPtr, data, size);
        vkUnmapMemory(mDevice->getDevice(), mBufferMemory);
    }

    void Buffer::updateBufferByStage(void* data, size_t size) {
        auto stageBuffer = create(mDevice, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        stageBuffer->updateBufferByMap(data, size);

        copyBuffer(stageBuffer->getBuffer(), mBuffer, static_cast<VkDeviceSize>(size));
    }

    void Buffer::copyBuffer(const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, VkDeviceSize size){
        auto commandPool = CommandPool::create(mDevice);
        auto commandBuffer = CommandBuffer::create(mDevice, commandPool);

        commandBuffer->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

        VkBufferCopy copyInfo{};
        copyInfo.size = size;
        copyInfo.dstOffset = 0;
        copyInfo.srcOffset = 0;

        commandBuffer->copyBufferToBuffer(srcBuffer, dstBuffer, 1, { copyInfo });

        commandBuffer->end();

        commandBuffer->submitSync(mDevice->getGraphicQueue(), VK_NULL_HANDLE);
    }


}
