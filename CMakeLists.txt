cmake_minimum_required(VERSION 3.12)
project(vulkanLearning-1)

set(CMAKE_CXX_STANDARD 17)

aux_source_directory(. DIRSRCS)

include_directories(
        SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../thirdParty/glm/include
        SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../thirdParty/glfw/include
        SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../thirdParty/vulkan/Include
)

link_directories(
        SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../thirdParty/glfw/lib
        SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../thirdParty/vulkan/Lib
)

add_subdirectory(vulkanWrapper)
add_subdirectory(texture)
    
add_executable(tea  ${DIRSRCS})
target_link_libraries(tea vulkanLib vulkan-1.lib textureLib glfw3.lib)
