#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_BUFFER_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_BUFFER_HPP

/** @file
*
* @brief Buffer.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <gbVk/DeviceMemory.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
class CommandBuffer;

class Buffer {
private:
    VkBuffer m_buffer;
    VkDevice m_device;
public:
    Buffer(VkDevice logical_device, VkBuffer buffer);
    ~Buffer();

    Buffer(Buffer const&) = delete;
    Buffer& operator=(Buffer const&) = delete;

    Buffer(Buffer&& rhs);
    Buffer& operator=(Buffer&& rhs) = delete;

    void transitionRelease(CommandBuffer& command_buffer, VkPipelineStageFlags src_stage,
                           VkPipelineStageFlags dst_stage, VkAccessFlags src_access_mask,
                           uint32_t dst_queue_family);

    void transitionRelease(CommandBuffer& command_buffer, VkPipelineStageFlags src_stage,
                           VkPipelineStageFlags dst_stage, VkAccessFlags src_access_mask,
                           uint32_t dst_queue_family, VkDeviceSize offset, VkDeviceSize size);

    void transitionAcquire(CommandBuffer& command_buffer, VkPipelineStageFlags src_stage,
                           VkPipelineStageFlags dst_stage, VkAccessFlags dst_access_mask,
                           uint32_t src_queue_family);

    void transitionAcquire(CommandBuffer& command_buffer, VkPipelineStageFlags src_stage,
                           VkPipelineStageFlags dst_stage, VkAccessFlags dst_access_mask,
                           uint32_t src_queue_family, VkDeviceSize offset, VkDeviceSize size);

    VkBuffer getVkBuffer();

    VkMemoryRequirements getMemoryRequirements();

    void bindBufferMemory(DeviceMemory& memory);

    void bindBufferMemory(DeviceMemory& memory, VkDeviceSize offset);
};
}
#endif
