# Vulkan Wrapper Project

## Overview

This project is a Vulkan wrapper, based on  the Vulkan Tutorial and Xinzheng Zhao's course. The wrapper abstracts Vulkan API calls, making it easier to manage Vulkan resources and rendering pipelines
## Compile

Compile this project using the standard cmake routine:

    mkdir build
    cd build
    cmake ..
    make

## Dependencies

GLM: A header-only library for OpenGL Mathematics, used for matrix and vector operations.
GLFW: A library for creating windows with OpenGL/Vulkan contexts and managing input.
Vulkan: The Vulkan API for rendering.

Create a directory named thirdParty at the root level of your project (relative to ${CMAKE_CURRENT_SOURCE_DIR}/../thirdParty/), and place the required libraries in this directory.
Ensure that the necessary include and library paths are correctly specified in your CMakeLists.txt file to link against these dependencies.
