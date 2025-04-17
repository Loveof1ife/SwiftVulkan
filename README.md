# Vulkan Wrapper Project

## 项目概述
这是一个基于 Vulkan Tutorial 和 Xinzheng Zhao 课程的 Vulkan 封装器项目。该封装器对 Vulkan 原生 API 进行了高层抽象，旨在简化 Vulkan 资源管理和渲染管线的使用。
Vulkan 是一个低开销、显式控制的图形 API，是 OpenGL 的替代者。然而，它的 API 设计非常底层，直接使用需要编写大量样板代码，例如手动管理资源生命周期、同步对象和命令缓冲区等。封装器的主要目标是：
### 简化代码：隐藏 Vulkan 的繁琐细节，例如对象的创建、销毁和错误检查。
### 提高安全性：通过 RAII（资源获取即初始化）自动管理资源，例如使用 std::shared_ptr 防止内存泄漏。
### 提供现代 C++ 接口：用类和方法替代 Vulkan 的 C 风格函数。

## 项目教学

### CommandBuffer:
在 Vulkan 中，命令缓冲区用于记录 GPU 要执行的指令（如绘制、内存复制、管线绑定等）。命令需要先录制（Recording），再提交（Submit）到队列（Queue）执行。原生 Vulkan API 需要手动管理命令缓冲区的生命周期、状态转换和同步。而通过封装器，我们可以实现以下改进：
class CommandBuffer {
public:
    using Ptr = std::shared_ptr<CommandBuffer>;  // 智能指针管理生命周期
    static Ptr create(...);  // 工厂方法创建对象

    // 构造函数/析构函数（RAII）
    CommandBuffer(...);
    ~CommandBuffer();

    // 命令录制接口
    void begin(...);
    void beginRenderPass(...);
    void bindGraphicPipeline(...);
    void bindDescriptorSet(...);
    void bindVertexBuffer(...);
    void bindIndexBuffer(...);
    void draw(...);
    void drawIndex(...);
    void endRenderPass();
    void end();

    // 内存操作
    void copyBufferToBuffer(...);
    void copyBufferToImage(...);
    void transferImageLayout(...);

    // 提交命令
    void submitSync(...);

    // 获取底层 Vulkan 对象
    [[nodiscard]] auto getCommandBuffer() const { return mCommandBuffer; }

private:
    VkCommandBuffer mCommandBuffer{ VK_NULL_HANDLE };
    Device::Ptr mDevice;
    CommandPool::Ptr mCommandPool;
};

| 原生 Vulkan 的问题 | 封装器的改进 | 
| --- | --- | 
| 需要手动调用  和  | RAII 自动管理（构造时分配，析构时释放） | 
| 命令录制需要手动调用 、 | 提供 / 方法，隐藏细节 | 
| 提交命令需要手动处理  和同步（Fence/Semaphore） | 提供  简化提交 | 
| 绑定管线、描述符集、顶点缓冲区等操作繁琐 | 封装成  方法，链式调用更直观 | 
| 内存操作（Buffer/Image 复制、布局转换）需要复杂参数 | 提供  等高层接口 | 


## Compile
开发环境: vscode + Visual Studio Community 2022 + cmake
使用标准的 CMake 流程编译此项目：

    mkdir build
    cd build
    cmake ..
    make

## Dependencies

GLM: A header-only library for OpenGL Mathematics, used for matrix and vector operations.
GLFW: A library for creating windows with OpenGL/Vulkan contexts and managing input.
Vulkan: The Vulkan API for rendering.

在项目根目录下创建一个名为 thirdParty 的目录（相对于 ${CMAKE_CURRENT_SOURCE_DIR}/../thirdParty/），并将所需的库放置在此目录中。确保在 CMakeLists.txt 文件中正确指定了必要的包含路径和库路径，以便链接这些依赖项。
