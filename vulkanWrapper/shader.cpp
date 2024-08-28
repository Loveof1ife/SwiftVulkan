#include "shader.h"

namespace Tea::Wrapper {
	//Static Function in a Namespace:meaning it belongs to the namespace and can be called without an instance of a class.
	static std::vector<char> readFile(const std::string& fileName) {
		std::ifstream file(fileName.c_str(), std::ios::ate | std::ios::binary | std::ios::in);

		if (!file) {
			throw std::runtime_error("Error: failed to open shader file");
		}

		const size_t fileSize = file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}


	Shader::Shader(const Device::Ptr& device, const std::string& fileName, VkShaderStageFlagBits shaderStage, const std::string& entryPoint){
		mDevice = device;
		mShaderStage = shaderStage;
		mEntryPoint = entryPoint;

		std::vector<char> codeBuffer = readFile(fileName);

		VkShaderModuleCreateInfo shaderCreateInfo{};
		shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderCreateInfo.codeSize = codeBuffer.size();
		shaderCreateInfo.pCode = reinterpret_cast<const uint32_t*>(codeBuffer.data() );


		if (vkCreateShaderModule(mDevice->getDevice(), &shaderCreateInfo, nullptr, &mShaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}
	}

	Shader::~Shader(){
		if (mShaderModule != VK_NULL_HANDLE) {
			vkDestroyShaderModule(mDevice->getDevice(), mShaderModule, nullptr);
		}
	}

}
