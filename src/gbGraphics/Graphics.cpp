#include <gbGraphics/Graphics.hpp>

#include <gbGraphics/Exceptions.hpp>
#include <gbGraphics/detail/QueueSelection.hpp>

#include <gbVk/Device.hpp>
#include <gbVk/DeviceBuilder.hpp>
#include <gbVk/Instance.hpp>
#include <gbVk/PhysicalDevice.hpp>
#include <gbVk/Queue.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Finally.hpp>
#include <gbBase/Log.hpp>

#include <algorithm>
#include <array>
#include <iterator>
#include <tuple>

#ifndef GLFW_INCLUDE_VULKAN
#   define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>

namespace GHULBUS_GRAPHICS_NAMESPACE {
namespace {
struct GlobalState {
    GhulbusVulkan::Instance instance;
    GhulbusVulkan::Device device;
    detail::DeviceQueues queues;

    GlobalState(GlobalState const&) = delete;
    GlobalState& operator=(GlobalState const&) = delete;
    GlobalState(GlobalState&&) = delete;
    GlobalState& operator=(GlobalState&&) = delete;
private:
    GlobalState(GhulbusVulkan::Instance&& i, GhulbusVulkan::Device&& d, detail::DeviceQueues&& q)
        :instance(std::move(i)), device(std::move(d)), queues(std::move(q))
    {}

    friend void initializeVulkanInstance(char const*, GhulbusVulkan::Instance::Version);
};
GlobalState* g_state = nullptr;

void initializeVulkanInstance(char const* application_name, GhulbusVulkan::Instance::Version application_version);
std::tuple<GhulbusVulkan::Device, detail::DeviceQueues> initializeVulkanDevice(GhulbusVulkan::Instance& instance);

void initializeVulkanInstance(char const* application_name, GhulbusVulkan::Instance::Version application_version)
{
    GHULBUS_PRECONDITION(!g_state);
    GhulbusVulkan::Instance::Layers layers{
#ifndef NDEBUG
        GhulbusVulkan::Instance::Layers::ActivateValidationLayers{}
#endif
    };
    GhulbusVulkan::Instance::Extensions extensions;
    uint32_t glfw_extension_count;
    char const** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    for (uint32_t i = 0; i < glfw_extension_count; ++i) {
        extensions.addExtension(glfw_extensions[i]);
    }

        GhulbusVulkan::Instance instance =
            GhulbusVulkan::Instance::createInstance(application_name, application_version, layers, extensions);
        auto [device, queues] = initializeVulkanDevice(instance);

        g_state = new GlobalState(std::move(instance), std::move(device), std::move(queues));
}

std::tuple <GhulbusVulkan::Device, detail::DeviceQueues> initializeVulkanDevice(GhulbusVulkan::Instance& instance)
{
    char const* required_extensions[] = { "VK_KHR_maintenance1" };

    std::vector<GhulbusVulkan::PhysicalDevice> physical_devices = instance.enumeratePhysicalDevices();
    detail::PhysicalDeviceCandidate const winner = [&instance, &physical_devices, required_extensions]() {
        std::vector<detail::PhysicalDeviceCandidate> candidates;
        // required
        for (std::size_t i = 0; i < physical_devices.size(); ++i) {
            GhulbusVulkan::PhysicalDevice& pd = physical_devices[i];
            detail::PhysicalDeviceCandidate candidate;
            candidate.physicalDeviceIndex = i;
            // required: must have graphics queue
            std::vector<VkQueueFamilyProperties> const qfps = pd.getQueueFamilyProperties();
            for (uint32_t queue_family_index = 0; queue_family_index < qfps.size(); ++queue_family_index) {
                VkQueueFamilyProperties const& qfprop = qfps[queue_family_index];
                if ((qfprop.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
                    candidate.graphics_queue_families.push_back(queue_family_index);
                } else if ((qfprop.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0) {
                    candidate.compute_queue_families.push_back(queue_family_index);
                } else if ((qfprop.queueFlags == VK_QUEUE_TRANSFER_BIT) != 0) {
                    candidate.transfer_queue_families.push_back(queue_family_index);
                }
            }
            if (candidate.graphics_queue_families.empty()) {
                continue;
            }

            // required: must be supported by glfw
            for (auto it = candidate.graphics_queue_families.begin(); it != candidate.graphics_queue_families.end();) {
                if (glfwGetPhysicalDevicePresentationSupport(instance.getVkInstance(),
                    pd.getVkPhysicalDevice(),
                    *it) == GLFW_TRUE)
                {
                    candidate.primary_queue_family = *it;
                    it = candidate.graphics_queue_families.erase(it);
                } else {
                    ++it;
                }
            }
            if (!candidate.primary_queue_family) {
                continue;
            }

            // required: extensions
            std::array<bool, sizeof(required_extensions) / sizeof(char*)> required_extension_supported;
            required_extension_supported.fill(false);

            for (auto const& ext : pd.enumerateDeviceExtensionProperties()) {
                auto it = std::find_if(std::begin(required_extensions), std::end(required_extensions),
                                       [&ext](char const* str) { return std::strcmp(str, ext.extensionName) == 0; });
                if (it != std::end(required_extensions)) {
                    required_extension_supported[std::distance(std::begin(required_extensions), it)] = true;
                }
            }
            if (auto it = std::find(required_extension_supported.begin(), required_extension_supported.end(), false);
                it != required_extension_supported.end())
            {
                continue;
            }

            candidates.push_back(candidate);
        }

        // prefer physical devices from discrete gpus
        auto it = std::find_if(candidates.begin(), candidates.end(), [&physical_devices](auto const& c) -> bool {
            GhulbusVulkan::PhysicalDevice& pd = physical_devices[c.physicalDeviceIndex];
            return (pd.getProperties().deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
            });
        if (it != candidates.end()) {
            return *it;
        } else if (!candidates.empty()) {
            // if still here, just pick the first device from the candidate list
            return candidates.front();
        } else {
            // we didn't find a suitable device (uh-oh)
            GHULBUS_THROW(Exceptions::IOError(), "No suitable Vulkan device could be found.");
        }
    }();

    GhulbusVulkan::PhysicalDevice& physical_device = physical_devices[winner.physicalDeviceIndex];
    GHULBUS_LOG(Info, "Using Vulkan device '" << physical_device.getProperties().deviceName << "'");

    GhulbusVulkan::DeviceBuilder device_builder = physical_device.createDeviceBuilder();

    // we instantiate:
    // 1 graphics queue
    // 1 compute queue
    // 1 transfer queue
    // the transfer queue, if absent, gets folded into compute queues, which in turn get folded into graphics
    detail::DeviceQueues queues = detail::selectQueues(winner, physical_device.getQueueFamilyProperties());
    for (auto const& q : detail::uniqueQueues(queues)) { device_builder.addQueues(q.queue_family_index, 1); }
    for (auto const& ext : required_extensions) { device_builder.addExtension(ext); }

    return std::make_tuple(device_builder.create(), queues);
}

void shutdownVulkanInstance()
{
    GHULBUS_PRECONDITION(g_state);
    delete g_state;
    g_state = nullptr;
}
}

void initialize()
{
    initialize(nullptr, {});
}

void initialize(char const* application_name, ApplicationVersion application_version)
{
    if (!glfwInit())
    {
        GHULBUS_THROW(Exceptions::GLFWError{}, "Initialization failed.");
    }
    auto exception_guard = Ghulbus::finally([]() { glfwTerminate(); });

    if (!glfwVulkanSupported()) {
        GHULBUS_THROW(Exceptions::GLFWError{}, "No Vulkan support in GLFW.");
    }
    glfwSetErrorCallback([](int ec, char const* msg) { GHULBUS_LOG(Error, "GLFW Error " << ec << " - " << msg); });
    initializeVulkanInstance(application_name,
        GhulbusVulkan::Instance::Version(application_version.major,
                                         application_version.minor,
                                         application_version.patch));
    exception_guard.defuse();
}

InitializeGuard initializeWithGuard()
{
    initialize();
    return InitializeGuard{};
}

InitializeGuard initializeWithGuard(char const* application_name, ApplicationVersion application_version)
{
    initialize(application_name, application_version);
    return InitializeGuard{};
}

void shutdown()
{
    shutdownVulkanInstance();
    glfwTerminate();
}

GhulbusVulkan::Instance& getVulkanInstance()
{
    GHULBUS_PRECONDITION(g_state);
    GlobalState* state = g_state;
    return state->instance;
}

GhulbusVulkan::PhysicalDevice getVulkanPhysicalDevice()
{
    GHULBUS_PRECONDITION(g_state);
    GlobalState* state = g_state;
    return state->device.getPhysicalDevice();
}

GhulbusVulkan::Device& getVulkanDevice()
{
    GHULBUS_PRECONDITION(g_state);
    GlobalState* state = g_state;
    return state->device;
}

GhulbusVulkan::Queue getGraphicsQueue()
{
    GHULBUS_PRECONDITION(g_state);
    GlobalState* state = g_state;
    detail::DeviceQueues::QueueId const& queue = state->queues.primary_queue;
    return state->device.getQueue(queue.queue_family_index, queue.queue_index);
}

uint32_t getGraphicsQueueFamilyIndex()
{
    GHULBUS_PRECONDITION(g_state);
    GlobalState* state = g_state;
    detail::DeviceQueues::QueueId const& queue = state->queues.primary_queue;
    return queue.queue_family_index;
}

uint32_t getGraphicsQueueIndex()
{
    GHULBUS_PRECONDITION(g_state);
    GlobalState* state = g_state;
    detail::DeviceQueues::QueueId const& queue = state->queues.primary_queue;
    return queue.queue_index;
}

GhulbusVulkan::Queue getComputeQueue()
{
    GHULBUS_PRECONDITION(g_state);
    GlobalState* state = g_state;
    detail::DeviceQueues::QueueId const& queue = state->queues.compute_queues.front();
    return state->device.getQueue(queue.queue_family_index, queue.queue_index);
}

uint32_t getComputeQueueFamilyIndex()
{
    GHULBUS_PRECONDITION(g_state);
    GlobalState* state = g_state;
    detail::DeviceQueues::QueueId const& queue = state->queues.compute_queues.front();
    return queue.queue_family_index;
}

uint32_t getComputeQueueIndex()
{
    GHULBUS_PRECONDITION(g_state);
    GlobalState* state = g_state;
    detail::DeviceQueues::QueueId const& queue = state->queues.compute_queues.front();
    return queue.queue_index;
}

GhulbusVulkan::Queue getTransferQueue()
{
    GHULBUS_PRECONDITION(g_state);
    GlobalState* state = g_state;
    detail::DeviceQueues::QueueId const& queue = state->queues.transfer_queues.front();
    return state->device.getQueue(queue.queue_family_index, queue.queue_index);
}

uint32_t getTransferQueueFamilyIndex()
{
    GHULBUS_PRECONDITION(g_state);
    GlobalState* state = g_state;
    detail::DeviceQueues::QueueId const& queue = state->queues.transfer_queues.front();
    return queue.queue_family_index;
}

uint32_t getTransferQueueIndex()
{
    GHULBUS_PRECONDITION(g_state);
    GlobalState* state = g_state;
    detail::DeviceQueues::QueueId const& queue = state->queues.transfer_queues.front();
    return queue.queue_index;
}
}
