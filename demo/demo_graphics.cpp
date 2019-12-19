
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
#include <gbGraphics/ImageLoader.hpp>
#include <gbGraphics/MemoryBuffer.hpp>
#include <gbGraphics/Mesh.hpp>
#include <gbGraphics/ObjParser.hpp>
#include <gbGraphics/Program.hpp>
#include <gbGraphics/Renderer.hpp>
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
#include <gbVk/SpirvCode.hpp>
#include <gbVk/StringConverters.hpp>
#include <gbVk/SubmitStaging.hpp>
#include <gbVk/Swapchain.hpp>

#include <gbBase/PerfLog.hpp>

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
        auto mapped = source_image.map();
        GhulbusGraphics::ImageLoader img_loader("textures/statue.jpg");
        auto const dim_x = img_loader.getWidth();
        auto const dim_y = img_loader.getHeight();
        auto img_data = img_loader.getData();
        for (uint32_t iy = 0; iy < main_window.getHeight(); ++iy) {
            for (uint32_t ix = 0; ix < main_window.getWidth(); ++ix) {
                uint32_t const i = iy * main_window.getWidth() + ix;
                if (!img_data || ix >= dim_x || iy >= dim_y) {
                    mapped[i * 4] = std::byte(255);           //R
                    mapped[i * 4 + 1] = std::byte(0);         //G
                    mapped[i * 4 + 2] = std::byte(0);         //B
                    mapped[i * 4 + 3] = std::byte(255);       //A
                } else {
                    auto const pixel_index = (iy * dim_x + ix) * 4;
                    mapped[i * 4] = img_data[pixel_index];                  //R
                    mapped[i * 4 + 1] = img_data[pixel_index + 1];          //G
                    mapped[i * 4 + 2] = img_data[pixel_index + 2];          //B
                    mapped[i * 4 + 3] = img_data[pixel_index + 3];          //A
                }
            }
        }
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

using Vertex = GhulbusGraphics::Mesh::VertexData;
using Index = GhulbusGraphics::Mesh::IndexData;

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

inline std::vector<Index> generateIndexData() {
    return { 0, 1, 2, 2, 3, 0,
             4, 5, 6, 6, 7, 4 };
}

int main()
{
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

    drawToBackbuffer(graphics_instance, main_window);

    // presentation
    main_window.present();

    graphics_instance.getGraphicsQueue().clearAllStaged();

    //std::this_thread::sleep_for(std::chrono::seconds(5));
    //return 0;

    perflog.tick(Ghulbus::LogLevel::Debug, "Initial present");

    std::vector<Vertex> const vertex_data = generateVertexData();
    std::vector<Index> const index_data = generateIndexData();
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
    vertex_attributes[1].offset = offsetof(Vertex, normal);

    vertex_attributes[2].location = 2;
    vertex_attributes[2].binding = vertex_binding.binding;
    vertex_attributes[2].format = VK_FORMAT_R32G32_SFLOAT;
    vertex_attributes[2].offset = offsetof(Vertex, texCoord);

    //*
    GhulbusGraphics::ImageLoader img_loader("textures/statue.jpg");
    GhulbusGraphics::Mesh mesh(graphics_instance, vertex_data.data(), vertex_data.size(),
                               index_data.data(), index_data.size(), img_loader);
    /*/
    GhulbusGraphics::ObjParser obj_parser;
    obj_parser.readFile("chalet.obj");
    GhulbusGraphics::ImageLoader img_loader("chalet.jpg");
    GhulbusGraphics::Mesh mesh(graphics_instance, obj_parser, img_loader);
    //*/
    auto& vertex_buffer = mesh.getVertexBuffer();
    auto& index_buffer = mesh.getIndexBuffer();
    auto& texture = mesh.getTexture();

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


    GhulbusVulkan::ImageView texture_image_view = texture.createImageView();
    GhulbusVulkan::Sampler texture_sampler = device.createSampler();

    auto vert_textured_spirv_code = GhulbusVulkan::SpirvCode::load("shaders/vert_textured.spv");
    auto frag_textured_spirv_code = GhulbusVulkan::SpirvCode::load("shaders/frag_textured.spv");
    GhulbusGraphics::Program shader_program(graphics_instance, vert_textured_spirv_code, frag_textured_spirv_code);

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

    GhulbusGraphics::Renderer renderer(graphics_instance, main_window.getSwapchain());

    GhulbusVulkan::Pipeline pipeline = [&device, &main_window, &pipeline_layout,
                                        &shader_program, &renderer,
                                        &vertex_binding, &vertex_attributes]() {
        auto builder = device.createGraphicsPipelineBuilder(main_window.getWidth(),
                                                            main_window.getHeight());
        builder.addVertexBindings(&vertex_binding, 1, vertex_attributes.data(),
                                  static_cast<uint32_t>(vertex_attributes.size()));
        return builder.create(pipeline_layout, shader_program.getShaderStageCreateInfos(),
                              shader_program.getNumberOfShaderStages(),
                              renderer.getRenderPass().getVkRenderPass());
    }();

    GhulbusVulkan::CommandBuffers triangle_draw_command_buffers =
        graphics_instance.getCommandPoolRegistry().allocateCommandBuffersGraphics(swapchain_n_images);

    for(uint32_t i = 0; i < triangle_draw_command_buffers.size(); ++i) {
        auto local_command_buffer = triangle_draw_command_buffers.getCommandBuffer(i);
        local_command_buffer.begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

        VkRenderPassBeginInfo render_pass_info;
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.pNext = nullptr;
        render_pass_info.renderPass = renderer.getRenderPass().getVkRenderPass();
        render_pass_info.framebuffer = renderer.getFramebufferByIndex(i).getVkFramebuffer();
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

        VkDescriptorSet desc_set_i = ubo_descriptor_sets.getDescriptorSet(i).getVkDescriptorSet();
        vkCmdBindDescriptorSets(local_command_buffer.getVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipeline_layout.getVkPipelineLayout(), 0, 1, &desc_set_i, 0, nullptr);

        VkBuffer vertexBuffers[] = { vertex_buffer.getBuffer().getVkBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(local_command_buffer.getVkCommandBuffer(), 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(local_command_buffer.getVkCommandBuffer(), index_buffer.getBuffer().getVkBuffer(),
                             0, VK_INDEX_TYPE_UINT32);
        auto const nindices = mesh.getNumberOfIndices();
        auto const nvertices = mesh.getNumberOfVertices();
        vkCmdDrawIndexed(local_command_buffer.getVkCommandBuffer(), mesh.getNumberOfIndices(),
                         1, 0, 0, 0);

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
        {
            GhulbusVulkan::SubmitStaging loop_stage;
            loop_stage.addWaitingSemaphore(main_window.getCurrentImageAcquireSemaphore(),
                                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
            auto command_buffer = triangle_draw_command_buffers.getCommandBuffer(frame_image_idx);
            loop_stage.addCommandBuffer(command_buffer);
            loop_stage.addSignalingSemaphore(semaphore_render_finished);
            graphics_instance.getGraphicsQueue().stageSubmission(std::move(loop_stage));
        }
        main_window.present(semaphore_render_finished);
        graphics_instance.getGraphicsQueue().clearAllStaged();
    }
}
