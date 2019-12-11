#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_COMMAND_POOL_REGISTRY_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_COMMAND_POOL_REGISTRY_HPP

/** @file
*
* @brief Command Pool Registry.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbVk/ForwardDecl.hpp>

#include <gbVk/CommandPool.hpp>

#include <optional>
#include <mutex>
#include <thread>
#include <unordered_map>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
class GraphicsInstance;

/** Lazily allocates one command pool per type, per queue, per thread and hands out command buffers as requested.
 */
class CommandPoolRegistry {
private:
    struct Pools {
        std::optional<GhulbusVulkan::CommandPool> default_pool;
        std::optional<GhulbusVulkan::CommandPool> transient;
        std::optional<GhulbusVulkan::CommandPool> non_resetable;
    };
    struct QueuePools {
        Pools graphics;
        Pools compute;
        Pools transfer;
    };
    std::thread::id const m_mainThread;
    QueuePools m_mainPools;
    std::mutex m_mtx;
    std::unordered_map<std::thread::id, QueuePools> m_perThreadPools;
    GraphicsInstance* m_instance;
public:
    CommandPoolRegistry(GraphicsInstance& instance);

    CommandPoolRegistry(CommandPoolRegistry const&) = delete;
    CommandPoolRegistry& operator=(CommandPoolRegistry const&) = delete;

    CommandPoolRegistry(CommandPoolRegistry&&) = delete;
    CommandPoolRegistry& operator=(CommandPoolRegistry&&) = delete;

    GhulbusVulkan::CommandBuffers allocateGraphicCommandBuffers(std::uint32_t command_buffer_count);
    GhulbusVulkan::CommandBuffers allocateGraphicCommandBuffers_Transient(std::uint32_t command_buffer_count);
    GhulbusVulkan::CommandBuffers allocateGraphicCommandBuffers_NonResetable(std::uint32_t command_buffer_count);

    GhulbusVulkan::CommandBuffers allocateComputeCommandBuffers(std::uint32_t command_buffer_count);
    GhulbusVulkan::CommandBuffers allocateComputeCommandBuffers_Transient(std::uint32_t command_buffer_count);
    GhulbusVulkan::CommandBuffers allocateComputeCommandBuffers_NonResetable(std::uint32_t command_buffer_count);

    GhulbusVulkan::CommandBuffers allocateTransferCommandBuffers(std::uint32_t command_buffer_count);
    GhulbusVulkan::CommandBuffers allocateTransferCommandBuffers_Transient(std::uint32_t command_buffer_count);
    GhulbusVulkan::CommandBuffers allocateTransferCommandBuffers_NonResetable(std::uint32_t command_buffer_count);

private:
    QueuePools& getThreadPools();
    GhulbusVulkan::CommandPool& getOrCreate(std::optional<GhulbusVulkan::CommandPool>& optional_pool);
};
}
#endif
