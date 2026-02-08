
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
#include <gbGraphics/InputCameraSpherical.hpp>
#include <gbGraphics/MemoryBuffer.hpp>
#include <gbGraphics/Mesh.hpp>
#include <gbGraphics/MeshPrimitives.hpp>
#include <gbGraphics/ObjParser.hpp>
#include <gbGraphics/Program.hpp>
#include <gbGraphics/Reactor.hpp>
#include <gbGraphics/Renderer.hpp>
#include <gbGraphics/VertexFormat.hpp>
#include <gbGraphics/Window.hpp>
#include <gbGraphics/WindowEventReactor.hpp>

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

[[nodiscard]]
GhulbusVulkan::Semaphore drawToBackbuffer(GhulbusGraphics::GraphicsInstance& graphics_instance, GhulbusGraphics::Window& main_window)
{
    auto command_buffers = graphics_instance.getCommandPoolRegistry().allocateCommandBuffersGraphics_Transient(1);
    auto& command_buffer = command_buffers.getCommandBuffer(0);

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
    submission.addWaitingSemaphore(main_window.getCurrentImageAcquireSemaphore(),
                                   VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);
    GhulbusVulkan::Semaphore render_completed_semaphore = graphics_instance.getVulkanDevice().createSemaphore();
    submission.addSignalingSemaphore(render_completed_semaphore, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);
    graphics_instance.getGraphicsQueue().stageSubmission(std::move(submission));
    return render_completed_semaphore;
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

using VertexData = GhulbusGraphics::Mesh<>::VertexData;
using IndexData = GhulbusGraphics::Mesh<>::IndexData;

struct UBOMVP {
    GhulbusMath::Matrix4<float> model;
    GhulbusMath::Matrix4<float> view;
    GhulbusMath::Matrix4<float> projection;
};

inline VertexData generateVertexData()
{
    using namespace GhulbusMath;
    return {
        {Point3f{-0.5f, -0.5f,  0.0f}, Normal3f{1.0f, 0.0f, 0.0f}, Vector2f{0.0f, 0.0f}},
        {Point3f{ 0.5f, -0.5f,  0.0f}, Normal3f{0.0f, 1.0f, 0.0f}, Vector2f{1.0f, 0.0f}},
        {Point3f{ 0.5f,  0.5f,  0.0f}, Normal3f{0.0f, 0.0f, 1.0f}, Vector2f{1.0f, 1.0f}},
        {Point3f{-0.5f,  0.5f,  0.0f}, Normal3f{1.0f, 1.0f, 1.0f}, Vector2f{0.0f, 1.0f}},

        {Point3f{-0.5f, -0.5f, -0.5f}, Normal3f{1.0f, 0.0f, 0.0f}, Vector2f{0.0f, 0.0f}},
        {Point3f{ 0.5f, -0.5f, -0.5f}, Normal3f{0.0f, 1.0f, 0.0f}, Vector2f{1.0f, 0.0f}},
        {Point3f{ 0.5f,  0.5f, -0.5f}, Normal3f{0.0f, 0.0f, 1.0f}, Vector2f{1.0f, 1.0f}},
        {Point3f{-0.5f,  0.5f, -0.5f}, Normal3f{1.0f, 1.0f, 1.0f}, Vector2f{0.0f, 1.0f}}
    };
}

inline IndexData generateIndexData() {
    return { {0, 1, 2}, {2, 3, 0},
             {4, 5, 6}, {6, 7, 4} };
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

    GhulbusVulkan::Semaphore render_completed_semaphore = drawToBackbuffer(graphics_instance, main_window);

    // presentation
    if(main_window.present(render_completed_semaphore) != GhulbusGraphics::Window::PresentStatus::Ok) {
        GHULBUS_LOG(Error, "Error during initial present.");
        return 1;
    }

    graphics_instance.getGraphicsQueue().clearAllStaged();

    //std::this_thread::sleep_for(std::chrono::seconds(5));
    //return 0;

    perflog.tick(Ghulbus::LogLevel::Debug, "Initial present");

    VertexData const vertex_data = generateVertexData();
    //std::vector<Index> const index_data = generateIndexData();

    GhulbusGraphics::Primitives::Quad<VertexData, uint32_t> mesh_quad(1.f, 1.f);
    GhulbusGraphics::Primitives::Grid<VertexData, uint32_t> mesh_grid(1.f, 1.f, 4, 6);
    GhulbusGraphics::Primitives::Box<VertexData, uint32_t> mesh_box(GhulbusMath::AABB3<float>({-0.5f, -0.75f, -1.5f}, {0.5f, 0.75f, 1.5f}));
    GhulbusGraphics::Primitives::OpenCylinder<VertexData, uint32_t> mesh_opencyl(0.2f, 1.f, 12, 7);
    GhulbusGraphics::Primitives::Disc<VertexData, uint32_t> mesh_disc(0.2f, 15);
    GhulbusGraphics::Primitives::Cone<VertexData, uint32_t> mesh_cone(0.2f, 0.5f, 15);
    GhulbusGraphics::Primitives::Sphere<VertexData, uint32_t> mesh_sphere(0.5f, 25, 20);

    //*
    GhulbusGraphics::ImageLoader img_loader("textures/statue.jpg");
    GhulbusGraphics::AnyMesh mesh(GhulbusGraphics::Mesh(graphics_instance, mesh_sphere.m_vertexData, mesh_sphere.m_indexData, img_loader));
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
    struct ViewportDimensions {
        float width;
        float height;
    } viewport_dimensions;
    viewport_dimensions.width = static_cast<float>(main_window.getWidth());
    viewport_dimensions.height = static_cast<float>(main_window.getHeight());
    main_window.addRecreateSwapchainCallback([&viewport_dimensions](GhulbusVulkan::Swapchain& swapchain) {
        viewport_dimensions.width = static_cast<float>(swapchain.getWidth());
        viewport_dimensions.height = static_cast<float>(swapchain.getHeight());
        GHULBUS_LOG(Info, "Viewport resize " << swapchain.getWidth() << "x" << swapchain.getHeight());
    });
    GhulbusGraphics::Input::CameraSpherical camera(main_window);
    camera.setCameraDistance(4.f);
    camera.setCameraAngleVertical(0.785f);
    bool do_animate = true;
    auto key_handler_guard = main_window.getEventReactor().eventHandlers.keyEvent.addHandler(
        [&do_animate](GhulbusGraphics::Event::Key const& e) {
            if ((e.action == GhulbusGraphics::KeyAction::Press) && (e.key == GhulbusGraphics::Key::A)) {
                do_animate ^= true;
            }
            return GhulbusGraphics::WindowEventReactor::Result::ContinueProcessing;
        });
    auto update_uniform_buffer = [&timestamp, &ubo_data, &ubo_buffers, &viewport_dimensions, &camera, &do_animate](uint32_t index) {
        auto const t = std::chrono::steady_clock::now();
        float const time = (do_animate) ? std::chrono::duration<float>(t - timestamp).count() : 0.f;
        ubo_data.model = GhulbusMath::make_rotation_x(-1.57079632679489f).m *
            GhulbusMath::make_rotation((GhulbusMath::traits::Pi<float>::value / 2.f) * time,
                                       GhulbusMath::Vector3f(0.f, 0.f, 1.f)).m;
        ubo_data.model = GhulbusMath::identity4<float>();
        ubo_data.view = camera.getTransform().m;
        ubo_data.projection = GhulbusMath::make_perspective_projection(viewport_dimensions.width,
                                                                       viewport_dimensions.height, 0.1f, 10.f).m;
        ubo_data.projection = GhulbusMath::make_perspective_projection_fov(
            (GhulbusMath::traits::Pi<float>::value / 4.f),
            viewport_dimensions.width / viewport_dimensions.height, 0.1f, 10.f).m;

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
    if(graphics_instance.getTransferQueueFamilyIndex() != graphics_instance.getGraphicsQueueFamilyIndex())
    {
        GhulbusVulkan::SubmitStaging graphics_transfer_sync;
        auto sync_command_buffers = graphics_instance.getCommandPoolRegistry().allocateCommandBuffersGraphics_Transient(1);
        auto& sync_command_buffer = sync_command_buffers.getCommandBuffer(0);
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
    shader_program.addVertexBinding(0, vertex_data.getFormat());
    shader_program.bindVertexInput(GhulbusGraphics::VertexFormatBase::ComponentSemantics::Position, 0, 0);
    //shader_program.bindVertexInput(GhulbusGraphics::VertexFormatBase::ComponentSemantics::Color, 0, 1);
    shader_program.bindVertexInput(GhulbusGraphics::VertexFormatBase::ComponentSemantics::Texture, 0, 2);

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
    GhulbusGraphics::Renderer renderer(graphics_instance, shader_program, main_window.getSwapchain());
    {
        GhulbusVulkan::PipelineLayout pipeline_layout = [&device, &ubo_layout]() {
            GhulbusVulkan::PipelineLayoutBuilder builder = device.createPipelineLayoutBuilder();
            builder.addDescriptorSetLayout(ubo_layout);
            return builder.create();
        }();
        renderer.addPipelineBuilder(std::move(pipeline_layout));
    }
    GhulbusVulkan::PipelineLayout& pipeline_layout = renderer.getPipelineLayout(0);
    renderer.clonePipelineBuilder(0);
    renderer.getPipelineBuilder(1).stage.rasterization.polygonMode = VK_POLYGON_MODE_LINE;
    renderer.getPipelineBuilder(1).stage.rasterization.cullMode = VK_CULL_MODE_NONE;

    renderer.recordDrawCommands(0,
        [&ubo_descriptor_sets, &pipeline_layout, &vertex_buffer, &index_buffer, &mesh]
        (GhulbusVulkan::CommandBuffer& command_buffer, uint32_t i)
        {
            VkDescriptorSet desc_set_i = ubo_descriptor_sets.getDescriptorSet(i).getVkDescriptorSet();
            vkCmdBindDescriptorSets(command_buffer.getVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipeline_layout.getVkPipelineLayout(), 0, 1, &desc_set_i, 0, nullptr);

            VkBuffer vertexBuffers[] = { vertex_buffer.getBuffer().getVkBuffer() };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(command_buffer.getVkCommandBuffer(), 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(command_buffer.getVkCommandBuffer(), index_buffer.getBuffer().getVkBuffer(),
                0, VK_INDEX_TYPE_UINT32);
            auto const nindices = mesh.getNumberOfIndices();
            auto const nvertices = mesh.getNumberOfVertices();
            vkCmdDrawIndexed(command_buffer.getVkCommandBuffer(), mesh.getNumberOfIndices(), 1, 0, 0, 0);
        });
    renderer.copyDrawCommands(0, 0, 1);
    renderer.setClearColor(GhulbusMath::Color4f(0.8f, 0.8f, 0.8f));
    renderer.recreateAllPipelines();

    perflog.tick(Ghulbus::LogLevel::Debug, "Main setup");

    enum class DrawMode {
        Solid,
        Wireframe
    };
    DrawMode draw_mode = DrawMode::Solid;
    auto const mouse_move_handler_guard = main_window.getEventReactor().eventHandlers.mouseMoveEvent.addHandler(
        [](GhulbusGraphics::Event::MouseMove const& mm) {
            GHULBUS_UNUSED_VARIABLE(mm);
            //GHULBUS_LOG(Info, "Mouse " << mm.position.x << ", " << mm.position.y);
            return GhulbusGraphics::WindowEventReactor::Result::ContinueProcessing;
        });
    auto const keypress_handler_guard = main_window.getEventReactor().eventHandlers.keyEvent.addHandler(
        [&draw_mode](GhulbusGraphics::Event::Key const& ke) {
        GHULBUS_UNUSED_VARIABLE(ke);
        GHULBUS_LOG(Info, ke.key << " " << ke.action << " (" << ke.modifiers << ")");
        if (ke.action == GhulbusGraphics::KeyAction::Press) {
            if (ke.key == GhulbusGraphics::Key::W) {
                draw_mode = (draw_mode != DrawMode::Wireframe) ? DrawMode::Wireframe : DrawMode::Solid;
            }
        }
        return GhulbusGraphics::WindowEventReactor::Result::ContinueProcessing;
    });

    transfer_fence.wait();
    graphics_instance.getTransferQueue().clearAllStaged();
    graphics_sync_fence.wait();
    graphics_instance.getGraphicsQueue().clearAllStaged();
    graphics_instance.getReactor().post([]() { GHULBUS_LOG(Debug, "Hello from Reactor!"); });

    while(!main_window.isDone()) {
        graphics_instance.pollEvents();
        graphics_instance.getReactor().pump();

        uint32_t frame_image_idx = main_window.getCurrentImageSwapchainIndex();
        update_uniform_buffer(frame_image_idx);

        uint32_t active_pipeline = (draw_mode == DrawMode::Wireframe) ? 1 : 0;
        renderer.render(active_pipeline, main_window);
    }
}
