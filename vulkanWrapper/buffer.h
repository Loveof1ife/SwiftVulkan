#pragma once

#include "../base.h"
#include "device.h"
namespace Tea::Wrapper {

   class Buffer{
   public: 
      using Ptr = std::shared_ptr<Buffer>;
      static Ptr create(const Device::Ptr& device, VkDeviceSize size, VkBufferUsageFlagBits usage, VkMemoryPropertyFlags properties){
         return std::make_shared<Buffer>(device, size, usage, properties);
      }

      static Ptr createVertexBuffer(const Device::Ptr& device, VkDeviceSize size, void * pData);

      static Ptr createIndexBuffer(const Device::Ptr& device, VkDeviceSize size, void * pData);

      static Ptr createUniformBuffer(const Device::Ptr& device, VkDeviceSize size, void* pData);

      static Ptr createStageBuffer(const Device::Ptr& device, VkDeviceSize size, void* pData);

      Buffer(const Device::Ptr& device, VkDeviceSize size, VkBufferUsageFlagBits usage, VkMemoryPropertyFlags properties);
         
      ~Buffer();

    /*
    * 1 通过内存Mapping的形式，直接对内存进行更改，适用于HostVisible(cpu可见)类型的内存
    * 2 如果本内存是LocalOptimal， 那么就必须创建中间的StageBuffer，先复制到StageBuffer，再拷贝入目标Buffer
    */

      void updateBufferByMap(void* data, size_t size);

      void updateBufferByStage(void* data, size_t size);

      void copyBuffer(const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, VkDeviceSize size);

      [[nodiscard]] auto getBuffer() const { return mBuffer; }

      [[nodiscard]] VkDescriptorBufferInfo& getBufferInfo() { return mBufferInfo; }
      
   private:
      uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

      Device::Ptr mDevice;
      //description in cpu, vkCreateBuffer 一般是分配在cpu上的描述符
      VkBuffer mBuffer{VK_NULL_HANDLE };
      //memory in gpu
      VkDeviceMemory mBufferMemory{VK_NULL_HANDLE };

      VkDescriptorBufferInfo mBufferInfo{};
   };

}
