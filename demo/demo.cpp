
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
#include <gbVk/Pipeline.hpp>
#include <gbVk/PipelineBuilder.hpp>
#include <gbVk/PipelineLayout.hpp>
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
    auto guard_surface = Ghulbus::finally([&instance, surface]() { vkDestroySurfaceKHR(instance.getVkInstance(), surface, nullptr); });

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
    //vkCmdCopyBuffer(command_buffer.getVkCommandBuffer(), host_memory, memory, 1, & region);
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
    GhulbusVulkan::RenderPass render_pass = [&device, &swapchain_image]() {
            auto builder = device.createRenderPassBuilder();
            builder.addSubpassGraphics();
            builder.addColorAttachment(swapchain_image->getFormat());
            return builder.create();
        }();

    // pipeline
    GhulbusVulkan::PipelineLayout pipeline_layout = device.createPipelineLayout();

    GhulbusVulkan::Pipeline pipeline = [&device, &swapchain_image, &pipeline_layout,
                                        &shader_stage_cis, &render_pass]() {
        auto builder = device.createGraphicsPipelineBuilder(swapchain_image->getWidth(),
                                                            swapchain_image->getHeight());
        return builder.create(pipeline_layout, shader_stage_cis.data(),
                              static_cast<std::uint32_t>(shader_stage_cis.size()),
                              render_pass.getVkRenderPass());
    }();


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
        framebuffer_ci.renderPass = render_pass.getVkRenderPass();
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
        render_pass_info.renderPass = render_pass.getVkRenderPass();
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
        vkCmdBindPipeline(local_command_buffer.getVkCommandBuffer(),
                          VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.getVkPipeline());
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
