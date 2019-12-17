#include <gbVk/DeviceMemoryAllocator.hpp>

#include <gbVk/Buffer.hpp>
#include <gbVk/Image.hpp>

namespace GHULBUS_VULKAN_NAMESPACE
{
DeviceMemoryAllocator::HandleConcept::~HandleConcept() = default;

using DeviceMemory = DeviceMemoryAllocator::DeviceMemory;

DeviceMemory::DeviceMemory(std::unique_ptr<HandleConcept>&& handle)
    :m_handle(std::move(handle))
{}

DeviceMemory::~DeviceMemory() = default;

void DeviceMemory::bindBuffer(Buffer& buffer)
{
    m_handle->bindBuffer(buffer.getVkBuffer());
}

void DeviceMemory::bindImage(Image& image)
{
    m_handle->bindImage(image.getVkImage());
}

VkDeviceMemory DeviceMemory::getVkDeviceMemory() const
{
    return m_handle->getVkDeviceMemory();
}

VkDeviceSize DeviceMemory::getOffset() const
{
    return m_handle->getOffset();
}

VkDeviceSize DeviceMemory::getSize() const
{
    return m_handle->getSize();
}

auto DeviceMemory::map() -> MappedMemory
{
    return map(0, VK_WHOLE_SIZE);
}

auto DeviceMemory::map(VkDeviceSize offset, VkDeviceSize size) -> MappedMemory
{
    void* mapped_memory = m_handle->mapMemory(offset, size);
    return MappedMemory(*m_handle, mapped_memory);
}

using MappedMemory = DeviceMemoryAllocator::DeviceMemory::MappedMemory;

MappedMemory::MappedMemory(HandleConcept& handle, void* mapped_memory)
    :m_mappedMemory(mapped_memory), m_handle(&handle)
{
}

MappedMemory::~MappedMemory()
{
    if(m_mappedMemory) {
        m_handle->unmapMemory(m_mappedMemory);
    }
}

MappedMemory::MappedMemory(MappedMemory&& rhs)
    :m_mappedMemory(rhs.m_mappedMemory), m_handle(rhs.m_handle)
{
    rhs.m_mappedMemory = nullptr;
    rhs.m_handle = nullptr;
}

MappedMemory& MappedMemory::operator=(MappedMemory&& rhs)
{
    if (&rhs != this) {
        if(m_mappedMemory) { m_handle->unmapMemory(m_mappedMemory); }
        m_mappedMemory = rhs.m_mappedMemory;
        m_handle = rhs.m_handle;
        rhs.m_mappedMemory = nullptr;
        rhs.m_handle = nullptr;
    }
    return *this;
}

MappedMemory::operator std::byte*()
{
    return reinterpret_cast<std::byte*>(m_mappedMemory);
}

std::byte& MappedMemory::operator[](std::size_t index)
{
    return reinterpret_cast<std::byte*>(m_mappedMemory)[index];
}

MappedMemory::operator std::byte const*() const
{
    return reinterpret_cast<std::byte const*>(m_mappedMemory);
}

std::byte MappedMemory::operator[](std::size_t index) const
{
    return reinterpret_cast<std::byte const*>(m_mappedMemory)[index];
}

void MappedMemory::flush()
{
    m_handle->flush(0, VK_WHOLE_SIZE);
}

void MappedMemory::invalidate()
{
    m_handle->invalidate(0, VK_WHOLE_SIZE);
}
}
