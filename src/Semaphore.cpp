#include <gbVk/Semaphore.hpp>
#include <gbVk/DebugUtilsObjectName.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
Semaphore::Semaphore(VkDevice logical_device, VkSemaphore semaphore)
    :m_semaphore(semaphore), m_device(logical_device)
{}

Semaphore::~Semaphore()
{
    if(m_semaphore) {
        vkDestroySemaphore(m_device, m_semaphore, nullptr);
    }
}

Semaphore::Semaphore(Semaphore&& rhs)
    :m_semaphore(rhs.m_semaphore), m_device(rhs.m_device)
{
    rhs.m_semaphore = nullptr;
    rhs.m_device    = nullptr;
}

void Semaphore::setDebugName(char const* name)
{
    DebugUtils::setObjectName(m_device, name, m_semaphore, VK_OBJECT_TYPE_SEMAPHORE);
}

VkSemaphore Semaphore::getVkSemaphore()
{
    return m_semaphore;
}
}
