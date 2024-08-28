#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <array>
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <algorithm> // Necessary for std::clamp

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>


const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

struct VPMatrices {
	glm::mat4 mViewMatrix;
	glm::mat4 mProjectionMatrix;

	VPMatrices() {
		mViewMatrix = glm::mat4(1.0f);
		mProjectionMatrix = glm::mat4(1.0f);
	}
};

struct ObjectUniform {
	glm::mat4 mModelMatrix;

	ObjectUniform() {
		mModelMatrix = glm::mat4(1.0f);
	}
};

//#ifdef NDEBUG
//const bool enableValidationLayers = false;
//#else
//const bool enableValidationLayers = true;
//#endif

