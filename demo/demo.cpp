
#include <gbBase/Finally.hpp>
#include <gbBase/Log.hpp>
#include <gbBase/LogHandlers.hpp>
#include <gbBase/UnusedVariable.hpp>

#include <gbMath/Vector2.hpp>
#include <gbMath/Vector3.hpp>
#include <gbMath/Matrix4.hpp>

#include <gbVk/Buffer.hpp>
#include <gbVk/CommandBuffer.hpp>
#include <gbVk/CommandBuffers.hpp>
#include <gbVk/CommandPool.hpp>
#include <gbVk/DescriptorSetLayout.hpp>
#include <gbVk/DescriptorSetLayoutBuilder.hpp>
#include <gbVk/Device.hpp>
#include <gbVk/DeviceBuilder.hpp>
#include <gbVk/DeviceMemory.hpp>
#include <gbVk/Fence.hpp>
#include <gbVk/Framebuffer.hpp>
#include <gbVk/Image.hpp>
#include <gbVk/ImageView.hpp>
#include <gbVk/Instance.hpp>
#include <gbVk/PhysicalDevice.hpp>
#include <gbVk/Pipeline.hpp>
#include <gbVk/PipelineBuilder.hpp>
#include <gbVk/PipelineLayout.hpp>
#include <gbVk/PipelineLayoutBuilder.hpp>
#include <gbVk/RenderPass.hpp>
#include <gbVk/RenderPassBuilder.hpp>
#include <gbVk/Semaphore.hpp>
#include <gbVk/ShaderModule.hpp>
#include <gbVk/Spirv.hpp>
#include <gbVk/StringConverters.hpp>
#include <gbVk/Swapchain.hpp>

#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <external/stb_image.h>

#include <algorithm>
#include <cstring>
#include <memory>
#include <vector>

struct DemoState {};

struct Vertex {
    GhulbusMath::Vector2f position;
    GhulbusMath::Vector3f color;
};

struct UBOMVP {
    GhulbusMath::Matrix4<float> model;
    GhulbusMath::Matrix4<float> view;
    GhulbusMath::Matrix4<float> projection;
};

inline std::vector<Vertex> generateVertexData()
{
    return { {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
             {{ 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
             {{ 0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
             {{-0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}} };
}

inline std::vector<uint16_t> generateIndexData() {
    return { 0, 1, 2, 2, 3, 0 };
}

inline UBOMVP generateMVP()
{
    return {};
}

enum class DrawMode {
    Hardcoded,
    Direct,
    Indexed
};

int main()
{
    DrawMode const draw_mode = DrawMode::Direct;

    Ghulbus::Log::initializeLogging();
    auto const gblog_init_guard = Ghulbus::finally([]() { Ghulbus::Log::shutdownLogging(); });
    Ghulbus::Log::Handlers::LogSynchronizeMutex logger(Ghulbus::Log::Handlers::logToCout);
    Ghulbus::Log::setLogHandler(logger);
    Ghulbus::Log::setLogLevel(Ghulbus::LogLevel::Trace);

    if(!glfwInit())
    {
        return 1;
    }
    auto const glfw_init_guard = Ghulbus::finally([]() { glfwTerminate(); });
    glfwSetErrorCallback([](int ec, char const* msg) { GHULBUS_LOG(Error, "GLFW Error " << ec << " - " << msg); });
    if(!glfwVulkanSupported()) {
        GHULBUS_LOG(Error, "No Vulkan support in GLFW.");
        return 1;
    }
    GHULBUS_LOG(Trace, "GLFW " << glfwGetVersionString() << " up and running.");

    GhulbusVulkan::Instance instance = GhulbusVulkan::Instance::createInstance();

    auto phys_devices = instance.enumeratePhysicalDevices();
    auto& physical_device = phys_devices.front();
    auto dev_props = physical_device.getProperties();
    GHULBUS_LOG(Info, "Using device " << dev_props.deviceName << " ("
                      << "Vulkan Version " << GhulbusVulkan::version_to_string(dev_props.apiVersion) << ", "
                      << "Driver Version " << GhulbusVulkan::version_to_string(dev_props.driverVersion)
                      << ").");

    glfwWindowHint(GLFW_RESIZABLE, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    int const WINDOW_WIDTH = 1280;
    int const WINDOW_HEIGHT = 720;
    auto main_window =
        std::unique_ptr<GLFWwindow, void(*)(GLFWwindow*)>(glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan Demo",
            nullptr, nullptr),
            glfwDestroyWindow);
    if (!main_window) {
        return 1;
    }
    VkSurfaceKHR surface;
    VkResult res = glfwCreateWindowSurface(instance.getVkInstance(), main_window.get(), nullptr, &surface);
    if (res != VK_SUCCESS) {
        GHULBUS_LOG(Error, "Unable to create Vulkan surface.");
    }
    auto guard_surface = Ghulbus::finally([&instance, surface]() { vkDestroySurfaceKHR(instance.getVkInstance(), surface, nullptr); });


    // find queue family
    auto const queue_family_properties = physical_device.getQueueFamilyProperties();
    auto const opt_queue_family = [&physical_device, &surface, &queue_family_properties]() -> std::optional<uint32_t> {
        uint32_t i = 0;
        for (auto const& qfp : queue_family_properties) {
            if ((qfp.queueCount > 0) && (qfp.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                // @todo: graphics and presentation might only be available on separate queues
                if (physical_device.getSurfaceSupport(i, surface)) {
                    return i;
                }
            }
            ++i;
        }
        GHULBUS_LOG(Error, "No suitable queue family found.");
        return std::nullopt;
    }();
    if (!opt_queue_family) {
        return 1;
    }

    uint32_t const queue_family = *opt_queue_family;

    uint32_t const transfer_queue_family = [&physical_device, queue_family, &queue_family_properties]() {
        uint32_t i = 0;
        for (auto const& qfp : queue_family_properties) {
            if ((qfp.queueCount > 0) &&
                (qfp.queueFlags & VK_QUEUE_TRANSFER_BIT) &&
                ((qfp.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)) {
                return i;
            }
            ++i;
        }
        return queue_family;
    }();

    GhulbusVulkan::Device device = [&physical_device, queue_family, transfer_queue_family,
                                    &queue_family_properties]()
    {
        GhulbusVulkan::DeviceBuilder device_builder = physical_device.createDeviceBuilder();
        if (queue_family == transfer_queue_family) {
            device_builder.addQueue(queue_family, std::min(queue_family_properties[queue_family].queueCount, 2u));
        } else {
            device_builder.addQueue(queue_family, 1);
            device_builder.addQueue(transfer_queue_family, 1);
        }
        return device_builder.create();
    }();

    DemoState state;
    glfwSetWindowUserPointer(main_window.get(), &state);

    glfwSetKeyCallback(main_window.get(),
        [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            GHULBUS_UNUSED_VARIABLE(scancode);
            GHULBUS_UNUSED_VARIABLE(action);
            GHULBUS_UNUSED_VARIABLE(mods);
            if(key == GLFW_KEY_ESCAPE) {
                glfwSetWindowShouldClose(window, true);
            }
        });

    GhulbusVulkan::DeviceMemory memory = device.allocateMemory(1024*1024*64, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    GhulbusVulkan::DeviceMemory host_memory = device.allocateMemory(1024*1024*64, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    {
        auto mapped = host_memory.map();
        for(int i=0; i<1024*1024*64; ++i) { mapped[i] = std::byte(i & 0xff); }
        mapped.flush();
    }
    {
        auto mapped_again = host_memory.map();
        mapped_again.invalidate();
        for(int i=0; i<1024; ++i) {
            /*
            GHULBUS_LOG(Info, i << " - " << std::to_integer<int>(
                static_cast<GhulbusVulkan::DeviceMemory::MappedMemory const&>(mapped_again)[i]));
             */
        }
    }

    auto swapchain = device.createSwapChain(surface, queue_family);

    auto command_pool = device.createCommandPool(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
                                                 VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, queue_family);
    auto command_buffers = command_pool.allocateCommandBuffers(1);
    auto command_buffer = command_buffers.getCommandBuffer(0);

    auto transfer_command_pool = device.createCommandPool(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
                                                          VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                                                          transfer_queue_family);

    command_buffer.begin();
    VkBufferCopy region;
    region.srcOffset = 0;
    region.dstOffset = 0;
    region.size = 1024*1024*64;
    //vkCmdCopyBuffer(command_buffer.getVkCommandBuffer(), host_memory, memory, 1, & region);
    command_buffer.end();

    auto queue = device.getQueue(queue_family, 0);
    command_buffer.submit(queue);
    vkQueueWaitIdle(queue);
    command_buffer.reset();

    auto transfer_queue = device.getQueue(transfer_queue_family, (transfer_queue_family == queue_family) ?
        ((queue_family_properties[queue_family].queueCount > 1) ? 1 : 0) : 0);

    auto fence = device.createFence();
    auto images = swapchain.getVkImages();
    auto swapchain_image = swapchain.acquireNextImage(fence);
    if(!swapchain_image) {
        GHULBUS_LOG(Error, "Unable to acquire image from swap chain.");
        return 1;
    }
    fence.wait();

    command_buffer.begin();

    auto source_image = device.createImage(WINDOW_WIDTH, WINDOW_HEIGHT);
    auto mem_reqs = source_image.getMemoryRequirements();
    auto source_image_memory = device.allocateMemory(mem_reqs.size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, mem_reqs);
    source_image.bindMemory(source_image_memory, 0);

    // create image view
    /*
    VkImageView image_view;
    VkImageViewCreateInfo color_image_view = {};
    color_image_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    color_image_view.pNext = nullptr;
    color_image_view.format = VK_FORMAT_B8G8R8A8_UNORM;
    color_image_view.components.r = VK_COMPONENT_SWIZZLE_R;
    color_image_view.components.g = VK_COMPONENT_SWIZZLE_G;
    color_image_view.components.b = VK_COMPONENT_SWIZZLE_B;
    color_image_view.components.a = VK_COMPONENT_SWIZZLE_A;
    color_image_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_image_view.subresourceRange.baseMipLevel = 0;
    color_image_view.subresourceRange.levelCount = 1;
    color_image_view.subresourceRange.baseArrayLayer = 0;
    color_image_view.subresourceRange.layerCount = 1;
    color_image_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    color_image_view.flags = 0;
    color_image_view.image = swapchain_image->getVkImage();

    res = vkCreateImageView(device.getVkDevice(), &color_image_view, NULL, &image_view);
    if(res != VK_SUCCESS) {
        GHULBUS_LOG(Error, "Unable to create image view.");
    }
    auto guard_image_view = Ghulbus::finally([&device, &image_view]() { vkDestroyImageView(device.getVkDevice(), image_view, nullptr); });
    */


    // fill host image
    source_image.transition(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_HOST_BIT,
                            VK_ACCESS_HOST_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL);
    {
        struct ImageDim { int x,y,comp; } dim;
        auto img_data = stbi_load("image.jpg", &dim.x, &dim.y, &dim.comp, 0);
        auto mapped = source_image_memory.map();
        for(int i=0; i<mem_reqs.size/4; ++i) {
            int ix = i % WINDOW_WIDTH;
            int iy = i / WINDOW_WIDTH;
            if(!img_data || ix >= dim.x || iy >= dim.y || (dim.comp != 3 && dim.comp != 4)) {
                mapped[i * 4] = std::byte(255);           //R
                mapped[i * 4 + 1] = std::byte(0);         //G
                mapped[i * 4 + 2] = std::byte(0);         //B
                mapped[i * 4 + 3] = std::byte(255);       //A
            } else {
                auto const pixel_index = (iy * dim.x + ix) * dim.comp;
                mapped[i * 4] = std::byte(img_data[pixel_index]);           //R
                mapped[i * 4 + 1] = std::byte(img_data[pixel_index + 1]);         //G
                mapped[i * 4 + 2] = std::byte(img_data[pixel_index + 2]);         //B
                mapped[i * 4 + 3] = std::byte((dim.comp == 4) ? img_data[pixel_index + 3] : 255);       //A
            }
        }
        mapped.flush();
        stbi_image_free(img_data);
    }

    // copy
    source_image.transition(command_buffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                            VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    swapchain_image->transition(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    GhulbusVulkan::Image::blit(command_buffer, source_image, *swapchain_image);

    // presentation
    swapchain_image->transition(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                VK_ACCESS_MEMORY_READ_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    command_buffer.end();
    command_buffer.submit(queue);
    command_buffer.reset();

    swapchain.present(queue, std::move(swapchain_image));
    vkQueueWaitIdle(queue);

    std::vector<Vertex> vertex_data = generateVertexData();
    std::vector<uint16_t> index_data = generateIndexData();
    VkVertexInputBindingDescription vertex_binding;
    vertex_binding.binding = 0;
    vertex_binding.stride = sizeof(Vertex);
    vertex_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription vertex_attributes[2];
    vertex_attributes[0].location = 0;
    vertex_attributes[0].binding = vertex_binding.binding;
    vertex_attributes[0].format = VK_FORMAT_R32G32_SFLOAT;
    vertex_attributes[0].offset = offsetof(Vertex, position);

    vertex_attributes[1].location = 1;
    vertex_attributes[1].binding = vertex_binding.binding;
    vertex_attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertex_attributes[1].offset = offsetof(Vertex, color);

    GhulbusVulkan::Buffer staging_buffer = device.createBuffer(vertex_data.size() * sizeof(Vertex),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    auto const staging_buffer_mem_reqs = staging_buffer.getMemoryRequirements();
    GhulbusVulkan::DeviceMemory staging_memory = device.allocateMemory(staging_buffer_mem_reqs.size,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        staging_buffer_mem_reqs);
    staging_buffer.bindBufferMemory(staging_memory);

    GhulbusVulkan::Buffer vertex_buffer = device.createBuffer(vertex_data.size() * sizeof(Vertex),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    auto const vertex_buffer_mem_reqs = vertex_buffer.getMemoryRequirements();
    GhulbusVulkan::DeviceMemory vertex_memory =
        device.allocateMemory(vertex_buffer_mem_reqs.size,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            vertex_buffer_mem_reqs);
    vertex_buffer.bindBufferMemory(vertex_memory);

    // copy vertex buffer
    {
        auto mapped_memory = staging_memory.map();
        std::memcpy(mapped_memory, vertex_data.data(), vertex_data.size() * sizeof(Vertex));
    }
    {
        auto transfer_command_buffers = transfer_command_pool.allocateCommandBuffers(1);
        auto transfer_command_buffer = transfer_command_buffers.getCommandBuffer(0);

        transfer_command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        VkBufferCopy buffer_copy;
        buffer_copy.srcOffset = 0;
        buffer_copy.dstOffset = 0;
        buffer_copy.size = vertex_data.size() * sizeof(Vertex);

        vkCmdCopyBuffer(transfer_command_buffer.getVkCommandBuffer(), staging_buffer.getVkBuffer(),
                        vertex_buffer.getVkBuffer(), 1, &buffer_copy);

        VkBufferMemoryBarrier barrier;
        barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        barrier.pNext = nullptr;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
        barrier.srcQueueFamilyIndex = transfer_queue_family;
        barrier.dstQueueFamilyIndex = queue_family;
        barrier.buffer = vertex_buffer.getVkBuffer();
        barrier.offset = 0;
        barrier.size = VK_WHOLE_SIZE;
        vkCmdPipelineBarrier(transfer_command_buffer.getVkCommandBuffer(),
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_DEPENDENCY_BY_REGION_BIT,
            0, nullptr, 1, &barrier, 0, nullptr);

        transfer_command_buffer.end();

        transfer_command_buffer.submit(transfer_queue);
    }


    GhulbusVulkan::Buffer index_staging_buffer = device.createBuffer(index_data.size() * sizeof(uint16_t),
                                                                     VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    auto const index_staging_buffer_mem_reqs = index_staging_buffer.getMemoryRequirements();
    GhulbusVulkan::DeviceMemory index_staging_memory = device.allocateMemory(index_staging_buffer_mem_reqs.size,
                                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                        index_staging_buffer_mem_reqs);
    index_staging_buffer.bindBufferMemory(index_staging_memory);

    GhulbusVulkan::Buffer index_buffer = device.createBuffer(index_data.size() * sizeof(uint16_t),
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    auto const index_buffer_mem_reqs = vertex_buffer.getMemoryRequirements();
    GhulbusVulkan::DeviceMemory index_memory =
        device.allocateMemory(index_buffer_mem_reqs.size,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            index_buffer_mem_reqs);
    index_buffer.bindBufferMemory(index_memory);

    // copy index buffer
    {
        auto mapped_memory = index_staging_memory.map();
        std::memcpy(mapped_memory, index_data.data(), index_data.size() * sizeof(uint16_t));
    }
    {
        auto transfer_command_buffers = transfer_command_pool.allocateCommandBuffers(1);
        auto transfer_command_buffer = transfer_command_buffers.getCommandBuffer(0);

        transfer_command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        VkBufferCopy buffer_copy;
        buffer_copy.srcOffset = 0;
        buffer_copy.dstOffset = 0;
        buffer_copy.size = index_data.size() * sizeof(uint16_t);

        vkCmdCopyBuffer(transfer_command_buffer.getVkCommandBuffer(), index_staging_buffer.getVkBuffer(),
                        index_buffer.getVkBuffer(), 1, &buffer_copy);

        VkBufferMemoryBarrier barrier;
        barrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        barrier.pNext = nullptr;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_INDEX_READ_BIT;
        barrier.srcQueueFamilyIndex = transfer_queue_family;
        barrier.dstQueueFamilyIndex = queue_family;
        barrier.buffer = index_buffer.getVkBuffer();
        barrier.offset = 0;
        barrier.size = VK_WHOLE_SIZE;
        vkCmdPipelineBarrier(transfer_command_buffer.getVkCommandBuffer(),
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                             VK_DEPENDENCY_BY_REGION_BIT,
                             0, nullptr, 1, &barrier, 0, nullptr);

        transfer_command_buffer.end();

        transfer_command_buffer.submit(transfer_queue);
    }


    auto spirv_code = GhulbusVulkan::Spirv::load("shaders/simple_compute.spv");
    auto version = spirv_code.getSpirvVersion();
    auto bound = spirv_code.getBound();
    GHULBUS_UNUSED_VARIABLE(version);
    GHULBUS_UNUSED_VARIABLE(bound);
    auto shader_module = device.createShaderModule(spirv_code);

    auto vert_spirv_code = GhulbusVulkan::Spirv::load("shaders/vert_hardcoded.spv");
    auto vert_hardcoded_shader_module = device.createShaderModule(vert_spirv_code);

    auto vert_direct_spirv_code = GhulbusVulkan::Spirv::load("shaders/vert_direct.spv");
    auto vert_direct_shader_module = device.createShaderModule(vert_direct_spirv_code);
    VkPipelineShaderStageCreateInfo vert_shader_stage_ci;
    vert_shader_stage_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_ci.pNext = nullptr;
    vert_shader_stage_ci.flags = 0;
    vert_shader_stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    if constexpr (draw_mode == DrawMode::Hardcoded) {
        vert_shader_stage_ci.module = vert_hardcoded_shader_module.getVkShaderModule();
    } else if constexpr (draw_mode == DrawMode::Direct) {
        vert_shader_stage_ci.module = vert_direct_shader_module.getVkShaderModule();
    }
    vert_shader_stage_ci.pName = "main";
    vert_shader_stage_ci.pSpecializationInfo = nullptr;

    auto frag_spirv_code = GhulbusVulkan::Spirv::load("shaders/frag_hardcoded.spv");
    auto frag_shader_module = device.createShaderModule(frag_spirv_code);
    VkPipelineShaderStageCreateInfo frag_shader_stage_ci;
    frag_shader_stage_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_ci.pNext = nullptr;
    frag_shader_stage_ci.flags = 0;
    frag_shader_stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_ci.module = frag_shader_module.getVkShaderModule();
    frag_shader_stage_ci.pName = "main";
    frag_shader_stage_ci.pSpecializationInfo = nullptr;

    std::vector<VkPipelineShaderStageCreateInfo> shader_stage_cis{ vert_shader_stage_ci, frag_shader_stage_ci };

    // render pass
    GhulbusVulkan::RenderPass render_pass = [&device, &swapchain_image]() {
            auto builder = device.createRenderPassBuilder();
            builder.addSubpassGraphics();
            builder.addColorAttachment(swapchain_image->getFormat());
            return builder.create();
        }();

    // ubo
    auto const ubo_data = generateMVP();
    GhulbusVulkan::DescriptorSetLayout ubo_layout = [&device]() {
        GhulbusVulkan::DescriptorSetLayoutBuilder layout_builder = device.createDescriptorSetLayoutBuilder();
        layout_builder.addUniformBuffer(0, VK_SHADER_STAGE_VERTEX_BIT);
        return layout_builder.create();
    }();

    // pipeline
    GhulbusVulkan::PipelineLayout pipeline_layout = [&device, &ubo_layout]() {
        GhulbusVulkan::PipelineLayoutBuilder builder = device.createPipelineLayoutBuilder();
        builder.addDescriptorSetLayout(ubo_layout);
        return builder.create();
    }();

    GhulbusVulkan::Pipeline pipeline = [&device, &swapchain_image, &pipeline_layout,
                                        &shader_stage_cis, &render_pass,
                                        &vertex_binding, &vertex_attributes]() {
        auto builder = device.createGraphicsPipelineBuilder(swapchain_image->getWidth(),
                                                            swapchain_image->getHeight());
        builder.addVertexBindings(&vertex_binding, 1, vertex_attributes, 2);
        return builder.create(pipeline_layout, shader_stage_cis.data(),
                              static_cast<std::uint32_t>(shader_stage_cis.size()),
                              render_pass.getVkRenderPass());
    }();


    std::vector<GhulbusVulkan::Framebuffer> framebuffers = device.createFramebuffers(swapchain, render_pass);

    GhulbusVulkan::CommandBuffers triangle_draw_command_buffers =
        command_pool.allocateCommandBuffers(swapchain.getNumberOfImages());

    for(uint32_t i = 0; i < triangle_draw_command_buffers.size(); ++i) {
        auto local_command_buffer = triangle_draw_command_buffers.getCommandBuffer(i);
        local_command_buffer.begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

        VkRenderPassBeginInfo render_pass_info;
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.pNext = nullptr;
        render_pass_info.renderPass = render_pass.getVkRenderPass();
        render_pass_info.framebuffer = framebuffers[i].getVkFramebuffer();
        render_pass_info.renderArea.offset.x = 0;
        render_pass_info.renderArea.offset.y = 0;
        render_pass_info.renderArea.extent.width = swapchain_image->getWidth();
        render_pass_info.renderArea.extent.height = swapchain_image->getHeight();
        VkClearValue clear_color;
        clear_color.color.float32[0] = 0.5f;
        clear_color.color.float32[1] = 0.f;
        clear_color.color.float32[2] = 0.5f;
        clear_color.color.float32[3] = 1.f;
        render_pass_info.clearValueCount = 1;
        render_pass_info.pClearValues = &clear_color;

        vkCmdBeginRenderPass(local_command_buffer.getVkCommandBuffer(),
                             &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(local_command_buffer.getVkCommandBuffer(),
                          VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getVkPipeline());

        if constexpr (draw_mode == DrawMode::Hardcoded) {
            vkCmdDraw(local_command_buffer.getVkCommandBuffer(), 3, 1, 0, 0);
        } else if constexpr (draw_mode == DrawMode::Direct) {
            VkBuffer vertexBuffers[] = { vertex_buffer.getVkBuffer() };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(local_command_buffer.getVkCommandBuffer(), 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(local_command_buffer.getVkCommandBuffer(), index_buffer.getVkBuffer(),
                                 0, VK_INDEX_TYPE_UINT16);
            vkCmdDrawIndexed(local_command_buffer.getVkCommandBuffer(), static_cast<uint32_t>(index_data.size()),
                             1, 0, 0, 0);
        }

        vkCmdEndRenderPass(local_command_buffer.getVkCommandBuffer());
        local_command_buffer.end();
    }

    GhulbusVulkan::Semaphore semaphore_image_available = device.createSemaphore();
    GhulbusVulkan::Semaphore semaphore_render_finished = device.createSemaphore();

    GHULBUS_LOG(Trace, "Entering main loop...");
    while(!glfwWindowShouldClose(main_window.get())) {
        glfwPollEvents();

        auto frame_image = swapchain.acquireNextImage(semaphore_image_available);
        VkSubmitInfo submit_info;
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pNext = nullptr;
        VkSemaphore wait_semaphores[] = { semaphore_image_available.getVkSemaphore() };
        VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = wait_semaphores;
        submit_info.pWaitDstStageMask = wait_stages;
        submit_info.commandBufferCount = 1;
        VkCommandBuffer cb = triangle_draw_command_buffers.getCommandBuffer(frame_image.getSwapchainIndex()).getVkCommandBuffer();
        submit_info.pCommandBuffers = &cb;
        VkSemaphore signal_semaphores[] = { semaphore_render_finished.getVkSemaphore() };
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = signal_semaphores;
        res = vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
        if(res != VK_SUCCESS) { GHULBUS_LOG(Error, "Error in vkQueueSubmit: " << res); return 1; }
        swapchain.present(queue, semaphore_render_finished, std::move(frame_image));
        vkQueueWaitIdle(queue);
    }
}
