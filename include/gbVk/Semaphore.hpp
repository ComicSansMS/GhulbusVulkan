#ifndef GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_SEMAPHORE_HPP
#define GHULBUS_LIBRARY_INCLUDE_GUARD_VULKAN_SEMAPHORE_HPP

/** @file
*
* @brief Semaphore.
* @author Andreas Weis (der_ghulbus@ghulbus-inc.de)
*/

#include <gbVk/config.hpp>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
class Semaphore {
private:
    VkSemaphore m_semaphore;
    VkDevice m_device;
public:
    Semaphore(VkDevice logical_device, VkSemaphore semaphore);

    ~Semaphore();

    Semaphore(Semaphore const&) = delete;
    Semaphore& operator=(Semaphore const&) = delete;

    Semaphore(Semaphore&& rhs);
    Semaphore& operator=(Semaphore&&) = delete;

    VkSemaphore getVkSemaphore();
};
}
#endif
