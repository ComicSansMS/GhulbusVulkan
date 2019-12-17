
#include <gbBase/Finally.hpp>
#include <gbBase/Log.hpp>
#include <gbBase/LogHandlers.hpp>
#include <gbBase/UnusedVariable.hpp>

#include <gbMath/Matrix4.hpp>
#include <gbMath/Transform3.hpp>
#include <gbMath/Vector2.hpp>
#include <gbMath/Vector3.hpp>

#include <gbGraphics/CommandPoolRegistry.hpp>
#include <gbGraphics/Graphics.hpp>
#include <gbGraphics/Image2d.hpp>
#include <gbGraphics/MemoryBuffer.hpp>
#include <gbGraphics/Window.hpp>

#include <gbVk/Buffer.hpp>
#include <gbVk/CommandBuffer.hpp>
#include <gbVk/CommandBuffers.hpp>
#include <gbVk/CommandPool.hpp>
#include <gbVk/DebugReportCallback.hpp>
#include <gbVk/DescriptorPool.hpp>
#include <gbVk/DescriptorPoolBuilder.hpp>
#include <gbVk/DescriptorSet.hpp>
#include <gbVk/DescriptorSetLayout.hpp>
#include <gbVk/DescriptorSetLayoutBuilder.hpp>
#include <gbVk/DescriptorSets.hpp>
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
#include <gbVk/Queue.hpp>
#include <gbVk/RenderPass.hpp>
#include <gbVk/RenderPassBuilder.hpp>
#include <gbVk/Sampler.hpp>
#include <gbVk/Semaphore.hpp>
#include <gbVk/ShaderModule.hpp>
#include <gbVk/Spirv.hpp>
#include <gbVk/StringConverters.hpp>
#include <gbVk/SubmitStaging.hpp>
#include <gbVk/Swapchain.hpp>

#include <gbBase/PerfLog.hpp>

#include <external/stb_image.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <memory>
#include <vector>

void drawToBackbuffer(GhulbusGraphics::GraphicsInstance& graphics_instance, GhulbusGraphics::Window& main_window)
{
    auto command_buffers = graphics_instance.getCommandPoolRegistry().allocateCommandBuffersGraphics_Transient(1);
    auto command_buffer = command_buffers.getCommandBuffer(0);

    auto& swapchain_image = main_window.getAcquiredImage();

    command_buffer.begin();

    GhulbusGraphics::Image2d source_image(graphics_instance, main_window.getWidth(), main_window.getHeight(),
        VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        GhulbusGraphics::MemoryUsage::CpuOnly);

    // fill host image
    //source_image.getImage().transition(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_HOST_BIT,
    //                                   VK_ACCESS_HOST_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL);
    {
        struct ImageDim { int x, y, comp; } dim;
        auto mapped = source_image.map();
        auto img_data = stbi_load("textures/statue.jpg", &dim.x, &dim.y, &dim.comp, 0);
        for (int iy = 0; iy < main_window.getHeight(); ++iy) {
            for (int ix = 0; ix < main_window.getWidth(); ++ix) {
                int const i = iy * main_window.getWidth() + ix;
                if (!img_data || ix >= dim.x || iy >= dim.y || (dim.comp != 3 && dim.comp != 4)) {
                    mapped[i * 4] = std::byte(255);           //R
                    mapped[i * 4 + 1] = std::byte(0);         //G
                    mapped[i * 4 + 2] = std::byte(0);         //B
                    mapped[i * 4 + 3] = std::byte(255);       //A
                } else {
                    auto const pixel_index = (iy * dim.x + ix) * dim.comp;
                    mapped[i * 4] = std::byte(img_data[pixel_index]);                                       //R
                    mapped[i * 4 + 1] = std::byte(img_data[pixel_index + 1]);                               //G
                    mapped[i * 4 + 2] = std::byte(img_data[pixel_index + 2]);                               //B
                    mapped[i * 4 + 3] = std::byte((dim.comp == 4) ? img_data[pixel_index + 3] : 255);       //A
                }
            }
        }
        stbi_image_free(img_data);
    }

    // copy
    source_image.getImage().transition(command_buffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    swapchain_image->transition(command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    GhulbusVulkan::Image::blit(command_buffer, source_image.getImage(), *swapchain_image);

    swapchain_image->transition(command_buffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        VK_ACCESS_MEMORY_READ_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    command_buffer.end();

    GhulbusVulkan::SubmitStaging submission;
    submission.addCommandBuffers(command_buffers);
    submission.adoptResources(std::move(source_image), std::move(command_buffers));
    graphics_instance.getGraphicsQueue().stageSubmission(std::move(submission));
}

void fullBarrier(GhulbusVulkan::CommandBuffer& command_buffer)
{
    VkMemoryBarrier memoryBarrier;
    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memoryBarrier.pNext = nullptr;
    memoryBarrier.srcAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT |
        VK_ACCESS_INDEX_READ_BIT |
        VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
        VK_ACCESS_UNIFORM_READ_BIT |
        VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |
        VK_ACCESS_SHADER_READ_BIT |
        VK_ACCESS_SHADER_WRITE_BIT |
        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_TRANSFER_READ_BIT |
        VK_ACCESS_TRANSFER_WRITE_BIT |
        VK_ACCESS_HOST_READ_BIT |
        VK_ACCESS_HOST_WRITE_BIT;
    memoryBarrier.dstAccessMask = VK_ACCESS_INDIRECT_COMMAND_READ_BIT |
        VK_ACCESS_INDEX_READ_BIT |
        VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
        VK_ACCESS_UNIFORM_READ_BIT |
        VK_ACCESS_INPUT_ATTACHMENT_READ_BIT |
        VK_ACCESS_SHADER_READ_BIT |
        VK_ACCESS_SHADER_WRITE_BIT |
        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_TRANSFER_READ_BIT |
        VK_ACCESS_TRANSFER_WRITE_BIT |
        VK_ACCESS_HOST_READ_BIT |
        VK_ACCESS_HOST_WRITE_BIT;

    vkCmdPipelineBarrier(command_buffer.getVkCommandBuffer(),
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
        1,                                  // memoryBarrierCount
        &memoryBarrier,                     // pMemoryBarriers
        0, nullptr, 0, nullptr);
}

struct Vertex {
    GhulbusMath::Vector3f position;
    GhulbusMath::Vector3f color;
    GhulbusMath::Vector2f texCoords;
};

struct UBOMVP {
    GhulbusMath::Matrix4<float> model;
    GhulbusMath::Matrix4<float> view;
    GhulbusMath::Matrix4<float> projection;
};

inline std::vector<Vertex> generateVertexData()
{
    return {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

        {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
    };
}

inline std::vector<uint16_t> generateIndexData() {
    return { 0, 1, 2, 2, 3, 0,
             4, 5, 6, 6, 7, 4 };
}

enum class DrawMode {
    Hardcoded,
    Direct,
    UniformTransform,
    Textured
};

int main()
{
    DrawMode const draw_mode = DrawMode::Textured;

    auto const gblog_init_guard = Ghulbus::Log::initializeLoggingWithGuard();
    Ghulbus::Log::Handlers::LogSynchronizeMutex logger(Ghulbus::Log::Handlers::logToCout);
    Ghulbus::Log::setLogHandler(logger);
    Ghulbus::Log::setLogLevel(Ghulbus::LogLevel::Trace);

    Ghulbus::PerfLog perflog;

    GhulbusGraphics::GraphicsInstance graphics_instance;
    //graphics_instance.setDebugLoggingEnabled(false);

    perflog.tick(Ghulbus::LogLevel::Debug, "gbGraphics Init");

    GhulbusVulkan::Device& device = graphics_instance.getVulkanDevice();
    GhulbusVulkan::PhysicalDevice physical_device = graphics_instance.getVulkanPhysicalDevice();

    auto const queue_family_properties = physical_device.getQueueFamilyProperties();
    auto dev_props = physical_device.getProperties();

    int const WINDOW_WIDTH = 1280;
    int const WINDOW_HEIGHT = 720;

    GhulbusGraphics::Window main_window(graphics_instance, WINDOW_WIDTH, WINDOW_HEIGHT, u8"Vulkan Demo");

    perflog.tick(Ghulbus::LogLevel::Debug, "Window creation");

    auto& swapchain_image = main_window.getAcquiredImage();

    perflog.tick(Ghulbus::LogLevel::Debug, "Swapchain setup");

    drawToBackbuffer(graphics_instance, main_window);

    // presentation
    main_window.present();

    graphics_instance.getGraphicsQueue().clearAllStaged();

    //std::this_thread::sleep_for(std::chrono::seconds(5));
    //return 0;

    perflog.tick(Ghulbus::LogLevel::Debug, "Initial present");

    std::vector<Vertex> const vertex_data = generateVertexData();
    std::vector<uint16_t> const index_data = generateIndexData();
    VkVertexInputBindingDescription vertex_binding;
    vertex_binding.binding = 0;
    vertex_binding.stride = sizeof(Vertex);
    vertex_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 3> vertex_attributes;
    vertex_attributes[0].location = 0;
    vertex_attributes[0].binding = vertex_binding.binding;
    vertex_attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertex_attributes[0].offset = offsetof(Vertex, position);

    vertex_attributes[1].location = 1;
    vertex_attributes[1].binding = vertex_binding.binding;
    vertex_attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertex_attributes[1].offset = offsetof(Vertex, color);

    vertex_attributes[2].location = 2;
    vertex_attributes[2].binding = vertex_binding.binding;
    vertex_attributes[2].format = VK_FORMAT_R32G32_SFLOAT;
    vertex_attributes[2].offset = offsetof(Vertex, texCoords);

    GhulbusGraphics::MemoryBuffer staging_buffer(graphics_instance, vertex_data.size() * sizeof(Vertex),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, GhulbusGraphics::MemoryUsage::CpuOnly);

    GhulbusGraphics::MemoryBuffer vertex_buffer(graphics_instance, vertex_data.size() * sizeof(Vertex),
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, GhulbusGraphics::MemoryUsage::GpuOnly);
    graphics_instance.getTransferQueue().stageSubmission(
        vertex_buffer.setDataAsynchronously(reinterpret_cast<std::byte const*>(vertex_data.data()),
                                            graphics_instance.getGraphicsQueueFamilyIndex()));

    GhulbusGraphics::MemoryBuffer index_buffer(graphics_instance, index_data.size() * sizeof(uint16_t),
                                               VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                               GhulbusGraphics::MemoryUsage::GpuOnly);
    graphics_instance.getTransferQueue().stageSubmission(
        index_buffer.setDataAsynchronously(reinterpret_cast<std::byte const*>(index_data.data()),
                                           graphics_instance.getGraphicsQueueFamilyIndex()));


    // ubo
    std::vector<GhulbusGraphics::MemoryBuffer> ubo_buffers;
    uint32_t const swapchain_n_images = main_window.getNumberOfImagesInSwapchain();
    ubo_buffers.reserve(swapchain_n_images);
    for (uint32_t i = 0; i < swapchain_n_images; ++i)
    {
        ubo_buffers.emplace_back(graphics_instance, sizeof(UBOMVP), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                 GhulbusGraphics::MemoryUsage::CpuOnly);
    }

    auto timestamp = std::chrono::steady_clock::now();
    UBOMVP ubo_data;
    auto update_uniform_buffer = [&timestamp, &ubo_data, &ubo_buffers, WINDOW_WIDTH, WINDOW_HEIGHT](uint32_t index) {
        auto const t = std::chrono::steady_clock::now();
        float const time = std::chrono::duration<float>(t - timestamp).count();
        ubo_data.model = GhulbusMath::make_rotation((GhulbusMath::traits::Pi<float>::value / 2.f) * time,
            GhulbusMath::Vector3f(0.f, 0.f, 1.f)).m;
        ubo_data.view = GhulbusMath::make_view_look_at(GhulbusMath::Vector3f(2.0f, 2.0f, 2.0f),
            GhulbusMath::Vector3f(0.0f, 0.0f, 0.0f),
            GhulbusMath::Vector3f(0.0f, 0.0f, 1.0f)).m;
        ubo_data.projection = GhulbusMath::make_perspective_projection_fov(
            (GhulbusMath::traits::Pi<float>::value / 4.f),
            static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT), 0.1f, 10.f).m;

        // correct for vulkan screen coordinate origin being upper left, instead of lower left
        ubo_data.projection.m22 *= -1.f;

        // correct memory layout to column-major
        ubo_data.model = GhulbusMath::transpose(ubo_data.model);
        ubo_data.view = GhulbusMath::transpose(ubo_data.view);
        ubo_data.projection = GhulbusMath::transpose(ubo_data.projection);
        {
            auto mapped_mem = ubo_buffers[index].map();
            std::memcpy(mapped_mem, &ubo_data, sizeof(UBOMVP));
        }
    };


    // texture
    int texture_dimensions_width, texture_dimensions_height, texture_dimensions_n_channels;
    stbi_uc* texture_data = stbi_load("textures/statue.jpg", &texture_dimensions_width, &texture_dimensions_height,
        &texture_dimensions_n_channels, STBI_rgb_alpha);
    if (!texture_data) { GHULBUS_LOG(Error, "Error loading texture file."); return 1; }
    GhulbusGraphics::Image2d texture(graphics_instance, texture_dimensions_width, texture_dimensions_height);
    graphics_instance.getTransferQueue().stageSubmission(
        texture.setDataAsynchronously(reinterpret_cast<std::byte const*>(texture_data),
                                      graphics_instance.getGraphicsQueueFamilyIndex()));


    auto transfer_fence = device.createFence();
    graphics_instance.getTransferQueue().submitAllStaged(transfer_fence);
    {
        GhulbusVulkan::SubmitStaging graphics_transfer_sync;
        auto sync_command_buffers = graphics_instance.getCommandPoolRegistry().allocateCommandBuffersGraphics_Transient(1);
        auto sync_command_buffer = sync_command_buffers.getCommandBuffer(0);
        sync_command_buffer.begin();
        vertex_buffer.getBuffer().transitionAcquire(sync_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                                    VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT,
                                                    graphics_instance.getTransferQueueFamilyIndex());
        index_buffer.getBuffer().transitionAcquire(sync_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                                   VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_ACCESS_INDEX_READ_BIT,
                                                   graphics_instance.getTransferQueueFamilyIndex());
        texture.getImage().transitionAcquire(sync_command_buffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                             VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, VK_ACCESS_SHADER_READ_BIT,
                                             graphics_instance.getTransferQueueFamilyIndex(),
                                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                             VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        sync_command_buffer.end();
        graphics_transfer_sync.addCommandBuffers(sync_command_buffers);
        graphics_transfer_sync.adoptResources(std::move(sync_command_buffers));
        graphics_instance.getGraphicsQueue().stageSubmission(std::move(graphics_transfer_sync));
    }
    auto graphics_sync_fence = device.createFence();
    graphics_instance.getGraphicsQueue().submitAllStaged(graphics_sync_fence);


    GhulbusVulkan::ImageView texture_image_view = texture.getImage().createImageView();
    GhulbusVulkan::Sampler texture_sampler = device.createSampler();

    // depth buffer attachment
    auto const depth_buffer_opt_format = physical_device.findDepthBufferFormat();
    if (!depth_buffer_opt_format) { GHULBUS_LOG(Error, "No supported depth buffer format found."); return 1; }
    VkFormat const depth_buffer_format = *depth_buffer_opt_format;
    GhulbusVulkan::Image depth_buffer_image =
        device.createImageDepthBuffer(WINDOW_WIDTH, WINDOW_HEIGHT, depth_buffer_format);
    auto const depth_buffer_image_mem_reqs = depth_buffer_image.getMemoryRequirements();
    GhulbusVulkan::DeviceMemory depth_buffer_memory = device.allocateMemory(depth_buffer_image_mem_reqs,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    depth_buffer_memory.bindImage(depth_buffer_image);
    GhulbusVulkan::ImageView depth_buffer_image_view = depth_buffer_image.createImageViewDepthBuffer();

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

    auto vert_mvp_spirv_code = GhulbusVulkan::Spirv::load("shaders/vert_mvp.spv");
    auto vert_mvp_shader_module = device.createShaderModule(vert_mvp_spirv_code);

    auto vert_textured_spirv_code = GhulbusVulkan::Spirv::load("shaders/vert_textured.spv");
    auto vert_textured_shader_module = device.createShaderModule(vert_textured_spirv_code);

    VkPipelineShaderStageCreateInfo vert_shader_stage_ci;
    vert_shader_stage_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_ci.pNext = nullptr;
    vert_shader_stage_ci.flags = 0;
    vert_shader_stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    if constexpr (draw_mode == DrawMode::Textured) {
        vert_shader_stage_ci.module = vert_textured_shader_module.getVkShaderModule();
    } else if constexpr (draw_mode == DrawMode::UniformTransform) {
        vert_shader_stage_ci.module = vert_mvp_shader_module.getVkShaderModule();
    } else if constexpr (draw_mode == DrawMode::Hardcoded) {
        vert_shader_stage_ci.module = vert_hardcoded_shader_module.getVkShaderModule();
    } else if constexpr (draw_mode == DrawMode::Direct) {
        vert_shader_stage_ci.module = vert_direct_shader_module.getVkShaderModule();
    }
    vert_shader_stage_ci.pName = "main";
    vert_shader_stage_ci.pSpecializationInfo = nullptr;

    auto frag_spirv_code = GhulbusVulkan::Spirv::load("shaders/frag_hardcoded.spv");
    auto frag_shader_module = device.createShaderModule(frag_spirv_code);

    auto frag_textured_spirv_code = GhulbusVulkan::Spirv::load("shaders/frag_textured.spv");
    auto frag_textured_shader_module = device.createShaderModule(frag_textured_spirv_code);

    VkPipelineShaderStageCreateInfo frag_shader_stage_ci;
    frag_shader_stage_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_ci.pNext = nullptr;
    frag_shader_stage_ci.flags = 0;
    frag_shader_stage_ci.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    if constexpr (draw_mode == DrawMode::Textured) {
        frag_shader_stage_ci.module = frag_textured_shader_module.getVkShaderModule();
    } else {
        frag_shader_stage_ci.module = frag_shader_module.getVkShaderModule();
    }
    frag_shader_stage_ci.pName = "main";
    frag_shader_stage_ci.pSpecializationInfo = nullptr;

    std::vector<VkPipelineShaderStageCreateInfo> shader_stage_cis{ vert_shader_stage_ci, frag_shader_stage_ci };

    // render pass
    GhulbusVulkan::RenderPass render_pass = [&device, &swapchain_image, &depth_buffer_image]() {
            auto builder = device.createRenderPassBuilder();
            builder.addSubpassGraphics();
            builder.addColorAttachment(swapchain_image->getFormat());
            builder.addDepthStencilAttachment(depth_buffer_image.getFormat());
            return builder.create();
        }();

    // ubo
    GhulbusVulkan::DescriptorSetLayout ubo_layout = [&device]() {
        GhulbusVulkan::DescriptorSetLayoutBuilder layout_builder = device.createDescriptorSetLayoutBuilder();
        layout_builder.addUniformBuffer(0, VK_SHADER_STAGE_VERTEX_BIT);
        layout_builder.addSampler(1, VK_SHADER_STAGE_FRAGMENT_BIT);
        return layout_builder.create();
    }();
    GhulbusVulkan::DescriptorPool ubo_descriptor_pool = [&device, swapchain_n_images]() {
        GhulbusVulkan::DescriptorPoolBuilder descpool_builder = device.createDescriptorPoolBuilder();
        descpool_builder.addDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, swapchain_n_images);
        descpool_builder.addDescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, swapchain_n_images);
        return descpool_builder.create(swapchain_n_images, 0,
                                       GhulbusVulkan::DescriptorPoolBuilder::NoImplicitFreeDescriptorFlag{});
    }();
    GhulbusVulkan::DescriptorSets ubo_descriptor_sets =
        ubo_descriptor_pool.allocateDescriptorSets(swapchain_n_images, ubo_layout);
    for (uint32_t i = 0; i < swapchain_n_images; ++i) {
        VkDescriptorBufferInfo buffer_info;
        buffer_info.buffer = ubo_buffers[i].getBuffer().getVkBuffer();
        buffer_info.offset = 0;
        buffer_info.range = sizeof(UBOMVP);

        std::array<VkWriteDescriptorSet, 2> write_desc_set;
        VkWriteDescriptorSet& ubo_write_desc_set = write_desc_set[0];
        ubo_write_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        ubo_write_desc_set.pNext = nullptr;
        ubo_write_desc_set.dstSet = ubo_descriptor_sets.getDescriptorSet(i).getVkDescriptorSet();
        ubo_write_desc_set.dstBinding = 0;
        ubo_write_desc_set.dstArrayElement = 0;
        ubo_write_desc_set.descriptorCount = 1;
        ubo_write_desc_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ubo_write_desc_set.pImageInfo = nullptr;
        ubo_write_desc_set.pBufferInfo = &buffer_info;
        ubo_write_desc_set.pTexelBufferView = nullptr;

        VkDescriptorImageInfo image_info;
        image_info.sampler = texture_sampler.getVkSampler();
        image_info.imageView = texture_image_view.getVkImageView();
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet& texture_write_desc_set = write_desc_set[1];
        texture_write_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        texture_write_desc_set.pNext = nullptr;
        texture_write_desc_set.dstSet = ubo_descriptor_sets.getDescriptorSet(i).getVkDescriptorSet();
        texture_write_desc_set.dstBinding = 1;
        texture_write_desc_set.dstArrayElement = 0;
        texture_write_desc_set.descriptorCount = 1;
        texture_write_desc_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        texture_write_desc_set.pImageInfo = &image_info;
        texture_write_desc_set.pBufferInfo = nullptr;
        texture_write_desc_set.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(device.getVkDevice(), static_cast<uint32_t>(write_desc_set.size()),
                               write_desc_set.data(), 0, nullptr);
    }


    // pipeline
    GhulbusVulkan::PipelineLayout pipeline_layout = [&device, &ubo_layout]() {
        GhulbusVulkan::PipelineLayoutBuilder builder = device.createPipelineLayoutBuilder();
        builder.addDescriptorSetLayout(ubo_layout);
        return builder.create();
    }();

    GhulbusVulkan::Pipeline pipeline = [&device, &main_window, &pipeline_layout,
                                        &shader_stage_cis, &render_pass,
                                        &vertex_binding, &vertex_attributes, draw_mode]() {
        auto builder = device.createGraphicsPipelineBuilder(main_window.getWidth(),
                                                            main_window.getHeight());
        if (draw_mode != DrawMode::Hardcoded) {
            builder.addVertexBindings(&vertex_binding, 1, vertex_attributes.data(),
                                      static_cast<uint32_t>(vertex_attributes.size()));
        }
        return builder.create(pipeline_layout, shader_stage_cis.data(),
                              static_cast<std::uint32_t>(shader_stage_cis.size()),
                              render_pass.getVkRenderPass());
    }();

    GhulbusVulkan::Swapchain& swapchain = main_window.getSwapchain();
    std::vector<GhulbusVulkan::Framebuffer> framebuffers =
        device.createFramebuffers(swapchain, render_pass, depth_buffer_image_view);

    GhulbusVulkan::CommandBuffers triangle_draw_command_buffers =
        graphics_instance.getCommandPoolRegistry().allocateCommandBuffersGraphics(swapchain_n_images);

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
        render_pass_info.renderArea.extent.width = main_window.getWidth();
        render_pass_info.renderArea.extent.height = main_window.getHeight();
        std::array<VkClearValue, 2> clear_color;
        clear_color[0].color.float32[0] = 0.5f;
        clear_color[0].color.float32[1] = 0.f;
        clear_color[0].color.float32[2] = 0.5f;
        clear_color[0].color.float32[3] = 1.f;
        clear_color[1].depthStencil.depth = 1.0f;
        clear_color[1].depthStencil.stencil = 0;
        render_pass_info.clearValueCount = static_cast<uint32_t>(clear_color.size());
        render_pass_info.pClearValues = clear_color.data();

        vkCmdBeginRenderPass(local_command_buffer.getVkCommandBuffer(),
                             &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(local_command_buffer.getVkCommandBuffer(),
                          VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getVkPipeline());

        if constexpr ((draw_mode == DrawMode::UniformTransform) || (draw_mode == DrawMode::Textured)) {
            VkDescriptorSet desc_set_i = ubo_descriptor_sets.getDescriptorSet(i).getVkDescriptorSet();
            vkCmdBindDescriptorSets(local_command_buffer.getVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipeline_layout.getVkPipelineLayout(), 0, 1, &desc_set_i, 0, nullptr);
        }

        if constexpr (draw_mode == DrawMode::Hardcoded) {
            vkCmdDraw(local_command_buffer.getVkCommandBuffer(), 3, 1, 0, 0);
        } else {
            VkBuffer vertexBuffers[] = { vertex_buffer.getBuffer().getVkBuffer() };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(local_command_buffer.getVkCommandBuffer(), 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(local_command_buffer.getVkCommandBuffer(), index_buffer.getBuffer().getVkBuffer(),
                                 0, VK_INDEX_TYPE_UINT16);
            vkCmdDrawIndexed(local_command_buffer.getVkCommandBuffer(), static_cast<uint32_t>(index_data.size()),
                             1, 0, 0, 0);
        }

        vkCmdEndRenderPass(local_command_buffer.getVkCommandBuffer());
        local_command_buffer.end();
    }

    perflog.tick(Ghulbus::LogLevel::Debug, "Main setup");

    GhulbusVulkan::Semaphore semaphore_render_finished = device.createSemaphore();

    transfer_fence.wait();
    graphics_instance.getTransferQueue().clearAllStaged();
    graphics_sync_fence.wait();
    graphics_instance.getGraphicsQueue().clearAllStaged();
    GHULBUS_LOG(Trace, "Entering main loop...");
    while(!main_window.isDone()) {
        graphics_instance.pollEvents();

        uint32_t frame_image_idx = main_window.getCurrentImageSwapchainIndex();
        update_uniform_buffer(frame_image_idx);
        VkSubmitInfo submit_info;
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.pNext = nullptr;
        VkSemaphore wait_semaphores[] = { main_window.getCurrentImageAcquireSemaphore().getVkSemaphore() };
        VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = wait_semaphores;
        submit_info.pWaitDstStageMask = wait_stages;
        submit_info.commandBufferCount = 1;
        VkCommandBuffer cb = triangle_draw_command_buffers.getCommandBuffer(frame_image_idx).getVkCommandBuffer();
        submit_info.pCommandBuffers = &cb;
        VkSemaphore signal_semaphores[] = { semaphore_render_finished.getVkSemaphore() };
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = signal_semaphores;
        VkResult const res = vkQueueSubmit(graphics_instance.getGraphicsQueue().getVkQueue(), 1, &submit_info, VK_NULL_HANDLE);
        if(res != VK_SUCCESS) { GHULBUS_LOG(Error, "Error in vkQueueSubmit: " << res); return 1; }
        main_window.present(semaphore_render_finished);
        graphics_instance.getGraphicsQueue().waitIdle();
    }
}
