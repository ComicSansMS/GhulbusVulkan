#include <gbGraphics/GraphicsInstance.hpp>

#include <gbGraphics/CommandPoolRegistry.hpp>
#include <gbGraphics/Exceptions.hpp>
#include <gbGraphics/Reactor.hpp>
#include <gbGraphics/detail/DeviceMemoryAllocator_VMA.hpp>
#include <gbGraphics/detail/QueueSelection.hpp>

#include <gbVk/DebugReportCallback.hpp>
#include <gbVk/Device.hpp>
#include <gbVk/DeviceBuilder.hpp>
#include <gbVk/Instance.hpp>
#include <gbVk/PhysicalDevice.hpp>
#include <gbVk/Queue.hpp>
#include <gbVk/StringConverters.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Finally.hpp>
#include <gbBase/Log.hpp>
#include <gbBase/UnusedVariable.hpp>

#ifndef GLFW_INCLUDE_VULKAN
#   define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>

#include <algorithm>
#include <array>
#include <iterator>
#include <tuple>

namespace GHULBUS_GRAPHICS_NAMESPACE {
struct GraphicsInstance::Pimpl {
    GhulbusVulkan::Instance instance;
    GhulbusVulkan::Device device;
    detail::DeviceQueues queues;
    detail::DeviceMemoryAllocator_VMA allocator;
    GhulbusVulkan::Queue queue_graphics;
    GhulbusVulkan::Queue queue_compute;
    GhulbusVulkan::Queue queue_transfer;
    std::optional<GhulbusVulkan::DebugReportCallback> debug_logging;

    Pimpl(GhulbusVulkan::Instance&& i, GhulbusVulkan::Device&& d, detail::DeviceQueues&& q,
          detail::DeviceMemoryAllocator_VMA && a)
        : instance(std::move(i)), device(std::move(d)), queues(std::move(q)), allocator(std::move(a)),
        queue_graphics(device.getQueue(queues.primary_queue.queue_family_index, queues.primary_queue.queue_index)),
        queue_compute(device.getQueue(queues.compute_queues.front().queue_family_index, queues.compute_queues.front().queue_index)),
        queue_transfer(device.getQueue(queues.transfer_queues.front().queue_family_index, queues.transfer_queues.front().queue_index))
    {}
};

namespace
{
std::tuple<GhulbusVulkan::Device, detail::DeviceQueues> initializeVulkanDevice(GhulbusVulkan::Instance& instance);

std::unique_ptr<GraphicsInstance::Pimpl> initializeVulkanInstance(char const* application_name,
                                                                  GhulbusVulkan::Instance::Version application_version)
{
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

#ifndef NDEBUG
    extensions.enable_debug_report_extension = true;
#endif

    GhulbusVulkan::Instance instance =
        GhulbusVulkan::Instance::createInstance(application_name, application_version, layers, extensions);
    auto [device, queues] = initializeVulkanDevice(instance);
    detail::DeviceMemoryAllocator_VMA allocator(instance, device);

    return std::make_unique<GraphicsInstance::Pimpl>(std::move(instance), std::move(device), std::move(queues),
                                                     std::move(allocator));
}

std::tuple<GhulbusVulkan::Device, detail::DeviceQueues> initializeVulkanDevice(GhulbusVulkan::Instance& instance)
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

            // required: Vulkan 1.1 support
            if (pd.getProperties().apiVersion < VK_API_VERSION_1_1) {
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

            // required features
            auto const pd_features = pd.getFeatures();
            if (!pd_features.fillModeNonSolid) { continue; }
            if (!pd_features.samplerAnisotropy) { continue; }

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
    auto const device_properties = physical_device.getProperties();
    GHULBUS_LOG(Info, "Using device \'" << device_properties.deviceName << "\' (" <<
                      "Vulkan Version " << GhulbusVulkan::version_to_string(device_properties.apiVersion) << ", " <<
                      "Driver Version " << GhulbusVulkan::version_to_string(device_properties.driverVersion) <<
                      ").");

    GhulbusVulkan::DeviceBuilder device_builder = physical_device.createDeviceBuilder();

    // we instantiate:
    // 1 graphics queue
    // 1 compute queue
    // 1 transfer queue
    // the transfer queue, if absent, gets folded into compute queues, which in turn get folded into graphics
    detail::DeviceQueues queues = detail::selectQueues(winner, physical_device.getQueueFamilyProperties());
    for (auto const& q : detail::uniqueQueues(queues)) { device_builder.addQueues(q.queue_family_index, 1); }
    for (auto const& ext : required_extensions) { device_builder.addExtension(ext); }

    // add requested features
    device_builder.requested_features.fillModeNonSolid = VK_TRUE;      // wireframe drawing
    device_builder.requested_features.samplerAnisotropy = VK_TRUE;     // anisotropic filtering

    return std::make_tuple(device_builder.create(), queues);
}
}

GraphicsInstance::GraphicsInstance()
    :GraphicsInstance(nullptr, {})
{
}

GraphicsInstance::GraphicsInstance(char const* application_name, ApplicationVersion application_version)
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
    m_pimpl = initializeVulkanInstance(application_name,
        GhulbusVulkan::Instance::Version(application_version.major,
                                         application_version.minor,
                                         application_version.patch));
    exception_guard.defuse();

    m_commandPoolRegistry = std::make_unique<CommandPoolRegistry>(*this);
    m_reactor = std::make_unique<Reactor>(*this);

#ifndef NDEBUG
    setDebugLoggingEnabled(true);
#endif
}

GraphicsInstance::~GraphicsInstance()
{
    // reset pimpl manually to ensure Vulkan shuts down before glfw
    m_commandPoolRegistry.reset();
    m_pimpl.reset();
    glfwTerminate();
}

GhulbusVulkan::Instance& GraphicsInstance::getVulkanInstance()
{
    return m_pimpl->instance;
}

GhulbusVulkan::PhysicalDevice GraphicsInstance::getVulkanPhysicalDevice()
{
    return m_pimpl->device.getPhysicalDevice();
}

GhulbusVulkan::Device& GraphicsInstance::getVulkanDevice()
{
    return m_pimpl->device;
}

GhulbusVulkan::Queue& GraphicsInstance::getGraphicsQueue()
{
    return m_pimpl->queue_graphics;
}

uint32_t GraphicsInstance::getGraphicsQueueFamilyIndex()
{
    detail::DeviceQueues::QueueId const& queue = m_pimpl->queues.primary_queue;
    return queue.queue_family_index;
}

uint32_t GraphicsInstance::getGraphicsQueueIndex()
{
    detail::DeviceQueues::QueueId const& queue = m_pimpl->queues.primary_queue;
    return queue.queue_index;
}

GhulbusVulkan::Queue& GraphicsInstance::getComputeQueue()
{
    return m_pimpl->queue_compute;
}

uint32_t GraphicsInstance::getComputeQueueFamilyIndex()
{
    detail::DeviceQueues::QueueId const& queue = m_pimpl->queues.compute_queues.front();
    return queue.queue_family_index;
}

uint32_t GraphicsInstance::getComputeQueueIndex()
{
    detail::DeviceQueues::QueueId const& queue = m_pimpl->queues.compute_queues.front();
    return queue.queue_index;
}

GhulbusVulkan::Queue& GraphicsInstance::getTransferQueue()
{
    return m_pimpl->queue_transfer;
}

uint32_t GraphicsInstance::getTransferQueueFamilyIndex()
{
    detail::DeviceQueues::QueueId const& queue = m_pimpl->queues.transfer_queues.front();
    return queue.queue_family_index;
}

uint32_t GraphicsInstance::getTransferQueueIndex()
{
    detail::DeviceQueues::QueueId const& queue = m_pimpl->queues.transfer_queues.front();
    return queue.queue_index;
}

GhulbusVulkan::DeviceMemoryAllocator& GraphicsInstance::getDeviceMemoryAllocator()
{
    return m_pimpl->allocator;
    //static GhulbusVulkan::DeviceMemoryAllocator_Trivial allocator(m_pimpl->device.getVkDevice(), m_pimpl->device.getPhysicalDevice().getVkPhysicalDevice());
    //return allocator;
}

void GraphicsInstance::setDebugLoggingEnabled(bool enabled)
{
    if (enabled) {
        if (!m_pimpl->debug_logging) {
            VkDebugReportFlagsEXT const all_the_flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
                                                        VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                                        VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
                                                        VK_DEBUG_REPORT_ERROR_BIT_EXT |
                                                        VK_DEBUG_REPORT_DEBUG_BIT_EXT;
            m_pimpl->debug_logging.emplace(m_pimpl->instance, all_the_flags);
            m_pimpl->debug_logging->addCallback(
                [](VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT object_type,
                   uint64_t object, size_t location, int32_t message_code,
                   const char* layer_prefix, const char* message)
                -> GhulbusVulkan::DebugReportCallback::Return
                {
                    GHULBUS_UNUSED_VARIABLE(location);
                    GHULBUS_UNUSED_VARIABLE(message_code);
                    GHULBUS_LOG(Debug, layer_prefix << " [" <<
                        GhulbusVulkan::DebugReportCallback::translateFlags(flags) << "] - (" <<
                        GhulbusVulkan::DebugReportCallback::translateObjectType(object_type) << ") " <<
                        "0x" << std::hex << object << std::dec << ": " << message);
                    return GhulbusVulkan::DebugReportCallback::Return::Continue;
                });
        }
    } else {
        m_pimpl->debug_logging = std::nullopt;
    }
}

void GraphicsInstance::pollEvents()
{
    glfwPollEvents();
}

void GraphicsInstance::waitEvents()
{
    glfwWaitEvents();
}

CommandPoolRegistry& GraphicsInstance::getCommandPoolRegistry()
{
    return *m_commandPoolRegistry;
}

Reactor& GraphicsInstance::getReactor()
{
    return *m_reactor;
}
}
