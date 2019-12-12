#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_GRAPHICS_INSTANCE_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_GRAPHICS_INSTANCE_HPP

/** @file
*
* @brief Graphics Instance.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbVk/ForwardDecl.hpp>

#include <cstdint>
#include <memory>
#include <mutex>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
class CommandPoolRegistry;

struct ApplicationVersion {
    uint16_t major;
    uint16_t minor;
    uint32_t patch;

    ApplicationVersion()
        :major(0), minor(0), patch(0)
    {}

    ApplicationVersion(uint16_t major_version, uint16_t minor_version, uint32_t patch_version)
        :major(major_version), minor(minor_version), patch(patch_version)
    {}
};

class GraphicsInstance {
public:
    struct Pimpl;
private:
    std::unique_ptr<Pimpl> m_pimpl;
    std::unique_ptr<CommandPoolRegistry> m_commandPoolRegistry;
    std::mutex m_mtx;
public:
    GraphicsInstance();

    GraphicsInstance(char const* application_name, ApplicationVersion application_version);

    ~GraphicsInstance();

    GraphicsInstance(GraphicsInstance const&) = delete;
    GraphicsInstance& operator=(GraphicsInstance const&) = delete;
    GraphicsInstance(GraphicsInstance&&) = delete;
    GraphicsInstance& operator=(GraphicsInstance&&) = delete;

    GhulbusVulkan::Instance& getVulkanInstance();

    GhulbusVulkan::PhysicalDevice getVulkanPhysicalDevice();

    GhulbusVulkan::Device& getVulkanDevice();

    GhulbusVulkan::Queue& getGraphicsQueue();
    uint32_t getGraphicsQueueFamilyIndex();
    uint32_t getGraphicsQueueIndex();
    GhulbusVulkan::Queue& getComputeQueue();
    uint32_t getComputeQueueFamilyIndex();
    uint32_t getComputeQueueIndex();
    GhulbusVulkan::Queue& getTransferQueue();
    uint32_t getTransferQueueFamilyIndex();
    uint32_t getTransferQueueIndex();

    GhulbusVulkan::DeviceMemoryAllocator& getDeviceMemoryAllocator();

    void setDebugLoggingEnabled(bool enabled);

    void pollEvents();
    void waitEvents();

    CommandPoolRegistry& getCommandPoolRegistry();

    template<typename F>
    void threadSafeDeviceAccess(F&& f)
    {
        std::scoped_lock lk(m_mtx);
        f(getVulkanDevice());
    }
};
}
#endif
