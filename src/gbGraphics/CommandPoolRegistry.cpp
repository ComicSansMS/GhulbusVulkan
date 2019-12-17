#include <gbGraphics/CommandPoolRegistry.hpp>

#include <gbGraphics/Exceptions.hpp>
#include <gbGraphics/GraphicsInstance.hpp>

#include <gbVk/CommandBuffers.hpp>
#include <gbVk/Device.hpp>

namespace GHULBUS_GRAPHICS_NAMESPACE
{

CommandPoolRegistry::CommandPoolRegistry(GraphicsInstance& instance)
    :m_instance(&instance), m_mainThread(std::this_thread::get_id())
{
}

GhulbusVulkan::CommandBuffers CommandPoolRegistry::allocateCommandBuffersGraphics(std::uint32_t command_buffer_count)
{
    QueuePools& p = getThreadPools();
    if(!p.graphics.default_pool) {
        uint32_t const queue_family = m_instance->getGraphicsQueueFamilyIndex();
        VkCommandPoolCreateFlags const flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        m_instance->threadSafeDeviceAccess(
            [&out_pool = p.graphics.default_pool, flags, queue_family](GhulbusVulkan::Device& device) {
                out_pool.emplace(device.createCommandPool(flags, queue_family));
            });
    }
    return p.graphics.default_pool->allocateCommandBuffers(command_buffer_count);
}

GhulbusVulkan::CommandBuffers CommandPoolRegistry::allocateCommandBuffersGraphics_Transient(std::uint32_t command_buffer_count)
{
    QueuePools& p = getThreadPools();
    if(!p.graphics.transient) {
        uint32_t const queue_family = m_instance->getGraphicsQueueFamilyIndex();
        VkCommandPoolCreateFlags const flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        m_instance->threadSafeDeviceAccess(
            [&out_pool = p.graphics.transient, flags, queue_family](GhulbusVulkan::Device& device) {
                out_pool.emplace(device.createCommandPool(flags, queue_family));
            });
    }
    return p.graphics.transient->allocateCommandBuffers(command_buffer_count);
}

GhulbusVulkan::CommandBuffers CommandPoolRegistry::allocateCommandBuffersGraphics_NonResetable(std::uint32_t command_buffer_count)
{
    QueuePools& p = getThreadPools();
    if(!p.graphics.non_resetable) {
        uint32_t const queue_family = m_instance->getGraphicsQueueFamilyIndex();
        VkCommandPoolCreateFlags const flags = 0;
        m_instance->threadSafeDeviceAccess(
            [&out_pool = p.graphics.non_resetable, flags, queue_family](GhulbusVulkan::Device& device) {
                out_pool.emplace(device.createCommandPool(flags, queue_family));
            });
    }
    return p.graphics.non_resetable->allocateCommandBuffers(command_buffer_count);
}

GhulbusVulkan::CommandBuffers CommandPoolRegistry::allocateCommandBuffersCompute(std::uint32_t command_buffer_count)
{
    QueuePools& p = getThreadPools();
    if(!p.compute.default_pool) {
        uint32_t const queue_family = m_instance->getComputeQueueFamilyIndex();
        VkCommandPoolCreateFlags const flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        m_instance->threadSafeDeviceAccess(
            [&out_pool = p.compute.default_pool, flags, queue_family](GhulbusVulkan::Device& device) {
            out_pool.emplace(device.createCommandPool(flags, queue_family));
        });
    }
    return p.compute.default_pool->allocateCommandBuffers(command_buffer_count);
}

GhulbusVulkan::CommandBuffers CommandPoolRegistry::allocateCommandBuffersCompute_Transient(std::uint32_t command_buffer_count)
{
    QueuePools& p = getThreadPools();
    if(!p.compute.transient) {
        uint32_t const queue_family = m_instance->getComputeQueueFamilyIndex();
        VkCommandPoolCreateFlags const flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        m_instance->threadSafeDeviceAccess(
            [&out_pool = p.compute.transient, flags, queue_family](GhulbusVulkan::Device& device) {
            out_pool.emplace(device.createCommandPool(flags, queue_family));
        });
    }
    return p.compute.transient->allocateCommandBuffers(command_buffer_count);
}

GhulbusVulkan::CommandBuffers CommandPoolRegistry::allocateCommandBuffersCompute_NonResetable(std::uint32_t command_buffer_count)
{
    QueuePools& p = getThreadPools();
    if(!p.compute.non_resetable) {
        uint32_t const queue_family = m_instance->getComputeQueueFamilyIndex();
        VkCommandPoolCreateFlags const flags = 0;
        m_instance->threadSafeDeviceAccess(
            [&out_pool = p.compute.non_resetable, flags, queue_family](GhulbusVulkan::Device& device) {
            out_pool.emplace(device.createCommandPool(flags, queue_family));
        });
    }
    return p.compute.non_resetable->allocateCommandBuffers(command_buffer_count);
}

GhulbusVulkan::CommandBuffers CommandPoolRegistry::allocateCommandBuffersTransfer(std::uint32_t command_buffer_count)
{
    QueuePools& p = getThreadPools();
    if(!p.transfer.default_pool) {
        uint32_t const queue_family = m_instance->getTransferQueueFamilyIndex();
        VkCommandPoolCreateFlags const flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        m_instance->threadSafeDeviceAccess(
            [&out_pool = p.transfer.default_pool, flags, queue_family](GhulbusVulkan::Device& device) {
            out_pool.emplace(device.createCommandPool(flags, queue_family));
        });
    }
    return p.transfer.default_pool->allocateCommandBuffers(command_buffer_count);
}

GhulbusVulkan::CommandBuffers CommandPoolRegistry::allocateCommandBuffersTransfer_Transient(std::uint32_t command_buffer_count)
{
    QueuePools& p = getThreadPools();
    if(!p.transfer.transient) {
        uint32_t const queue_family = m_instance->getTransferQueueFamilyIndex();
        VkCommandPoolCreateFlags const flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        m_instance->threadSafeDeviceAccess(
            [&out_pool = p.transfer.transient, flags, queue_family](GhulbusVulkan::Device& device) {
            out_pool.emplace(device.createCommandPool(flags, queue_family));
        });
    }
    return p.transfer.transient->allocateCommandBuffers(command_buffer_count);
}

GhulbusVulkan::CommandBuffers CommandPoolRegistry::allocateCommandBuffersTransfer_NonResetable(std::uint32_t command_buffer_count)
{
    QueuePools& p = getThreadPools();
    if(!p.transfer.non_resetable) {
        uint32_t const queue_family = m_instance->getTransferQueueFamilyIndex();
        VkCommandPoolCreateFlags const flags = 0;
        m_instance->threadSafeDeviceAccess(
            [&out_pool = p.transfer.non_resetable, flags, queue_family](GhulbusVulkan::Device& device) {
            out_pool.emplace(device.createCommandPool(flags, queue_family));
        });
    }
    return p.transfer.non_resetable->allocateCommandBuffers(command_buffer_count);
}

CommandPoolRegistry::QueuePools& CommandPoolRegistry::getThreadPools()
{
    auto const thread_id = std::this_thread::get_id();
    if (thread_id == m_mainThread) {
        return m_mainPools;
    } else {
        std::scoped_lock lk(m_mtx);
        auto it = m_perThreadPools.find(thread_id);
        if (it == m_perThreadPools.end()) {
            it = m_perThreadPools.emplace(thread_id, QueuePools{}).first;
        }
        return it->second;
    }
}
}
