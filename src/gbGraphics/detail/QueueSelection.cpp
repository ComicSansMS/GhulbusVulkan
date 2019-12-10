#include <gbGraphics/detail/QueueSelection.hpp>

#include <gbBase/Assert.hpp>

namespace GHULBUS_GRAPHICS_NAMESPACE::detail
{

namespace 
{
std::optional<DeviceQueues::QueueId> fallbackQueuePrimaryFamily(VkQueueFlagBits requested,
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
            queue_id.is_unique = true;
            return queue_id;
        } else {
            // share queue for primary and requested
            DeviceQueues::QueueId queue_id;
            queue_id.queue_family_index = primary_fallback_family;
            queue_id.queue_index = 0;
            queue_id.is_unique = false;
            return queue_id;
        }
    } else {
        // the fallback queue does not support requested
        return std::nullopt;
    }
}

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
        queue_id.is_unique = true;
        return queue_id;
    } else if(!candidate_families.empty()) {
        // no secondary fallback queue family supports the requested; try primary one
        uint32_t const primary_fallback_family = candidate_families.front();
        return fallbackQueuePrimaryFamily(requested, primary_fallback_family, queue_properties);
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
    ret.primary_queue.is_unique = true;

    if (!candidate.compute_queue_families.empty()) {
        DeviceQueues::QueueId queue_id;
        queue_id.queue_family_index = candidate.compute_queue_families.front();
        queue_id.queue_index = 0;
        queue_id.is_unique = true;
        ret.compute_queues.push_back(queue_id);
    } else {
        auto const opt_queue_graphics =
            fallbackQueueFor(VK_QUEUE_COMPUTE_BIT, candidate.graphics_queue_families, queue_properties);
        if (opt_queue_graphics) {
            assert(opt_queue_graphics->is_unique);
            ret.compute_queues.push_back(*opt_queue_graphics);
        } else {
            auto opt_primary =
                fallbackQueuePrimaryFamily(VK_QUEUE_COMPUTE_BIT, *candidate.primary_queue_family, queue_properties);
            ret.compute_queues.push_back(*opt_primary);
            ret.primary_queue.is_unique = false;
        }
    }

    if (!candidate.transfer_queue_families.empty()) {
        // use the first available transfer queue family
        DeviceQueues::QueueId queue_id;
        queue_id.queue_family_index = candidate.transfer_queue_families.front();
        queue_id.queue_index = 0;
        queue_id.is_unique = true;
        ret.transfer_queues.push_back(queue_id);
    } else {
        if (!candidate.compute_queue_families.empty()) {
            auto const opt_queue_id =
                fallbackQueueFor(VK_QUEUE_TRANSFER_BIT, candidate.compute_queue_families, queue_properties);
            if (opt_queue_id) {
                if (!opt_queue_id->is_unique) {
                    // obtained queue is shared; mark the other end as non-unique
                    assert(ret.compute_queues.size() == 1);
                    ret.compute_queues.back().is_unique = false;
                }
                ret.transfer_queues.push_back(*opt_queue_id);
            }
        }
        if (ret.transfer_queues.empty()) {
            auto const opt_queue_graphics =
                fallbackQueueFor(VK_QUEUE_TRANSFER_BIT, candidate.graphics_queue_families, queue_properties);
            if (opt_queue_graphics) {
                assert(opt_queue_graphics->is_unique);
                ret.transfer_queues.push_back(*opt_queue_graphics);
            } else {
                auto opt_primary =
                    fallbackQueuePrimaryFamily(VK_QUEUE_TRANSFER_BIT, *candidate.primary_queue_family, queue_properties);
                ret.transfer_queues.push_back(*opt_primary);
                ret.primary_queue.is_unique = false;
            }
        }
    }

    return ret;
}
}
