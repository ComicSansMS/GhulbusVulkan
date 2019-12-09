#include <gbGraphics/Graphics.hpp>

#include <gbGraphics/Exceptions.hpp>
#include <gbVk/Instance.hpp>
#include <gbBase/Assert.hpp>
#include <gbBase/Log.hpp>

#ifndef GLFW_INCLUDE_VULKAN
#   define GLFW_INCLUDE_VULKAN
#endif
#include <GLFW/glfw3.h>

namespace GHULBUS_GRAPHICS_NAMESPACE {
namespace {
GhulbusVulkan::Instance* g_Instance = nullptr;

void initializeVulkanInstance(char const* application_name, GhulbusVulkan::Instance::Version application_version)
{
    GHULBUS_PRECONDITION(!g_Instance);
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
    g_Instance = new GhulbusVulkan::Instance(std::move(instance));
}

void shutdownVulkanInstance()
{
    GHULBUS_PRECONDITION(g_Instance);
    delete g_Instance;
    g_Instance = nullptr;
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
    if (!glfwVulkanSupported()) {
        glfwTerminate();
        GHULBUS_THROW(Exceptions::GLFWError{}, "No Vulkan support in GLFW.");
    }
    glfwSetErrorCallback([](int ec, char const* msg) { GHULBUS_LOG(Error, "GLFW Error " << ec << " - " << msg); });
    initializeVulkanInstance(application_name,
        GhulbusVulkan::Instance::Version(application_version.major,
                                         application_version.minor,
                                         application_version.patch));
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

GHULBUS_VULKAN_NAMESPACE::Instance& getVulkanInstance()
{
    return *g_Instance;
}
}
