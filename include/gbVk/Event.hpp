#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_EVENT_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_EVENT_HPP

/** @file
*
* @brief Event.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
class Event {
private:
    VkEvent m_event;
    VkDevice m_device;
public:
    Event(VkDevice logical_device, VkEvent event_v);

    ~Event();

    Event(Event const&) = delete;
    Event& operator=(Event const&) = delete;

    Event(Event&& rhs);
    Event& operator=(Event&& rhs) = delete;

    void setEvent();
    void resetEvent();

    bool getStatus();

    VkEvent getVkEvent();
};
}
#endif
