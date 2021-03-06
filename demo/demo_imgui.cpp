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

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <memory>
#include <vector>

using VertexData = GhulbusGraphics::Mesh<>::VertexData;
using IndexData = GhulbusGraphics::Mesh<>::IndexData;

struct UBOMVP {
    GhulbusMath::Matrix4<float> model;
    GhulbusMath::Matrix4<float> view;
    GhulbusMath::Matrix4<float> projection;
};

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
    ImGuiContext* imgui_context = ImGui::CreateContext();
    auto const guard_imgui_context = Ghulbus::finally([imgui_context]() { ImGui::DestroyContext(imgui_context); });
    ImGui::SetCurrentContext(imgui_context);
    ImGui_ImplGlfw_InitForVulkan(main_window.getGlfwWindow(), true);

    perflog.tick(Ghulbus::LogLevel::Debug, "Window creation");

    GhulbusGraphics::Primitives::Quad<VertexData, uint32_t> mesh_quad(1.f, 1.f);

    GhulbusGraphics::ImageLoader img_loader("textures/statue.jpg");
    GhulbusGraphics::Mesh mesh(graphics_instance, mesh_quad.m_vertexData, mesh_quad.m_indexData, img_loader);
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
    struct RenderMode {
        bool wireframe = false;
    } render_mode;
    auto key_handler_guard = main_window.getEventReactor().eventHandlers.keyEvent.addHandler(
        [&do_animate, &render_mode](GhulbusGraphics::Event::Key const& e) {
            if (e.action == GhulbusGraphics::KeyAction::Press) {
                if (e.key == GhulbusGraphics::Key::A) {
                    do_animate ^= true;
                } else if (e.key == GhulbusGraphics::Key::W) {
                    render_mode.wireframe ^= true;
                }
            }
            return GhulbusGraphics::WindowEventReactor::Result::ContinueProcessing;
        });
    auto update_uniform_buffer = [&timestamp, &ubo_data, &ubo_buffers, &viewport_dimensions, &camera, &do_animate](uint32_t index) {
        auto const t = std::chrono::steady_clock::now();
        float const time = (do_animate) ? std::chrono::duration<float>(t - timestamp).count() : 0.f;
        ubo_data.model = GhulbusMath::make_rotation_x(-1.57079632679489f).m *
            GhulbusMath::make_rotation((GhulbusMath::traits::Pi<float>::value / 2.f) * time,
                                       GhulbusMath::Vector3f(0.f, 0.f, 1.f)).m;
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
    shader_program.addVertexBinding(0, mesh_quad.m_vertexData.getFormat());
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

    auto imgui_descpool = [&device]() {
        auto imgui_descpool_builder = device.createDescriptorPoolBuilder();
        imgui_descpool_builder.addDescriptorPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, 1000);
        imgui_descpool_builder.addDescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000);
        imgui_descpool_builder.addDescriptorPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000);
        imgui_descpool_builder.addDescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000);
        imgui_descpool_builder.addDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000);
        imgui_descpool_builder.addDescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000);
        imgui_descpool_builder.addDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000);
        imgui_descpool_builder.addDescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000);
        imgui_descpool_builder.addDescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000);
        imgui_descpool_builder.addDescriptorPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000);
        imgui_descpool_builder.addDescriptorPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000);
        return imgui_descpool_builder.create(11000, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);
    }();
    {
        ImGui_ImplVulkan_InitInfo imgui_init;
        imgui_init.Instance = graphics_instance.getVulkanInstance().getVkInstance();
        imgui_init.PhysicalDevice = physical_device.getVkPhysicalDevice();
        imgui_init.Device = device.getVkDevice();
        imgui_init.Queue = graphics_instance.getGraphicsQueue().getVkQueue();
        imgui_init.PipelineCache = nullptr;
        imgui_init.DescriptorPool = imgui_descpool.getVkDescriptorPool();
        imgui_init.MinImageCount = 2;
        imgui_init.ImageCount = 2;
        imgui_init.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        imgui_init.Allocator = nullptr;
        imgui_init.CheckVkResultFn = nullptr;
        ImGui_ImplVulkan_Init(&imgui_init, renderer.getRenderPass().getVkRenderPass());
    }
    auto guard_imgui_vulkan = Ghulbus::finally([]() { ImGui_ImplVulkan_Shutdown(); });
    {
        auto imgui_command_buffers = graphics_instance.getCommandPoolRegistry().allocateCommandBuffersGraphics_Transient(1);
        auto& imgui_create_command_buffer = imgui_command_buffers.getCommandBuffer(0);
        imgui_create_command_buffer.begin();
        ImGui_ImplVulkan_CreateFontsTexture(imgui_create_command_buffer.getVkCommandBuffer());
        imgui_create_command_buffer.end();
        graphics_instance.getGraphicsQueue().submit(imgui_create_command_buffer);
        graphics_instance.getGraphicsQueue().waitIdle();
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

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
    renderer.recreateAllPipelines();

    perflog.tick(Ghulbus::LogLevel::Debug, "Main setup");

    auto const mouse_move_handler_guard = main_window.getEventReactor().eventHandlers.mouseMoveEvent.addHandler(
        [](GhulbusGraphics::Event::MouseMove const& mm) {
            GHULBUS_UNUSED_VARIABLE(mm);
            //GHULBUS_LOG(Info, "Mouse " << mm.position.x << ", " << mm.position.y);
            return GhulbusGraphics::WindowEventReactor::Result::ContinueProcessing;
        });
    auto const keypress_handler_guard = main_window.getEventReactor().eventHandlers.keyEvent.addHandler(
        [](GhulbusGraphics::Event::Key const& ke) {
            GHULBUS_UNUSED_VARIABLE(ke);
            GHULBUS_LOG(Info, ke.key << " " << ke.action << " (" << ke.modifiers << ")");
            return GhulbusGraphics::WindowEventReactor::Result::ContinueProcessing;
        });

    transfer_fence.wait();
    graphics_instance.getTransferQueue().clearAllStaged();
    graphics_sync_fence.wait();
    graphics_instance.getGraphicsQueue().clearAllStaged();
    graphics_instance.getReactor().post([]() { GHULBUS_LOG(Debug, "Hello from Reactor!"); });

    bool show_demo_window = true;
    renderer.recordDrawCommands(0, [](GhulbusVulkan::CommandBuffer& command_buffer, uint32_t) {
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer.getVkCommandBuffer());
    });
    while(!main_window.isDone()) {
        graphics_instance.pollEvents();

        uint32_t frame_image_idx = main_window.getCurrentImageSwapchainIndex();
        update_uniform_buffer(frame_image_idx);
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
        }
        ImGui::Render();
        renderer.forceInvokeDrawCallback(0, frame_image_idx);

        renderer.render(0, main_window);
    }
}
