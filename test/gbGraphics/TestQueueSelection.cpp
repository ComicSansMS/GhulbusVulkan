#include <gbGraphics/detail/QueueSelection.hpp>

#include <catch.hpp>

TEST_CASE("Queue Selection")
{
    using namespace GHULBUS_GRAPHICS_NAMESPACE::detail;

    SECTION("All Queues Available Separately")
    {
        PhysicalDeviceCandidate candidate;
        std::vector<VkQueueFamilyProperties> queue_props;
        candidate.physicalDeviceIndex = 42;
        candidate.primary_queue_family = 0;
        {
            VkQueueFamilyProperties primary;
            primary.queueCount = 1;
            primary.queueFlags = VK_QUEUE_GRAPHICS_BIT;
            queue_props.push_back(primary);
        }
        candidate.graphics_queue_families.push_back(1);
        {
            VkQueueFamilyProperties secondary_graphics;
            secondary_graphics.queueCount = 5;
            secondary_graphics.queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
            queue_props.push_back(secondary_graphics);
        }
        candidate.transfer_queue_families.push_back(2);
        {
            VkQueueFamilyProperties transfer;
            transfer.queueCount = 3;
            transfer.queueFlags = VK_QUEUE_TRANSFER_BIT;
            queue_props.push_back(transfer);
        }
        candidate.compute_queue_families.push_back(3);
        {
            VkQueueFamilyProperties compute;
            compute.queueCount = 2;
            compute.queueFlags = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
            queue_props.push_back(compute);
        }
        DeviceQueues queues = selectQueues(candidate, queue_props);
        CHECK(queues.primary_queue.is_unique);
        CHECK(queues.primary_queue.queue_family_index == 0);
        CHECK(queues.primary_queue.queue_index == 0);

        REQUIRE(queues.compute_queues.size() == 1);
        CHECK(queues.compute_queues[0].is_unique);
        CHECK(queues.compute_queues[0].queue_family_index == 3);
        CHECK(queues.compute_queues[0].queue_index == 0);

        REQUIRE(queues.transfer_queues.size() == 1);
        CHECK(queues.transfer_queues[0].is_unique);
        CHECK(queues.transfer_queues[0].queue_family_index == 2);
        CHECK(queues.transfer_queues[0].queue_index == 0);
    }

    SECTION("No Transfer Queues Available, Fold Into Compute")
    {
        PhysicalDeviceCandidate candidate;
        std::vector<VkQueueFamilyProperties> queue_props;
        candidate.physicalDeviceIndex = 42;
        candidate.primary_queue_family = 0;
        {
            VkQueueFamilyProperties primary;
            primary.queueCount = 1;
            primary.queueFlags = VK_QUEUE_GRAPHICS_BIT;
            queue_props.push_back(primary);
        }
        candidate.graphics_queue_families.push_back(1);
        {
            VkQueueFamilyProperties secondary_graphics;
            secondary_graphics.queueCount = 5;
            secondary_graphics.queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
            queue_props.push_back(secondary_graphics);
        }
        candidate.compute_queue_families.push_back(2);
        {
            VkQueueFamilyProperties compute;
            compute.queueCount = 2;
            compute.queueFlags = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
            queue_props.push_back(compute);
        }
        candidate.compute_queue_families.push_back(3);
        {
            VkQueueFamilyProperties compute;
            compute.queueCount = 2;
            compute.queueFlags = VK_QUEUE_COMPUTE_BIT;
            queue_props.push_back(compute);
        }
        DeviceQueues queues = selectQueues(candidate, queue_props);
        CHECK(queues.primary_queue.is_unique);
        CHECK(queues.primary_queue.queue_family_index == 0);
        CHECK(queues.primary_queue.queue_index == 0);

        REQUIRE(queues.compute_queues.size() == 1);
        CHECK(queues.compute_queues[0].is_unique);
        CHECK(queues.compute_queues[0].queue_family_index == 2);
        CHECK(queues.compute_queues[0].queue_index == 0);

        REQUIRE(queues.transfer_queues.size() == 1);
        CHECK(queues.transfer_queues[0].is_unique);
        CHECK(queues.transfer_queues[0].queue_family_index == 2);
        CHECK(queues.transfer_queues[0].queue_index == 1);
    }

    SECTION("No Transfer Queues Available, Fold Into Compute, Second Family")
    {
        PhysicalDeviceCandidate candidate;
        std::vector<VkQueueFamilyProperties> queue_props;
        candidate.physicalDeviceIndex = 42;
        candidate.primary_queue_family = 0;
        {
            VkQueueFamilyProperties primary;
            primary.queueCount = 1;
            primary.queueFlags = VK_QUEUE_GRAPHICS_BIT;
            queue_props.push_back(primary);
        }
        candidate.graphics_queue_families.push_back(1);
        {
            VkQueueFamilyProperties secondary_graphics;
            secondary_graphics.queueCount = 5;
            secondary_graphics.queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
            queue_props.push_back(secondary_graphics);
        }
        candidate.compute_queue_families.push_back(2);
        {
            VkQueueFamilyProperties compute;
            compute.queueCount = 2;
            compute.queueFlags = VK_QUEUE_COMPUTE_BIT;
            queue_props.push_back(compute);
        }
        candidate.compute_queue_families.push_back(3);
        {
            VkQueueFamilyProperties compute;
            compute.queueCount = 2;
            compute.queueFlags = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
            queue_props.push_back(compute);
        }
        DeviceQueues queues = selectQueues(candidate, queue_props);
        CHECK(queues.primary_queue.is_unique);
        CHECK(queues.primary_queue.queue_family_index == 0);
        CHECK(queues.primary_queue.queue_index == 0);

        REQUIRE(queues.compute_queues.size() == 1);
        CHECK(queues.compute_queues[0].is_unique);
        CHECK(queues.compute_queues[0].queue_family_index == 2);
        CHECK(queues.compute_queues[0].queue_index == 0);

        REQUIRE(queues.transfer_queues.size() == 1);
        CHECK(queues.transfer_queues[0].is_unique);
        CHECK(queues.transfer_queues[0].queue_family_index == 3);
        CHECK(queues.transfer_queues[0].queue_index == 0);
    }

    SECTION("No Transfer Queues Available, Only Single Compute Family")
    {
        PhysicalDeviceCandidate candidate;
        std::vector<VkQueueFamilyProperties> queue_props;
        candidate.physicalDeviceIndex = 42;
        candidate.primary_queue_family = 0;
        {
            VkQueueFamilyProperties primary;
            primary.queueCount = 1;
            primary.queueFlags = VK_QUEUE_GRAPHICS_BIT;
            queue_props.push_back(primary);
        }
        candidate.graphics_queue_families.push_back(1);
        {
            VkQueueFamilyProperties secondary_graphics;
            secondary_graphics.queueCount = 5;
            secondary_graphics.queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
            queue_props.push_back(secondary_graphics);
        }
        candidate.compute_queue_families.push_back(2);
        {
            VkQueueFamilyProperties compute;
            compute.queueCount = 2;
            compute.queueFlags = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
            queue_props.push_back(compute);
        }
        DeviceQueues queues = selectQueues(candidate, queue_props);
        CHECK(queues.primary_queue.is_unique);
        CHECK(queues.primary_queue.queue_family_index == 0);
        CHECK(queues.primary_queue.queue_index == 0);

        REQUIRE(queues.compute_queues.size() == 1);
        CHECK(queues.compute_queues[0].is_unique);
        CHECK(queues.compute_queues[0].queue_family_index == 2);
        CHECK(queues.compute_queues[0].queue_index == 0);

        REQUIRE(queues.transfer_queues.size() == 1);
        CHECK(queues.transfer_queues[0].is_unique);
        CHECK(queues.transfer_queues[0].queue_family_index == 2);
        CHECK(queues.transfer_queues[0].queue_index == 1);
    }

    SECTION("No Transfer Queues Available, Only Single Compute Family And Queue")
    {
        PhysicalDeviceCandidate candidate;
        std::vector<VkQueueFamilyProperties> queue_props;
        candidate.physicalDeviceIndex = 42;
        candidate.primary_queue_family = 0;
        {
            VkQueueFamilyProperties primary;
            primary.queueCount = 1;
            primary.queueFlags = VK_QUEUE_GRAPHICS_BIT;
            queue_props.push_back(primary);
        }
        candidate.graphics_queue_families.push_back(1);
        {
            VkQueueFamilyProperties secondary_graphics;
            secondary_graphics.queueCount = 5;
            secondary_graphics.queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
            queue_props.push_back(secondary_graphics);
        }
        candidate.compute_queue_families.push_back(2);
        {
            VkQueueFamilyProperties compute;
            compute.queueCount = 1;
            compute.queueFlags = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
            queue_props.push_back(compute);
        }
        DeviceQueues queues = selectQueues(candidate, queue_props);
        CHECK(queues.primary_queue.is_unique);
        CHECK(queues.primary_queue.queue_family_index == 0);
        CHECK(queues.primary_queue.queue_index == 0);

        REQUIRE(queues.compute_queues.size() == 1);
        CHECK_FALSE(queues.compute_queues[0].is_unique);
        CHECK(queues.compute_queues[0].queue_family_index == 2);
        CHECK(queues.compute_queues[0].queue_index == 0);

        REQUIRE(queues.transfer_queues.size() == 1);
        CHECK_FALSE(queues.transfer_queues[0].is_unique);
        CHECK(queues.transfer_queues[0].queue_family_index == 2);
        CHECK(queues.transfer_queues[0].queue_index == 0);
    }

    SECTION("No Transfer Queues Available, Compute Cannot Transfer, Fallback To Secondary Graphics")
    {
        PhysicalDeviceCandidate candidate;
        std::vector<VkQueueFamilyProperties> queue_props;
        candidate.physicalDeviceIndex = 42;
        candidate.primary_queue_family = 0;
        {
            VkQueueFamilyProperties primary;
            primary.queueCount = 1;
            primary.queueFlags = VK_QUEUE_GRAPHICS_BIT;
            queue_props.push_back(primary);
        }
        candidate.graphics_queue_families.push_back(1);
        {
            VkQueueFamilyProperties secondary_graphics;
            secondary_graphics.queueCount = 5;
            secondary_graphics.queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
            queue_props.push_back(secondary_graphics);
        }
        candidate.compute_queue_families.push_back(2);
        {
            VkQueueFamilyProperties compute;
            compute.queueCount = 1;
            compute.queueFlags = VK_QUEUE_COMPUTE_BIT;
            queue_props.push_back(compute);
        }
        DeviceQueues queues = selectQueues(candidate, queue_props);
        CHECK(queues.primary_queue.is_unique);
        CHECK(queues.primary_queue.queue_family_index == 0);
        CHECK(queues.primary_queue.queue_index == 0);

        REQUIRE(queues.compute_queues.size() == 1);
        CHECK(queues.compute_queues[0].is_unique);
        CHECK(queues.compute_queues[0].queue_family_index == 2);
        CHECK(queues.compute_queues[0].queue_index == 0);

        REQUIRE(queues.transfer_queues.size() == 1);
        CHECK(queues.transfer_queues[0].is_unique);
        CHECK(queues.transfer_queues[0].queue_family_index == 1);
        CHECK(queues.transfer_queues[0].queue_index == 1);
    }

    SECTION("No Transfer Queues Available, Compute And Secondary Graphics Cannot Transfer, Fallback To Primary")
    {
        PhysicalDeviceCandidate candidate;
        std::vector<VkQueueFamilyProperties> queue_props;
        candidate.physicalDeviceIndex = 42;
        candidate.primary_queue_family = 0;
        {
            VkQueueFamilyProperties primary;
            primary.queueCount = 2;
            primary.queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;
            queue_props.push_back(primary);
        }
        candidate.graphics_queue_families.push_back(1);
        {
            VkQueueFamilyProperties secondary_graphics;
            secondary_graphics.queueCount = 5;
            secondary_graphics.queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
            queue_props.push_back(secondary_graphics);
        }
        candidate.compute_queue_families.push_back(2);
        {
            VkQueueFamilyProperties compute;
            compute.queueCount = 1;
            compute.queueFlags = VK_QUEUE_COMPUTE_BIT;
            queue_props.push_back(compute);
        }
        DeviceQueues queues = selectQueues(candidate, queue_props);
        CHECK(queues.primary_queue.is_unique);
        CHECK(queues.primary_queue.queue_family_index == 0);
        CHECK(queues.primary_queue.queue_index == 0);

        REQUIRE(queues.compute_queues.size() == 1);
        CHECK(queues.compute_queues[0].is_unique);
        CHECK(queues.compute_queues[0].queue_family_index == 2);
        CHECK(queues.compute_queues[0].queue_index == 0);

        REQUIRE(queues.transfer_queues.size() == 1);
        CHECK(queues.transfer_queues[0].is_unique);
        CHECK(queues.transfer_queues[0].queue_family_index == 0);
        CHECK(queues.transfer_queues[0].queue_index == 1);
    }

    SECTION("Everything Shared With Primary")
    {
        PhysicalDeviceCandidate candidate;
        std::vector<VkQueueFamilyProperties> queue_props;
        candidate.physicalDeviceIndex = 42;
        candidate.primary_queue_family = 0;
        {
            VkQueueFamilyProperties primary;
            primary.queueCount = 1;
            primary.queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
            queue_props.push_back(primary);
        }
        DeviceQueues queues = selectQueues(candidate, queue_props);
        CHECK_FALSE(queues.primary_queue.is_unique);
        CHECK(queues.primary_queue.queue_family_index == 0);
        CHECK(queues.primary_queue.queue_index == 0);

        REQUIRE(queues.compute_queues.size() == 1);
        CHECK_FALSE(queues.compute_queues[0].is_unique);
        CHECK(queues.compute_queues[0].queue_family_index == 0);
        CHECK(queues.compute_queues[0].queue_index == 0);

        REQUIRE(queues.transfer_queues.size() == 1);
        CHECK_FALSE(queues.transfer_queues[0].is_unique);
        CHECK(queues.transfer_queues[0].queue_family_index == 0);
        CHECK(queues.transfer_queues[0].queue_index == 0);
    }
}
