#include <gbVk/Event.hpp>

#include <gbVk/Exceptions.hpp>

#include <gbBase/Assert.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{

Event::Event(VkDevice logical_device, VkEvent event_v)
    :m_event(event_v), m_device(logical_device)
{
}

Event::~Event()
{
    if (m_event) {
        vkDestroyEvent(m_device, m_event, nullptr);
    }
}

Event::Event(Event&& rhs)
    :m_event(rhs.m_event), m_device(rhs.m_device)
{
    rhs.m_event = nullptr;
    rhs.m_device = nullptr;
}

void Event::setEvent()
{
    VkResult const res = vkSetEvent(m_device, m_event);
    checkVulkanError(res, "Error in vkSetEvent.");
}

void Event::resetEvent()
{
    VkResult const res = vkResetEvent(m_device, m_event);
    checkVulkanError(res, "Error in vkResetEvent.");
}

bool Event::getStatus()
{
    VkResult const res = vkGetEventStatus(m_device, m_event);
    if (res == VK_EVENT_SET) {
        return true;
    } else if (res == VK_EVENT_RESET) {
        return false;
    }
    checkVulkanError(res, "Error in vkGetEventStatus.");
    GHULBUS_UNREACHABLE();
}

VkEvent Event::getVkEvent()
{
    return m_event;
}
}
