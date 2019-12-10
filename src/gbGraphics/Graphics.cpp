#include <gbGraphics/Graphics.hpp>

#include <gbGraphics/Exceptions.hpp>
#include <gbGraphics/detail/QueueSelection.hpp>

#include <gbVk/DeviceBuilder.hpp>
#include <gbVk/Instance.hpp>
#include <gbVk/PhysicalDevice.hpp>

#include <gbBase/Assert.hpp>
#include <gbBase/Finally.hpp>
#include <gbBase/Log.hpp>

#ifndef GLFW_INCLUDE_VULKAN
#   define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>

namespace GHULBUS_GRAPHICS_NAMESPACE {
namespace {
GhulbusVulkan::Instance* g_instance = nullptr;
GhulbusVulkan::PhysicalDevice* g_physicalDevice = nullptr;

void initializeVulkanInstance(char const* application_name, GhulbusVulkan::Instance::Version application_version);
void initializeVulkanDevice(GhulbusVulkan::Instance& instance);

void initializeVulkanInstance(char const* application_name, GhulbusVulkan::Instance::Version application_version)
{
    GHULBUS_PRECONDITION(!g_instance);
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
        initializeVulkanDevice(instance);

        g_instance = new GhulbusVulkan::Instance(std::move(instance));
}

void initializeVulkanDevice(GhulbusVulkan::Instance& instance)
{
    std::vector<GhulbusVulkan::PhysicalDevice> physical_devices = instance.enumeratePhysicalDevices();
    detail::PhysicalDeviceCandidate const winner = [&instance, &physical_devices]() {
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
            if (candidate.primary_queue_family) {
                candidates.push_back(candidate);
            }
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
    GhulbusVulkan::DeviceBuilder device_builder = physical_device.createDeviceBuilder();

    // we instantiate:
    // 1 graphics queue
    // 1 compute queues
    // 1 transfer queues
    // the transfer queue, if absent, gets folded into compute queues, which in turn get folded into graphics
    std::vector<VkQueueFamilyProperties> const qfprops = physical_device.getQueueFamilyProperties();

    GHULBUS_ASSERT(winner.primary_queue_family.has_value());
    device_builder.addQueues(*winner.primary_queue_family, 1);
    if (!winner.compute_queue_families.empty()) {

    }
}

void shutdownVulkanInstance()
{
    GHULBUS_PRECONDITION(g_instance);
    //GHULBUS_PRECONDITION(g_physicalDevice);
    //delete g_physicalDevice;
    //g_physicalDevice = nullptr;
    delete g_instance;
    g_instance = nullptr;
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
    GHULBUS_PRECONDITION(g_instance);
    return *g_instance;
}

GhulbusVulkan::PhysicalDevice& getVulkanPhysicalDevice()
{
    GHULBUS_PRECONDITION(g_physicalDevice);
    return *g_physicalDevice;
}
}
