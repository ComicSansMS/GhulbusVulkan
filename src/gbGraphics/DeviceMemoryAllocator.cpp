#include <gbGraphics/DeviceMemoryAllocator.hpp>

namespace GHULBUS_GRAPHICS_NAMESPACE
{
DeviceMemoryAllocator::Handle::Handle(std::unique_ptr<HandleConcept>&& handle)
    :m_handle(std::move(handle))
{}

VkDeviceMemory DeviceMemoryAllocator::Handle::getVkDeviceMemory() const
{
    return m_handle->getVkDeviceMemory();
}

VkDeviceSize DeviceMemoryAllocator::Handle::getOffset() const
{
    return m_handle->getOffset();
}

VkDeviceSize DeviceMemoryAllocator::Handle::getSize() const
{
    return m_handle->getSize();
}

DeviceMemoryAllocator::Handle::~Handle() = default;

DeviceMemoryAllocator::HandleConcept::~HandleConcept() = default;
}
