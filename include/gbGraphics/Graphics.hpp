#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_GRAPHICS_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_GRAPHICS_HPP

/** @file
*
* @brief Main Header.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbVk/ForwardDecl.hpp>

#include <cstdint>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
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

void initialize();

void initialize(char const* application_name, ApplicationVersion application_version);

void shutdown();

struct [[nodiscard]] InitializeGuard{
private:
    bool m_doShutdown;
public:
    InitializeGuard()
        :m_doShutdown(true)
    {}

    ~InitializeGuard() {
        if (m_doShutdown) { shutdown(); }
    }

    InitializeGuard(InitializeGuard const&) = delete;
    InitializeGuard& operator=(InitializeGuard const&) = delete;

    InitializeGuard(InitializeGuard&& rhs)
        :m_doShutdown(rhs.m_doShutdown)
    {
        rhs.m_doShutdown = false;
    }

    InitializeGuard& operator=(InitializeGuard&& rhs) = delete;
};

InitializeGuard initializeWithGuard();

InitializeGuard initializeWithGuard(char const* application_name, ApplicationVersion application_version);

GhulbusVulkan::Instance& getVulkanInstance();

GhulbusVulkan::PhysicalDevice& getVulkanPhysicalDevice();
}
#endif
