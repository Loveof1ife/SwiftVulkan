#include "device.h"

namespace Tea::Wrapper {

	const std::vector<const char*> deviceRequiredExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	Device::Device(Instance::Ptr instance, WindowSurface::Ptr surface) {
		mInstance = instance;
		mSurface = surface;
		pickPhysicalDevice();
		initQueueFamilies(mPhysicalDevice);
		createLogicalDevice();
	}

	Device::~Device() {
		vkDestroyDevice(mDevice, nullptr);
		mSurface.reset();
		mInstance.reset();
	}
	
	void Device::pickPhysicalDevice() {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(mInstance->getInstance(), &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("Error:failed to enumeratePhysicalDevice");
		}
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(mInstance->getInstance(), &deviceCount, devices.data() );

		std::multimap<int, VkPhysicalDevice> candidates;
		for (const auto& device : devices) {
			int score = rateDevice(device);
			candidates.insert(std::make_pair(score, device));
		}

		if (candidates.rbegin()->first > 0 && isDeviceSuitable(candidates.rbegin()->second)) {
			mPhysicalDevice = candidates.rbegin()->second;
		}

		if (mPhysicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("Error:failed to get physical device");
		}
	}
	int Device::rateDevice(VkPhysicalDevice device) {
		int score = 0;

		//设备名称 类型 支持vulkan的版本
		VkPhysicalDeviceProperties  deviceProp;
		vkGetPhysicalDeviceProperties(device, &deviceProp);

		//纹理压缩 浮点数运算特性 多视口渲染
		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		if (deviceProp.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			score += 1000;
		}

		score += deviceProp.limits.maxImageDimension2D;

		if (!deviceFeatures.geometryShader) {
			return 0;
		}

		return score;

	}
	bool Device::isDeviceSuitable(VkPhysicalDevice device) {
		VkPhysicalDeviceProperties deviceProp;
		vkGetPhysicalDeviceProperties(device, &deviceProp);

		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		return deviceProp.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
			deviceFeatures.geometryShader;
	}

	void Device::initQueueFamilies(VkPhysicalDevice device) {
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
		/*	The queueFamilyIndex field specifies the
		family of the queues you want to create.This is an index into the array of queue families returned
		from vkGetPhysicalDeviceQueueFamilyProperties()*/
		int i = 0;
		for (const auto& queueFamily : queueFamilies) {
			if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) 
				mQueueFamilyIndices.graphicsFamily = i;
			
			//寻找支持显示的队列族
			VkBool32 presentSupport = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, mSurface->getSurface(), &presentSupport);

			if (presentSupport) 
				mQueueFamilyIndices.presentFamily = i;

			if (mQueueFamilyIndices.isComplete())
				break;
			
			i++;
		}
	}

	void Device::createLogicalDevice() {
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> queueFamilies = { mQueueFamilyIndices.graphicsFamily.value(), mQueueFamilyIndices.presentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : queueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;

			queueCreateInfos.push_back(queueCreateInfo);
		}
		//填写逻辑设备创建信息
		VkDeviceCreateInfo deviceCreateInfo{};

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceRequiredExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = deviceRequiredExtensions.data();

		if (mInstance->getEnableValidationLayer() ) {
			deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();

		}
		else {
			deviceCreateInfo.enabledLayerCount = 0;
			deviceCreateInfo.ppEnabledLayerNames = nullptr;
		}
		if (vkCreateDevice(mPhysicalDevice, &deviceCreateInfo, nullptr, &mDevice) != VK_SUCCESS) {
			throw std::runtime_error("Error:failed to create logical device");
		}

		//The queues are automatically created along with the logical device, but we don't have a handle to interface with them yet
		//add a class member::VkQueue graphics(present)Queue;  to store a handle to the xxqueue retrieve the queue handle:
		vkGetDeviceQueue(mDevice, mQueueFamilyIndices.graphicsFamily.value(), 0, &mGraphicQueue);
		vkGetDeviceQueue(mDevice, mQueueFamilyIndices.presentFamily.value(), 0, &mPresentQueue);
	}
}
