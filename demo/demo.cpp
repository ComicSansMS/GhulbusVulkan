
#include <gbBase/Finally.hpp>
#include <gbBase/Log.hpp>
#include <gbBase/LogHandlers.hpp>

#include <gbVk/CommandBuffer.hpp>
#include <gbVk/CommandPool.hpp>
#include <gbVk/Device.hpp>
#include <gbVk/DeviceMemory.hpp>
#include <gbVk/Fence.hpp>
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
    VkImageMemoryBarrier image_barr;
    image_barr.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    image_barr.pNext = nullptr;
    image_barr.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    image_barr.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    image_barr.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    image_barr.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    image_barr.srcQueueFamilyIndex = 0;
    image_barr.dstQueueFamilyIndex = 0;
    image_barr.image = image;
    image_barr.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_barr.subresourceRange.baseMipLevel = 0;
    image_barr.subresourceRange.levelCount = 1;
    image_barr.subresourceRange.baseArrayLayer = 0;
    image_barr.subresourceRange.layerCount = 1;
    vkCmdPipelineBarrier(command_buffer.getVkCommandBuffer(),
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                         0, 0, nullptr, 0, nullptr,
                         1, &image_barr);
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
