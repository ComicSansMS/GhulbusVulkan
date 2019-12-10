#include <gbGraphics/detail/QueueSelection.hpp>

#include <gbBase/Assert.hpp>

namespace GHULBUS_GRAPHICS_NAMESPACE::detail
{

namespace 
{
/** Predicate checking whether a queue family supports requested.
 */
inline bool queueDoesSupport(uint32_t queue_family_index, VkQueueFlagBits requested,
                             std::vector<VkQueueFamilyProperties> const& queue_properties)
{
    return (queue_properties[queue_family_index].queueFlags & requested) != 0;
}

/** Finds the first queue family in the list that supports requested.
 */
template<typename It>
inline It findQueueSupporting(It it_begin, It it_end, VkQueueFlagBits requested,
                              std::vector<VkQueueFamilyProperties> const& queue_properties)
{
    return std::find_if(it_begin, it_end, [&queue_properties, requested](uint32_t family_index) {
            return queueDoesSupport(family_index, requested, queue_properties);
        });
}

/** Obtain a free queue from the supplied family, giving preference to unoccupied ones.
 */
std::optional<DeviceQueues::QueueId> fallbackQueueSharedFamily(VkQueueFlagBits requested,
    uint32_t const& primary_fallback_family,
    std::vector<VkQueueFamilyProperties> const& queue_properties)
{
    if (queueDoesSupport(primary_fallback_family, requested, queue_properties)) {
        auto const qprops = queue_properties[primary_fallback_family];
        if (qprops.queueCount > 1) {
            // use second queue of primary fallback family for requested
            DeviceQueues::QueueId queue_id;
            queue_id.queue_family_index = primary_fallback_family;
            queue_id.queue_index = 1;
            return queue_id;
        } else {
            // share queue for primary and requested
            DeviceQueues::QueueId queue_id;
            queue_id.queue_family_index = primary_fallback_family;
            queue_id.queue_index = 0;
            return queue_id;
        }
    } else {
        // the fallback queue does not support requested
        return std::nullopt;
    }
}

/** Obtain a free queue from any family able to serve requested, giving preference to unoccupied ones.
 */
std::optional<DeviceQueues::QueueId> fallbackQueueFor(VkQueueFlagBits requested,
                                                      std::vector<uint32_t> const& candidate_families,
                                                      std::vector<VkQueueFamilyProperties> const& queue_properties)
{
    auto const it = (candidate_families.size() > 1) ?
        findQueueSupporting(candidate_families.begin() + 1, candidate_families.end(), requested, queue_properties) :
        candidate_families.end();
    if (it != candidate_families.end()) {
        // use secondary fallback family for requested
        DeviceQueues::QueueId queue_id;
        queue_id.queue_family_index = *it;
        queue_id.queue_index = 0;
        return queue_id;
    } else if(!candidate_families.empty()) {
        // no secondary fallback queue family supports the requested; try primary one
        uint32_t const primary_fallback_family = candidate_families.front();
        return fallbackQueueSharedFamily(requested, primary_fallback_family, queue_properties);
    } else {
        // candidate families does not contain anything
        return std::nullopt;
    }
}
}

DeviceQueues selectQueues(PhysicalDeviceCandidate const& candidate,
                          std::vector<VkQueueFamilyProperties> const& queue_properties)
{
    GHULBUS_PRECONDITION(candidate.primary_queue_family.has_value());
    DeviceQueues ret;
    ret.primary_queue.queue_family_index = *candidate.primary_queue_family;
    ret.primary_queue.queue_index = 0;

    auto const fallbackToGraphics = [&queue_properties, &candidate](VkQueueFlagBits requested) {
        // first try secondary graphics queues
        auto const opt_queue_graphics =
            fallbackQueueFor(requested, candidate.graphics_queue_families, queue_properties);
        if (opt_queue_graphics) { return opt_queue_graphics; }
        auto const opt_primary =
                fallbackQueueSharedFamily(requested, *candidate.primary_queue_family, queue_properties);
        return opt_primary;
    };

    if (!candidate.compute_queue_families.empty()) {
        DeviceQueues::QueueId queue_id;
        queue_id.queue_family_index = candidate.compute_queue_families.front();
        queue_id.queue_index = 0;
        ret.compute_queues.push_back(queue_id);
    } else {
        auto const opt_fallback = fallbackToGraphics(VK_QUEUE_COMPUTE_BIT);
        if (opt_fallback) { ret.compute_queues.push_back(*opt_fallback); }
    }

    if (!candidate.transfer_queue_families.empty()) {
        // use the first available transfer queue family
        DeviceQueues::QueueId queue_id;
        queue_id.queue_family_index = candidate.transfer_queue_families.front();
        queue_id.queue_index = 0;
        ret.transfer_queues.push_back(queue_id);
    } else {
        auto const opt_queue_id =
            fallbackQueueFor(VK_QUEUE_TRANSFER_BIT, candidate.compute_queue_families, queue_properties);
        if (opt_queue_id) {
            ret.transfer_queues.push_back(*opt_queue_id);
        } else {
            auto const opt_fallback = fallbackToGraphics(VK_QUEUE_TRANSFER_BIT);
            if (opt_fallback) { ret.transfer_queues.push_back(*opt_fallback); }
        }
    }

    return ret;
}
}
