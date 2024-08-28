#pragma once

#include "../base.h"
#include "device.h"

namespace Tea::Wrapper {

	//Each subpass references a number of attachments(from the array you passed in
	//pAttachments) as inputs or outputs.
	//VkAttachmentReference structure or an
	//array of them and forms a reference to one of the attachments described in pAttachments. 
	
	//attachment
	//VkAttachmentDescription：描述一个颜色或者深度模板附着的结构，他并不是一个真正的附着，而是一种单纯的描述
	//VkAttachmentDescription1 VkAttachmentDescription2 VkAttachmentDescription3 VkAttachmentDescription4（深度模板），告诉外界，我要什么
	//VkAttachmentReference 说明本个SubPass 需要的attachment其中的一个，这一个的数组索引id是多少，这一个附着的图片期望格式是什么
	//VkSubpassDescription用来填写一个子Pass的描述结构
	//VkSubpassDependency：描述不同的子流程之间的依赖关系

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