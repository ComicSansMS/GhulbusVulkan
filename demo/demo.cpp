
#include <gbBase/Finally.hpp>
#include <gbBase/Log.hpp>
#include <gbBase/LogHandlers.hpp>

#include <gbVk/CommandBuffer.hpp>
#include <gbVk/CommandPool.hpp>
#include <gbVk/Device.hpp>
#include <gbVk/DeviceMemory.hpp>
#include <gbVk/Fence.hpp>
#include <gbVk/Image.hpp>
#include <gbVk/Instance.hpp>
#include <gbVk/PhysicalDevice.hpp>
#include <gbVk/StringConverters.hpp>
#include <gbVk/Swapchain.hpp>

#include <GLFW/glfw3.h>

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
    auto dev_props = phys_devices.front().getProperties();
    GHULBUS_LOG(Info, "Using device " << dev_props.deviceName << " ("
                      << "Vulkan Version " << GhulbusVulkan::version_to_string(dev_props.apiVersion) << ", "
                      << "Driver Version " << GhulbusVulkan::version_to_string(dev_props.driverVersion)
                      << ").");

    GhulbusVulkan::Device device = phys_devices.front().createDevice();

    glfwWindowHint(GLFW_RESIZABLE, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    auto main_window =
        std::unique_ptr<GLFWwindow, void(*)(GLFWwindow*)>(glfwCreateWindow(1280, 720, "Vulkan Demo", nullptr, nullptr),
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

    uint32_t const queue_family = 0;    // @todo

    if(!device.getPhysicalDevice().getSurfaceSupport(queue_family, surface)) {
        GHULBUS_LOG(Error, "Selected queue does not support presentation for this surface.");
    }

    auto swapchain = device.createSwapChain(surface, queue_family);

    auto command_pool = device.createCommandPool(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, queue_family);
    auto command_buffer = command_pool.allocateCommandBuffers();

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
    auto images = swapchain.getImages();
    auto const image_index = swapchain.acquireNextImage(fence);
    auto image = images[*image_index];
    fence.wait();

    command_buffer.begin();

    auto source_image = device.createImage();
    auto mem_reqs = source_image.getMemoryRequirements();
    auto source_image_memory = device.allocateMemory(mem_reqs.size, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, mem_reqs);
    res = vkBindImageMemory(device.getVkDevice(), source_image.getVkImage(), source_image_memory.getVkDeviceMemory(), 0);
    if(res != VK_SUCCESS) {
        GHULBUS_LOG(Error, "Unable to bind image memory.");
    }
    {
        auto mapped = source_image_memory.map();
        for(int i=0; i<mem_reqs.size/4; ++i) {
            mapped[i * 4] = std::byte(0);           //B
            mapped[i * 4 + 1] = std::byte(0);       //G
            mapped[i * 4 + 2] = std::byte(255);     //R
            mapped[i * 4 + 3] = std::byte(0);       //A
        }
        mapped.flush();
    }

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
    color_image_view.image = image;

    res = vkCreateImageView(device.getVkDevice(), &color_image_view, NULL, &image_view);
    if(res != VK_SUCCESS) {
        GHULBUS_LOG(Error, "Unable to create image view.");
    }
    auto guard_image_view = Ghulbus::finally([&device, &image_view]() { vkDestroyImageView(device.getVkDevice(), image_view, nullptr); });
    */


    // set host image layout
    {
        VkImageMemoryBarrier image_barr;
        image_barr.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        image_barr.pNext = nullptr;
        image_barr.srcAccessMask = 0;
        image_barr.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        image_barr.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
        image_barr.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        image_barr.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        image_barr.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        image_barr.image = source_image.getVkImage();
        image_barr.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_barr.subresourceRange.baseMipLevel = 0;
        image_barr.subresourceRange.levelCount = 1;
        image_barr.subresourceRange.baseArrayLayer = 0;
        image_barr.subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(command_buffer.getVkCommandBuffer(),
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0, 0, nullptr, 0, nullptr,
            1, &image_barr);
    }

    // set swapchain image layout
    {
        VkImageMemoryBarrier image_barr;
        image_barr.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        image_barr.pNext = nullptr;
        image_barr.srcAccessMask = 0;
        image_barr.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        image_barr.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_barr.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        image_barr.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        image_barr.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        image_barr.image = image;
        image_barr.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_barr.subresourceRange.baseMipLevel = 0;
        image_barr.subresourceRange.levelCount = 1;
        image_barr.subresourceRange.baseArrayLayer = 0;
        image_barr.subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(command_buffer.getVkCommandBuffer(),
                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0, 0, nullptr, 0, nullptr,
                             1, &image_barr);
    }
    // copy
    {
        VkImageCopy region;
        region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.srcSubresource.mipLevel = 0;
        region.srcSubresource.baseArrayLayer = 0;
        region.srcSubresource.layerCount = 1;
        region.srcOffset.x = 0;
        region.srcOffset.y = 0;
        region.srcOffset.z = 0;
        region.dstSubresource = region.srcSubresource;
        region.dstOffset = region.srcOffset;
        region.extent.width = 1280;
        region.extent.height = 720;
        region.extent.depth = 1;
        vkCmdCopyImage(command_buffer.getVkCommandBuffer(),
                       source_image.getVkImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                       image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                       1, &region);
    }
    // presentation
    {
        VkImageMemoryBarrier image_barr;
        image_barr.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        image_barr.pNext = nullptr;
        image_barr.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        image_barr.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        image_barr.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        image_barr.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        image_barr.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        image_barr.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        image_barr.image = image;
        image_barr.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_barr.subresourceRange.baseMipLevel = 0;
        image_barr.subresourceRange.levelCount = 1;
        image_barr.subresourceRange.baseArrayLayer = 0;
        image_barr.subresourceRange.layerCount = 1;
        vkCmdPipelineBarrier(command_buffer.getVkCommandBuffer(),
                             VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                             0, 0, nullptr, 0, nullptr,
                             1, &image_barr);
    }
    command_buffer.end();
    command_buffer.submit(queue);
    vkQueueWaitIdle(queue);
    command_buffer.reset();

    command_buffer.begin();
    VkPresentInfoKHR present_info;
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = nullptr;
    present_info.waitSemaphoreCount = 0;
    present_info.pWaitSemaphores = nullptr;
    present_info.swapchainCount = 1;
    VkSwapchainKHR the_swapchain = swapchain.getVkSwapchainKHR();
    present_info.pSwapchains = &the_swapchain;
    uint32_t the_image_index = *image_index;
    present_info.pImageIndices = &the_image_index;
    VkResult the_result;
    present_info.pResults = &the_result;
    vkQueuePresentKHR(queue, &present_info);
    command_buffer.end();
    command_buffer.submit(queue);
    vkQueueWaitIdle(queue);
    command_buffer.reset();


    GHULBUS_LOG(Trace, "Entering main loop...");
    while(!glfwWindowShouldClose(main_window.get())) {
        glfwPollEvents();
    }
}
