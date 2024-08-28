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
    * 1 ͨ���ڴ�Mapping����ʽ��ֱ�Ӷ��ڴ���и��ģ�������HostVisible(cpu�ɼ�)���͵��ڴ�
    * 2 ������ڴ���LocalOptimal�� ��ô�ͱ��봴���м��StageBuffer���ȸ��Ƶ�StageBuffer���ٿ�����Ŀ��Buffer
    */

      void updateBufferByMap(void* data, size_t size);

      void updateBufferByStage(void* data, size_t size);

      void copyBuffer(const VkBuffer& srcBuffer, const VkBuffer& dstBuffer, VkDeviceSize size);

      [[nodiscard]] auto getBuffer() const { return mBuffer; }

      [[nodiscard]] VkDescriptorBufferInfo& getBufferInfo() { return mBufferInfo; }
      
   private:
      uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

      Device::Ptr mDevice;
      //description in cpu, vkCreateBuffer һ���Ƿ�����cpu�ϵ�������
      VkBuffer mBuffer{VK_NULL_HANDLE };
      //memory in gpu
      VkDeviceMemory mBufferMemory{VK_NULL_HANDLE };

      VkDescriptorBufferInfo mBufferInfo{};
   };

}
