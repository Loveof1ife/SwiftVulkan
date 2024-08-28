#pragma once

#include "../base.h"
#include "device.h"

namespace Tea::Wrapper {

	//Each subpass references a number of attachments(from the array you passed in
	//pAttachments) as inputs or outputs.
	//VkAttachmentReference structure or an
	//array of them and forms a reference to one of the attachments described in pAttachments. 
	
	//attachment
	//VkAttachmentDescription������һ����ɫ�������ģ�帽�ŵĽṹ����������һ�������ĸ��ţ�����һ�ֵ���������
	//VkAttachmentDescription1 VkAttachmentDescription2 VkAttachmentDescription3 VkAttachmentDescription4�����ģ�壩��������磬��Ҫʲô
	//VkAttachmentReference ˵������SubPass ��Ҫ��attachment���е�һ������һ������������id�Ƕ��٣���һ�����ŵ�ͼƬ������ʽ��ʲô
	//VkSubpassDescription������дһ����Pass�������ṹ
	//VkSubpassDependency��������ͬ��������֮���������ϵ

	class SubPass {
	public:
		SubPass();

		~SubPass();

		void addColorAttachmentReference(const VkAttachmentReference& ref);

		void addInputAttachmentReference(const VkAttachmentReference& ref);

		void setDepthStencilAttachmentReference(const VkAttachmentReference& ref);

		void buildSubPassDescription();

		[[nodiscard]] auto getSubPassDescription() const { return mSubPassDescription; }
	private:
		//subpuss == description + reference
		VkSubpassDescription mSubPassDescription{};

		std::vector<VkAttachmentReference> mColorAttachmentReferences{};
		std::vector<VkAttachmentReference> mInputAttachmentReferences{};
		VkAttachmentReference mDepthStencilAttachmentReference{};
	};

	//A single render pass can consist of multiple subpasses
	//def: subpasses are subsequent rendering operations, which depend on the contents of framebuffers in previous passes
	//group these rendering operations into one render pass, then vulkan is able to reorder the operations  for better performence

	class RenderPass {
	public:
		using Ptr = std::shared_ptr<RenderPass>;

		static Ptr create(Device::Ptr& device) {
			return std::make_shared<RenderPass>(device);
		}
		
		RenderPass(const Device::Ptr& device);

		~RenderPass();

		//add at application
		void addSubPass(const SubPass& subpass);

		void addDependency(const VkSubpassDependency& dependency);

		void addAttachment(const VkAttachmentDescription& attachmentDes);

		void buildRenderPass();

		[[nodiscard]] auto getRenderPass() const { return mRenderPass; }

	private:
		VkRenderPass mRenderPass{ VK_NULL_HANDLE };
		//each mSubPass contains it attachmentdescrtption
		std::vector<SubPass> mSubPasses{};
		
		std::vector<VkAttachmentDescription> mAttachmentDescriptions{};

		std::vector<VkSubpassDependency> mDependencies{};

		Device::Ptr mDevice{ nullptr };
	};
}