#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_DETAIL_QUEUE_SELECTION_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_GRAPHICS_DETAIL_QUEUE_SELECTION_HPP

/** @file
*
* @brief Queue Selection.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbGraphics/config.hpp>

#include <gbVk/config.hpp>

#include <vulkan/vulkan.hpp>

#include <algorithm>
#include <optional>
#include <tuple>
#include <vector>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
namespace detail
{
struct PhysicalDeviceCandidate {
    std::size_t physicalDeviceIndex;
    std::optional<uint32_t> primary_queue_family;       ///< primary queue: must support graphics & presentation
    std::vector<uint32_t> graphics_queue_families;      ///< all other graphics queues
    std::vector<uint32_t> compute_queue_families;       ///< compute, but non-graphic queues
    std::vector<uint32_t> transfer_queue_families;      ///< transfer, but non-compute and non-graphic queues
};

struct DeviceQueues {
    struct QueueId {
        uint32_t queue_family_index;
        uint32_t queue_index;
    };
    QueueId primary_queue;
    std::vector<QueueId> compute_queues;
    std::vector<QueueId> transfer_queues;
};

inline bool operator==(DeviceQueues::QueueId const& lhs, DeviceQueues::QueueId const& rhs) {
    return (lhs.queue_family_index == rhs.queue_family_index) && (lhs.queue_index == rhs.queue_index);
}

inline bool operator<(DeviceQueues::QueueId const& lhs, DeviceQueues::QueueId const& rhs) {
    return std::tie(lhs.queue_family_index, lhs.queue_index) < std::tie(rhs.queue_family_index, rhs.queue_index);
}

DeviceQueues selectQueues(PhysicalDeviceCandidate const& candidate,
                          std::vector<VkQueueFamilyProperties> const& queue_properties);

std::vector<DeviceQueues::QueueId> uniqueQueues(DeviceQueues const& device_queues);
}
}
#endif
