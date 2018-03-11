
#include <gbBase/Finally.hpp>
#include <gbBase/Log.hpp>
#include <gbBase/LogHandlers.hpp>

#include <gbVk/CommandBuffer.hpp>
#include <gbVk/CommandBuffers.hpp>
#include <gbVk/CommandPool.hpp>
#include <gbVk/Device.hpp>
#include <gbVk/DeviceMemory.hpp>
#include <gbVk/Fence.hpp>
#include <gbVk/Image.hpp>
#include <gbVk/ImageView.hpp>
#include <gbVk/Instance.hpp>
#include <gbVk/PhysicalDevice.hpp>
#include <gbVk/Semaphore.hpp>
#include <gbVk/ShaderModule.hpp>
#include <gbVk/Spirv.hpp>
#include <gbVk/StringConverters.hpp>
#include <gbVk/Swapchain.hpp>

#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <external/stb_image.h>

#include <cstring>
#include <memory>
#include <vector>


struct DemoState {};

int main()
{
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

    GhulbusVulkan::Device device = physical_device.createDevice();

    glfwWindowHint(GLFW_RESIZABLE, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    int const WINDOW_WIDTH = 1280;
    int const WINDOW_HEIGHT = 720;
    auto main_window =
        std::unique_ptr<GLFWwindow, void(*)(GLFWwindow*)>(glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan Demo",
                                                                           nullptr, nullptr),
                                                          glfwDestroyWindow);
    if(!main_window) {
        return 1;
    }
    VkSurfaceKHR surface;
    VkResult res = glfwCreateWindowSurface(instance.getVkInstance(), main_window.get(), nullptr, &surface);
    if(res != VK_SUCCESS) {
        GHULBUS_LOG(Error, "Unable to create Vulkan surface.");
    }

    DemoState state;
    glfwSetWindowUserPointer(main_window.get(), &state);

    glfwSetKeyCallback(main_window.get(),
        [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
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


    // find queue family
    auto const opt_queue_family = [&device, &surface]() -> std::optional<uint32_t> {
        uint32_t i = 0;
        for(auto const& qfp : device.getPhysicalDevice().getQueueFamilyProperties()) {
            if((qfp.queueCount > 0) && (qfp.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
                // @todo: graphics and presentation might only be available on separate queues
                if(device.getPhysicalDevice().getSurfaceSupport(i, surface)) {
                    return i;
                }
            }
            ++i;
        }
        GHULBUS_LOG(Error, "No suitable queue family found.");
        return std::nullopt;
    }();
    if(!opt_queue_family) {
        return 1;
    }

    uint32_t const queue_family = *opt_queue_family;

    auto swapchain = device.createSwapChain(surface, queue_family);

    auto command_pool = device.createCommandPool(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT |
                                                 VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, queue_family);
    auto command_buffers = command_pool.allocateCommandBuffers(1);
    auto command_buffer = command_buffers.getCommandBuffer(0);

    command_buffer.begin();
    VkBufferCopy region;
    region.srcOffset = 0;
    region.dstOffset = 0;
    region.size = 1024*1024*64;
    //vkCmdCopyBuffer(command_buffer, host_memory, memory, 1, & region);
    command_buffer.end();

    auto queue = device.getQueue(queue_family, 0);
    command_buffer.submit(queue);
    vkQueueWaitIdle(queue);
    command_buffer.reset();

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

    auto spirv_code = GhulbusVulkan::Spirv::load("../demo/shaders/simple.spv");
    auto version = spirv_code.getSpirvVersion();
    auto bound = spirv_code.getBound();
    auto shader_module = device.createShaderModule(spirv_code);

    auto vert_spirv_code = GhulbusVulkan::Spirv::load("../demo/shaders/vert.spv");
    auto vert_shader_module = device.createShaderModule(vert_spirv_code);
    VkPipelineShaderStageCreateInfo vert_shader_stage_ci;
    vert_shader_stage_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_ci.pNext = nullptr;
    vert_shader_stage_ci.flags = 0;
    vert_shader_stage_ci.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_ci.module = vert_shader_module.getVkShaderModule();
    vert_shader_stage_ci.pName = "main";
    vert_shader_stage_ci.pSpecializationInfo = nullptr;

    auto frag_spirv_code = GhulbusVulkan::Spirv::load("../demo/shaders/frag.spv");
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
    VkAttachmentDescription render_pass_color_attachment;
    render_pass_color_attachment.flags = 0;
    render_pass_color_attachment.format = swapchain_image->getFormat();
    render_pass_color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    render_pass_color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    render_pass_color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    render_pass_color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    render_pass_color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    render_pass_color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    render_pass_color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    VkAttachmentReference render_pass_color_attachment_ref;
    render_pass_color_attachment_ref.attachment = 0;
    render_pass_color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkSubpassDescription render_pass_subpass_desc;
    render_pass_subpass_desc.flags = 0;
    render_pass_subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    render_pass_subpass_desc.inputAttachmentCount = 0;
    render_pass_subpass_desc.pInputAttachments = nullptr;
    render_pass_subpass_desc.colorAttachmentCount = 1;
    render_pass_subpass_desc.pColorAttachments = &render_pass_color_attachment_ref;
    render_pass_subpass_desc.pResolveAttachments = nullptr;
    render_pass_subpass_desc.pDepthStencilAttachment = nullptr;
    render_pass_subpass_desc.preserveAttachmentCount = 0;
    render_pass_subpass_desc.pPreserveAttachments = nullptr;
    VkSubpassDependency render_pass_subpass_dep;
    render_pass_subpass_dep.srcSubpass = VK_SUBPASS_EXTERNAL;
    render_pass_subpass_dep.dstSubpass = 0;
    render_pass_subpass_dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    render_pass_subpass_dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    render_pass_subpass_dep.srcAccessMask = 0;
    render_pass_subpass_dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    render_pass_subpass_dep.dependencyFlags = 0;
    VkRenderPassCreateInfo render_pass_ci;
    render_pass_ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_ci.pNext = nullptr;
    render_pass_ci.flags = 0;
    render_pass_ci.attachmentCount = 1;
    render_pass_ci.pAttachments = &render_pass_color_attachment;
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &render_pass_subpass_desc;
    render_pass_ci.dependencyCount = 1;
    render_pass_ci.pDependencies = &render_pass_subpass_dep;
    VkRenderPass render_pass;
    res = vkCreateRenderPass(device.getVkDevice(), &render_pass_ci, nullptr, &render_pass);
    if(res != VK_SUCCESS) { GHULBUS_LOG(Error, "Error in vkCreateRenderPass: " << res); return 1; }
    std::unique_ptr<VkRenderPass, std::function<void(VkRenderPass*)>> guard_render_pass(&render_pass,
        [&device](VkRenderPass* r) { vkDestroyRenderPass(device.getVkDevice(), *r, nullptr); });


    // vertex input
    VkPipelineVertexInputStateCreateInfo vertex_input_ci;
    vertex_input_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_ci.pNext = nullptr;
    vertex_input_ci.flags = 0;
    vertex_input_ci.vertexBindingDescriptionCount = 0;
    vertex_input_ci.pVertexBindingDescriptions = nullptr;
    vertex_input_ci.vertexAttributeDescriptionCount = 0;
    vertex_input_ci.pVertexAttributeDescriptions = nullptr;

    // input assembly
    VkPipelineInputAssemblyStateCreateInfo input_assembly_ci;
    input_assembly_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_ci.pNext = nullptr;
    input_assembly_ci.flags = 0;
    input_assembly_ci.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_ci.primitiveRestartEnable = VK_FALSE;

    // viewport
    VkViewport viewport;
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = static_cast<float>(swapchain_image->getWidth());
    viewport.height = static_cast<float>(swapchain_image->getHeight());
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;
    VkRect2D scissor;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent.width = swapchain_image->getWidth();
    scissor.extent.height = swapchain_image->getHeight();
    VkPipelineViewportStateCreateInfo viewport_ci;
    viewport_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_ci.pNext = nullptr;
    viewport_ci.flags = 0;
    viewport_ci.viewportCount = 1;
    viewport_ci.pViewports = &viewport;
    viewport_ci.scissorCount = 1;
    viewport_ci.pScissors = &scissor;

    // rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer_ci;
    rasterizer_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer_ci.pNext = nullptr;
    rasterizer_ci.flags = 0;
    rasterizer_ci.depthClampEnable = VK_FALSE;
    rasterizer_ci.rasterizerDiscardEnable = VK_FALSE;
    rasterizer_ci.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer_ci.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer_ci.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer_ci.depthBiasEnable = VK_FALSE;
    rasterizer_ci.depthBiasConstantFactor = 0.f;
    rasterizer_ci.depthBiasClamp = 0.f;
    rasterizer_ci.depthBiasSlopeFactor = 0.f;
    rasterizer_ci.lineWidth = 1.f;

    // multisampling
    VkPipelineMultisampleStateCreateInfo multisample_ci;
    multisample_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_ci.pNext = nullptr;
    multisample_ci.flags = 0;
    multisample_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample_ci.sampleShadingEnable = VK_FALSE;
    multisample_ci.minSampleShading = 1.f;
    multisample_ci.pSampleMask = nullptr;
    multisample_ci.alphaToCoverageEnable = VK_FALSE;
    multisample_ci.alphaToOneEnable = VK_FALSE;

    // depth/stencil
    //not yet

    // color blending
    VkPipelineColorBlendAttachmentState color_blend_attach_state;
    color_blend_attach_state.blendEnable = VK_FALSE;
    color_blend_attach_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attach_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attach_state.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attach_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attach_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attach_state.alphaBlendOp = VK_BLEND_OP_ADD;
    color_blend_attach_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                              VK_COLOR_COMPONENT_G_BIT |
                                              VK_COLOR_COMPONENT_B_BIT |
                                              VK_COLOR_COMPONENT_A_BIT;
    VkPipelineColorBlendStateCreateInfo color_blend_ci;
    color_blend_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_ci.pNext = nullptr;
    color_blend_ci.flags = 0;
    color_blend_ci.logicOpEnable = VK_FALSE;
    color_blend_ci.logicOp = VK_LOGIC_OP_COPY;
    color_blend_ci.attachmentCount = 1;
    color_blend_ci.pAttachments = &color_blend_attach_state;
    color_blend_ci.blendConstants[0] = 0.f;
    color_blend_ci.blendConstants[1] = 0.f;
    color_blend_ci.blendConstants[2] = 0.f;
    color_blend_ci.blendConstants[3] = 0.f;

    // pipeline layout
    VkPipelineLayoutCreateInfo pipeline_layout_ci;
    pipeline_layout_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_ci.pNext = nullptr;
    pipeline_layout_ci.flags = 0;
    pipeline_layout_ci.setLayoutCount = 0;
    pipeline_layout_ci.pSetLayouts = nullptr;
    pipeline_layout_ci.pushConstantRangeCount = 0;
    pipeline_layout_ci.pPushConstantRanges = nullptr;
    VkPipelineLayout pipeline_layout;
    res = vkCreatePipelineLayout(device.getVkDevice(), &pipeline_layout_ci, nullptr, &pipeline_layout);
    if(res != VK_SUCCESS) { GHULBUS_LOG(Error, "Error in vkCreatePipelineLayout: " << res); return 1; }
    std::unique_ptr<VkPipelineLayout, std::function<void(VkPipelineLayout*)>> guard_pipeline_layout(&pipeline_layout,
        [&device](VkPipelineLayout* p) { vkDestroyPipelineLayout(device.getVkDevice(), *p, nullptr); });

    // pipeline
    VkGraphicsPipelineCreateInfo pipeline_ci;
    pipeline_ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_ci.pNext = nullptr;
    pipeline_ci.flags = 0;
    pipeline_ci.stageCount = static_cast<std::uint32_t>(shader_stage_cis.size());
    pipeline_ci.pStages = shader_stage_cis.data();
    pipeline_ci.pVertexInputState = &vertex_input_ci;
    pipeline_ci.pInputAssemblyState = &input_assembly_ci;
    pipeline_ci.pTessellationState = nullptr;
    pipeline_ci.pViewportState = &viewport_ci;
    pipeline_ci.pRasterizationState = &rasterizer_ci;
    pipeline_ci.pMultisampleState = &multisample_ci;
    pipeline_ci.pDepthStencilState = nullptr;
    pipeline_ci.pColorBlendState = &color_blend_ci;
    pipeline_ci.pDynamicState = nullptr;
    pipeline_ci.layout = pipeline_layout;
    pipeline_ci.renderPass = render_pass;
    pipeline_ci.subpass = 0;
    pipeline_ci.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_ci.basePipelineIndex = -1;
    VkPipeline pipeline;
    res = vkCreateGraphicsPipelines(device.getVkDevice(), VK_NULL_HANDLE, 1, &pipeline_ci, nullptr, &pipeline);
    if(res != VK_SUCCESS) { GHULBUS_LOG(Error, "Error in vkCreateGraphicsPipelines: " << res); return 1; }
    std::unique_ptr<VkPipeline, std::function<void(VkPipeline*)>> guard_pipeline(&pipeline,
        [&device](VkPipeline* p) { vkDestroyPipeline(device.getVkDevice(), *p, nullptr); });


    // framebuffer creation
    std::vector<GhulbusVulkan::ImageView> image_views = swapchain.getImageViews();
    std::vector<VkFramebuffer> framebuffers;
    framebuffers.resize(images.size());
    for(std::size_t i = 0; i < image_views.size(); ++i) {
        VkImageView attachments[] = { image_views[i].getVkImageView() };
        VkFramebufferCreateInfo framebuffer_ci;
        framebuffer_ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_ci.pNext = nullptr;
        framebuffer_ci.flags = 0;
        framebuffer_ci.renderPass = render_pass;
        framebuffer_ci.attachmentCount = 1;
        framebuffer_ci.pAttachments = attachments;
        framebuffer_ci.width = swapchain_image->getWidth();
        framebuffer_ci.height = swapchain_image->getHeight();
        framebuffer_ci.layers = 1;
        res = vkCreateFramebuffer(device.getVkDevice(), &framebuffer_ci, nullptr, &framebuffers[i]);
        if(res != VK_SUCCESS) { GHULBUS_LOG(Error, "Error in vkCreateFramebuffer: " << res); return 1; }
    }
    std::unique_ptr<std::vector<VkFramebuffer>, std::function<void(std::vector<VkFramebuffer>*)>> guard_framebuffers(&framebuffers,
        [&device](std::vector<VkFramebuffer>* f) { for(auto& fb : *f) { vkDestroyFramebuffer(device.getVkDevice(), fb, nullptr); } });

    GhulbusVulkan::CommandBuffers triangle_draw_command_buffers =
        command_pool.allocateCommandBuffers(swapchain.getNumberOfImages());

    for(uint32_t i = 0; i < triangle_draw_command_buffers.size(); ++i) {
        auto local_command_buffer = triangle_draw_command_buffers.getCommandBuffer(i);
        local_command_buffer.begin(VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);

        VkRenderPassBeginInfo render_pass_info;
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.pNext = nullptr;
        render_pass_info.renderPass = render_pass;
        render_pass_info.framebuffer = framebuffers[i];
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
        vkCmdBindPipeline(local_command_buffer.getVkCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        vkCmdDraw(local_command_buffer.getVkCommandBuffer(), 3, 1, 0, 0);
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
